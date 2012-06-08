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

#include <stdio.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include "common.h"
#include "maps.h"
#include "cmd.h"
#include "ptrace.h"

#define ELF(x) ENV(elf.x)

/**
 * Parses an line from /proc/<pid>/maps
 */
void px_maps_region(const char *line) {
	uintptr_t start, end;
	char perms[5], filename[PATH_MAX];
	int offset, dmajor, dminor, inode;
	const size_t n = ENV(nregions);

	if (sscanf(line, "%lx-%lx %s %x %x:%x %u %s",
		&start, &end, perms, &offset, &dmajor, &dminor, &inode, filename) < 6 ||
		(end - start) == 0) {
		return;
	}

	if (ENV(maps) == NULL || n % 5 == 0) {
		ENV(maps) = (px_maps*) realloc(ENV(maps), sizeof(px_maps) * (n + 5));

		if (ENV(maps) == NULL) {
			px_error("Failed to realloc!\n");
			return;
		}
	}

	ENV(maps)[n].start = start;
	ENV(maps)[n].end = end;
	memcpy(ENV(maps)[n].filename, filename, sizeof(filename));
	memcpy(ENV(maps)[n].perms, perms, sizeof(perms));

	/* Sets the start address where the read ELF data will be read */
	ELF(saddr) = ENV(maps)[0].start;

	++ENV(nregions);
}

/**
 * Finds a mapped region by address
 */
int px_maps_find_region(uintptr_t addr) {
	int i = 0;

	while (i < ENV(nregions)) {
		if (ENV(maps)[i].start <= addr && ENV(maps)[i].end >= addr) {
			printf("Found... %s (%s)\n",
				ENV(maps)[i].filename, ENV(maps)[i].perms);
			return 1;
		}
		++i;
	}
	return 0;
}

/**
 * Resolves the link_map tables
 */
int _px_maps_resolv_tables(struct link_map *map) {
	ElfW(Dyn) dyn;
	uintptr_t addr = (uintptr_t) map->l_ld;
	int nchains;

	do {
		ptrace_read(addr, &dyn, sizeof(dyn));

		if (dyn.d_tag == 0) {
			break;
		}

		switch (dyn.d_tag) {
			case DT_HASH:
				ptrace_read(dyn.d_un.d_ptr + map->l_addr + sizeof(void*),
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
int px_maps_find_symbol(const char* name) {
	struct link_map map;
	ElfW(Sym) sym;
	char str[128];
	char libname[256];
	int i = 0, nchains, found = 0;

	ptrace_read(ELF(map), &map, sizeof(map));

	do {
		ptrace_read((uintptr_t)map.l_name, libname, sizeof(libname));

		nchains = _px_maps_resolv_tables(&map);
		i = 0;

		while (i < nchains) {
			ptrace_read(ELF(symtab) + (i * sizeof(ElfW(Sym))), &sym, sizeof(sym));

			++i;

			if (ELF_ST_TYPE(sym.st_info) != STT_FUNC) {
				continue;
			}

			ptrace_read(ELF(strtab) + sym.st_name, str, sizeof(str));

			printf("[%s]\n", str);

			if (memcmp(str, name, strlen(name)) == 0) {
				return 1;
			}
		}

		ptrace_read((uintptr_t)map.l_next, &map, sizeof(map));
	} while (found == 0 && map.l_next);

	return 0;
}

/**
 * Reads the ELF data from the target process
 */
void px_maps_elf(const char *file) {
	ElfW(Ehdr) header;
	ElfW(Phdr) phdr;
	ElfW(Dyn) dyn;
	uintptr_t addr;
	int i = 0;

	ptrace_read(ELF(saddr), &header, sizeof(header));

	printf("Program header at %#lx\n", ELF(saddr) + header.e_phoff);

	/* Locate the PT_DYNAMIC program header */
	do {
		ptrace_read(ELF(saddr) + header.e_phoff + i, &phdr, sizeof(phdr));
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

	ELF(map) = addr;

	px_maps_find_symbol("foo");
}

/**
 * Deallocs memory used to the mapped regions
 */
void px_maps_clear(void) {
	if (ENV(maps) != NULL) {
		free(ENV(maps));
		ENV(maps) = NULL;
	}
	ENV(nregions) = 0;
	ELF(saddr) = 0;
	ELF(got) = 0;
}
