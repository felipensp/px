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
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include "common.h"
#include "trace.h"
#include "cmd.h"

/**
 * Attaches to an specified pid
 */
void px_attach_pid(void) {
	int stat;

	printf("[+] Attaching to pid %d\n", ENV(pid));

	if (ptrace(PTRACE_ATTACH, ENV(pid), NULL, NULL) == -1) {
		px_error("Failed to attach to pid (%s)", strerror(errno));
		return;
	}

	if (waitpid(ENV(pid), &stat, WNOHANG) != ENV(pid) || !WIFSTOPPED(stat)) {
		px_error("Unexpected wait result (%s)", strerror(errno));
	}
}

/**
 * Detaches from an previously attached pid
 */
void px_detach_pid(void) {
	printf("[+] Detaching from pid %d\n", ENV(pid));

	if (ptrace(PTRACE_DETACH, ENV(pid), NULL, NULL) == -1) {
		px_error("Failed to detach from pid (%s)", strerror(errno));
		return;
	}

	ENV(pid) = 0;
}

/**
 * Sends a signal to the attached child process
 */
void px_send_signal(int signum) {
	int stat;

	printf("[+] Sending signal %d to attached process\n", signum);

	if (kill(ENV(pid), signum) == -1) {
		px_error("%s", strerror(errno));
		return;
	}

	waitpid(ENV(pid), &stat, WNOHANG);

	if (WIFEXITED(stat)) {
		printf("Child status: %d (Exited with status: %d)\n",
			stat, WEXITSTATUS(stat));

		ENV(pid) = 0;
	} else {
		printf("Child status: %d\n", stat);
	}
}
