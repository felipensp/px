#ifndef PX_CMD
#define PX_CMD

#define PX_STRL(x) x, sizeof(x)
#define PX_PROMPT "px!> "
#define PX_MAX_CMD_LEN 100

typedef void (*px_command_handler)(void);

typedef struct _px_command {
	const char *cmd;
	size_t cmd_len;
	px_command_handler handler;
} px_command;

#endif /* PX_CMD */
