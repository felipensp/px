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
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include "maps.h"
#include "cmd.h"

/**
 * Parses an line from /proc/<pid>/maps
 */
void px_maps_region(const char *line) {
	uintptr_t start, end;
	char perms[5], filename[PATH_MAX];
	int offset, dmajor, dminor, inode;

	if (sscanf(line, "%lx-%lx %s %x %x:%x %u %s",
		&start, &end, perms, &offset, &dmajor, &dminor, &inode, filename) < 6 ||
		(end - start) == 0) {
		return;
	}

	if (g_env.maps == NULL || g_env.nregions % 5 == 0) {
		g_env.maps = realloc(g_env.maps, sizeof(px_maps) * (g_env.nregions + 5));
	}

	g_env.maps[g_env.nregions].start = start;
	g_env.maps[g_env.nregions].end = end;
	memcpy(g_env.maps[g_env.nregions].filename, filename, sizeof(filename));
	memcpy(g_env.maps[g_env.nregions].perms, perms, sizeof(perms));

	++g_env.nregions;
}

/**
 * Finds a mapped region by address
 */
int px_maps_find_region(uintptr_t addr) {
	int i = 0;

	while (i < g_env.nregions) {
		if (g_env.maps[i].start <= addr && g_env.maps[i].end >= addr) {
			printf("Found... %s (%s)\n",
				g_env.maps[i].filename, g_env.maps[i].perms);
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
	if (g_env.maps != NULL) {
		free(g_env.maps);
		g_env.maps = NULL;
	}
	g_env.nregions = 0;
}
