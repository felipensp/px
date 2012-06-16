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
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "maps.h"
#include "cmd.h"
#include "ptrace.h"
#include "elf.h"

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
	ELF(header) = ENV(maps)[0].start;

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
 * Deallocs memory used to the mapped regions
 */
void px_maps_clear(void) {
	if (ENV(maps) != NULL) {
		free(ENV(maps));
		ENV(maps) = NULL;
	}
	ENV(nregions) = 0;
	
	px_elf_clear();
}
