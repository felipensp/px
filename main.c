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
#include <getopt.h>
#include "cmd.h"

#define PX_VERSION "0.1.0"

/**
 * Commands
 */
static const px_command commands[] = {
	{PX_STRL("quit"), px_quit_handler},
	{NULL, 0, NULL}
};

/**
 * Displays the usage of px
 */
static void usage() {
	printf("Usage: px [options]\n\n"
			"Options:\n"
			"	-h, --help	Displays this information\n");
}

/**
 * Handles the command execution
 */
void handle_cmd(char *cmd) {
	char *params, *op = strtok_r(cmd, " ", &params);
	const px_command *cmd_ptr = commands;

	while (cmd_ptr->cmd) {
		if (memcmp(op, cmd_ptr->cmd, cmd_ptr->cmd_len) == 0) {
			cmd_ptr->handler();
		}
		cmd_ptr++;
	}

	printf("Command not found!\n");
}

int main(int argc, char **argv) {
	char c, cmd[PX_MAX_CMD_LEN];
	int ignore = 0, cmd_len, opt_index = 0;
	static struct option long_opts[] = {
		{"help", no_argument, 0, 'h'},
		{0, 0, 0, 0}
	};

	printf("px - Process Examinator (" PX_VERSION ")\n");

	if (argc > 1) {
		while ((c = getopt_long(argc, argv, "h", long_opts, &opt_index)) != -1) {
			switch (c) {
				case 'h':
					usage();
					exit(0);
				case '?':
					break;
			}
		}
	}

	printf(PX_PROMPT);
	memset(cmd, 0, PX_MAX_CMD_LEN+1);

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
		} else {
			ignore = 0;
		}
		printf(PX_PROMPT);
	}

	return 0;
}