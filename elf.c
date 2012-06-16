/**
 * Copyright (c) 2012, Felipe Pena
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *   Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <elf.h>
#include <link.h>
#include <limits.h>
#include <string.h>
#include "cmd.h"
#include "elf.h"
#include "ptrace.h"

/**
 * Resolves the link_map tables
 */
static int _px_elf_resolv_tables(struct link_map *map) {
	ElfW(Dyn) dyn;
	uintptr_t addr = (uintptr_t) map->l_ld;
	ElfW(Word) nchains;

	do {
		ptrace_read(addr, &dyn, sizeof(dyn));

		if (dyn.d_tag == 0) {
			break;
		}

		switch (dyn.d_tag) {
			case DT_HASH:
				ptrace_read(dyn.d_un.d_ptr + map->l_addr,
					&nchains, sizeof(nchains));
				break;
			case DT_STRTAB:
				ELF(strtab) = dyn.d_un.d_ptr;
				printf("Strtab found at %#lx\n", ELF(strtab));
				break;
			case DT_SYMTAB:
				ELF(symtab) = dyn.d_un.d_ptr;
				printf("Symtab found at %#lx\n", ELF(symtab));
				break;
		}

		addr += sizeof(dyn);
	} while (1);

	return nchains;
}

#define ELF_ST_TYPE _ElfW(ELF, __ELF_NATIVE_CLASS, ST_TYPE)

/**
 * Finds a symbol in the tables
 */
int px_elf_find_symbol(const char* name) {
	struct link_map map;
	ElfW(Sym) sym;
	char str[128], libname[PATH_MAX];
	int i = 0, nchains;

	ptrace_read(ELF(map), &map, sizeof(map));

	do {
		ptrace_read((uintptr_t)map.l_next, &map, sizeof(map));
		ptrace_read((uintptr_t)map.l_name, libname, sizeof(libname));

		nchains = _px_elf_resolv_tables(&map);

		printf("Library %s (chains=%d)\n", libname, nchains);
		i = 0;

		while (i < nchains) {
			ptrace_read(ELF(symtab) + (i * sizeof(ElfW(Sym))), &sym, sizeof(sym));

			++i;

			if (ELF_ST_TYPE(sym.st_info) != STT_FUNC) {
				continue;
			}

			ptrace_read(ELF(strtab) + sym.st_name, str, sizeof(str));

			printf("%d [%s]\n", ELF_ST_TYPE(sym.st_info), str);

			if (memcmp(str, name, strlen(name)) == 0) {
				return 1;
			}
		}
	} while (map.l_next);

	return 0;
}

/**
 * Reads the ELF data from the target process
 */
void px_elf_maps(void) {
	ElfW(Ehdr) header;
	ElfW(Phdr) phdr;
	ElfW(Dyn) dyn;
	uintptr_t addr;
	int i = 0;

	ptrace_read(ELF(header), &header, sizeof(header));

	printf("Program header at %#lx\n", ELF(header) + header.e_phoff);

	/* Locate the PT_DYNAMIC program header */
	do {
		ptrace_read(ELF(header) + header.e_phoff + i, &phdr, sizeof(phdr));
		i += sizeof(phdr);
	} while (phdr.p_type != PT_DYNAMIC);

	addr = phdr.p_vaddr;

	/* Locate the GOT address */
	do {
		ptrace_read(addr += sizeof(dyn), &dyn, sizeof(dyn));

		if (dyn.d_tag == DT_PLTGOT) {
			ELF(got) = dyn.d_un.d_ptr;
			break;
		}
	} while (1);

	printf("GOT address at %#lx\n", ELF(got));

	/* Read the link_map address from the second GOT entry */
	ptrace_read(ELF(got) + sizeof(void*), &addr, sizeof(void*));

	/* Sets the link_map address */
	ELF(map) = addr;

	px_elf_find_symbol("foo");
}

/**
 * Dumps the ELF sections
 */
void px_elf_dump_sections(void) {
	ElfW(Ehdr) header;
	ElfW(Shdr) section;
	const char *name;
	uintptr_t sections;
	int i;

	ptrace_read(ELF(header), &header, sizeof(header));

	sections = ELF(header) + header.e_shoff;

	printf("Nr  | Type     | Flags | VAddr\n");

	for (i = 0; i < header.e_shnum; ++i) {
		ptrace_read(sections + (i * sizeof(section)), &section, sizeof(section));

		switch (section.sh_type) {
			case SHT_SYMTAB:   name = "SYMTAB";   break;
			case SHT_STRTAB:   name = "STRTAB";   break;
			case SHT_HASH:     name = "HASH";     break;
			case SHT_DYNAMIC:  name = "DYNAMIC";  break;
            case SHT_NOTE:     name = "NOTE";     break;
			case SHT_NULL:     name = "NULL";     break;
			case SHT_PROGBITS: name = "PROGBITS"; break;
			case SHT_NOBITS:   name = "NOBITS";   break;
			case SHT_REL:      name = "REL";      break;
			case SHT_SHLIB:    name = "SHLIB";    break;
			case SHT_DYNSYM:   name = "DYNSYM";   break;
			case SHT_LOPROC:   name = "LOPROC";   break;
			case SHT_HIPROC:   name = "HIPROC";   break;
			case SHT_LOUSER:   name = "LOUSER";   break;
			case SHT_HIUSER:   name = "HIUSER";   break;
			default:           name = "UNKNOWN";  break;
		}
		printf(" %-2d | %-8s | %c%c%c   | %#lx\n", i, name,
			section.sh_flags & SHF_ALLOC     ? 'A' : '-',
			section.sh_flags & SHF_WRITE     ? 'W' : '-',
			section.sh_flags & SHF_EXECINSTR ? 'X' : '-',
			section.sh_addr
			);
	}

	printf("Number of sections: %d\n", header.e_shnum);
}

/**
 * Clears the ELF related data
 */
void px_elf_clear(void) {
	memset(&ENV(elf), 0, sizeof(ENV(elf)));
}
