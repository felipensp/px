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

#ifndef PX_ELF
#define PX_ELF

/**
 * ELF information
 */
typedef struct _px_elf {
	uintptr_t header; /* base address */
	uintptr_t got;    /* GOT address */
	uintptr_t strtab; /* strtab address */
	uintptr_t symtab; /* symtab address */
	uintptr_t map;    /* link_map address */
} px_elf;

/**
 * Helper macro to access ELF information in the g_env global var
 */
#define ELF(x) ENV(elf.x)

void px_elf_maps(void);
int px_elf_find_symbol(const char*);
void px_elf_clear(void);
void px_elf_dump_sections(void);
void px_elf_dump_pheaders(void);

#endif /* PX_ELF */
