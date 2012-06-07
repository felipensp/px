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
#include "common.h"
#include "cmd.h"
#include "trace.h"

px_env g_env;

/**
 * quit operation handler
 */
static void px_quit_handler(const char* params) {
	printf("quit!\n");
	exit(0);
}

/**
 * attach operation handler
 * attach <pid>
 */
static void px_attach_handler(const char* params) {
	long pid = strtol(params, NULL, 10);

	g_env.trace.mode = TRACE_BY_PID;
	g_env.trace.data.pid = pid;
}

/**
 * run operation handler
 * run <file> <args>
 */
static void px_run_handler(const char* params) {
	char *args, *proc = strtok_r((char*)params, " ", &args);

	g_env.trace.mode = TRACE_BY_PID;
	g_env.trace.data.prog[0] = strdup(proc);
	g_env.trace.data.prog[1] = args ? strdup(args) : NULL;
}

/**
 * trace operation handler
 * trace
 */
static void px_trace_handler(const char* params) {
	switch (g_env.trace.mode) {
		case TRACE_NONE:
			px_error("No program information to start tracing "
					"(use attach/run first)");
			break;
		case TRACE_BY_NAME:
			px_trace_prog(g_env.trace.data.prog[0], g_env.trace.data.prog[1]);
			break;
		case TRACE_BY_PID:
			px_trace_pid(g_env.trace.data.pid);
			break;
	}
}

/**
 * Deallocs dynamic memory alloc'ed to the environment data
 */
static void px_free_env() {
	if (g_env.trace.mode == TRACE_BY_NAME &&
		g_env.trace.data.prog[0] != NULL) {
		px_safe_free(g_env.trace.data.prog[0]);
		px_safe_free(g_env.trace.data.prog[1]);
	}
}

/**
 * Commands
 */
#define PX_STRL(x) x, sizeof(x)-1
static const px_command commands[] = {
	{PX_STRL("quit"),   px_quit_handler  },
	{PX_STRL("attach"), px_attach_handler},
	{PX_STRL("run"),    px_run_handler   },
	{PX_STRL("trace"),  px_trace_handler },
	{NULL, 0, NULL}
};
#undef PX_STRL

/**
 * Handles the command execution
 */
static void handle_cmd(char *cmd) {
	char *params, *op = strtok_r(cmd, " ", &params);
	const px_command *cmd_ptr = commands;
	size_t op_len = op ? strlen(op) : 0;

	if (op) {
		while (cmd_ptr->cmd) {
			if (op_len == cmd_ptr->cmd_len
				&& memcmp(op, cmd_ptr->cmd, cmd_ptr->cmd_len) == 0) {
				cmd_ptr->handler(params);
				return;
			}
			++cmd_ptr;
		}
	}

	px_error("Command not found!\n");
}

/**
 * Prompt
 */
void prompt_cmd() {
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
			handle_cmd(cmd);
		}
		printf(PX_PROMPT);
	}

	px_free_env();
}
