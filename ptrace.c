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

#include <sys/ptrace.h>
#include <string.h>
#include "ptrace.h"
#include "cmd.h"

/**
 * Reads memory from the child process
 */
void ptrace_read(uintptr_t addr, void *vptr, size_t len) {
	const size_t long_size = sizeof(long);
	int i = 0, j = len / long_size, is_exact = len % long_size;
	long word;
	void *saddr = vptr;

	while (i <= j) {
		if (i == j && is_exact == 0) {
			break;
		}
		word = ptrace(PTRACE_PEEKTEXT, ENV(pid), addr + i * long_size, NULL);
		memcpy(saddr, &word, i == j ? (len % long_size) : long_size);
		saddr += long_size;
		++i;
	}
}
