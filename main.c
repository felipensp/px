#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmd.h"

void px_quit_handler(void) {
	printf("quit!\n");
	exit(0);
}

static px_command commands[] = {
	{ PX_STRL("quit"), px_quit_handler },
	{ NULL, 0, NULL }
};

/**
 * Handles the command execution
 */
void handle_cmd(char *cmd, size_t cmd_len) {
	char *params, *op = strtok_r(cmd, " ", &params);
	px_command *cmd_ptr = commands;

	while (cmd_ptr->cmd) {
		if (memcmp(op, cmd_ptr->cmd, cmd_ptr->cmd_len) == 0) {
			cmd_ptr->handler();
		}
		cmd_ptr++;
	}

	printf("Command not found!\n");
}

int main(int argc, char **argv) {
	char cmd[PX_MAX_CMD_LEN+1];
	char *cmd_ptr = cmd;
	char c;

	memset(cmd, 0, PX_MAX_CMD_LEN+1);

	printf(PX_PROMPT);

	while (1) {
		c = getchar();

		if (c == '\n') {
			if (cmd_ptr != cmd) {
				*cmd_ptr = '\0';
				handle_cmd(cmd, cmd_ptr - cmd);
				cmd_ptr = cmd;
			}
			printf(PX_PROMPT);
			continue;
		}
		*cmd_ptr++ = c;
	}

	return 0;
}
