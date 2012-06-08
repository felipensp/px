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
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include "common.h"
#include "cmd.h"
#include "trace.h"

px_env g_env;

#define PX_STRL(x) x, sizeof(x)-1

/**
 * Finds and call a handler if found
 */
static int _px_find_cmd(const px_command *cmd_list, char *cmd) {
	char *params, *op = strtok_r(cmd, " ", &params);
	const px_command *cmd_ptr = cmd_list;
	size_t op_len = op ? strlen(op) : 0;

	if (op) {
		while (cmd_ptr->cmd) {
			if (op_len == cmd_ptr->cmd_len
				&& memcmp(op, cmd_ptr->cmd, cmd_ptr->cmd_len) == 0) {
				cmd_ptr->handler(params);
				return 1;
			}
			++cmd_ptr;
		}
	}
	return 0;
}

/**
 * quit operation handler
 */
static void _px_quit_handler(const char *params) {
	printf("quit!\n");
	exit(0);
}

/**
 * attach operation handler
 * attach <pid>
 */
static void _px_attach_handler(const char *params) {
	pid_t pid = strtol(params, NULL, 10);

	if (kill(pid, 0) == -1) {
		g_env.pid = 0;
		px_error("Invalid process id `%d' (%s)", pid, strerror(errno));
		return;
	}
	g_env.pid = pid;
}

/**
 * trace operation handler
 * trace
 */
static void _px_trace_handler(const char *params) {
	if (g_env.pid == 0) {
		px_error("No program information to start tracing "
				"(use attach/run first)");
		return;
	}
	px_trace_pid(g_env.pid);
}

/**
 * Dumps all /proc/<pid>/maps information
 */
static void _px_show_maps_handler(const char *params) {
	char fname[PATH_MAX], buf[200];
	int fd, size;

	snprintf(fname, sizeof(fname), "/proc/%d/maps", g_env.pid);

	if ((fd = open(fname, O_RDONLY)) == -1) {
		px_error("Fail to open '%s'", fname);
		return;
	}

	while ((size = read(fd, buf, sizeof(buf)))) {
		printf("%.*s", size, buf);
	}

	close(fd);
}

/**
 * show operation handler
 * show <maps | perms>
 */
static void _px_show_handler(const char *params) {
	static const px_command _commands[] = {
		{PX_STRL("maps"), _px_show_maps_handler},
		{NULL, 0, NULL}
	};

	if (_px_find_cmd(_commands, (char*)params) == 0) {
		px_error("Command not found!");
	}
}

/**
 * General commands
 */
static const px_command commands[] = {
	{PX_STRL("quit"),   _px_quit_handler  },
	{PX_STRL("attach"), _px_attach_handler},
	{PX_STRL("trace"),  _px_trace_handler },
	{PX_STRL("show"),   _px_show_handler  },
	{NULL, 0, NULL}
};

/**
 * Prompt
 */
void px_prompt() {
	char cmd[PX_MAX_CMD_LEN];
	int ignore = 0, cmd_len;

	printf(PX_PROMPT);
	memset(cmd, 0, sizeof(cmd));

	while (fgets(cmd, PX_MAX_CMD_LEN, stdin) != NULL) {
		cmd_len = strlen(cmd) - 1;

		if (ignore == 1) {
			if (cmd[cmd_len] == '\n') {
				ignore = 0;
			}
			continue;
		}

		if (cmd[0] != '\n') {
			if (cmd[cmd_len] != '\n') {
				ignore = 1;
			} else {
				cmd[cmd_len] = '\0';
			}
			if (_px_find_cmd(commands, cmd) == 0) {
				px_error("Command not found!\n");
			}
		}
		printf(PX_PROMPT);
	}
}
