#ifndef CMDLINE_H
#define CMDLINE_H

typedef struct
{
	int argc;
	const char** argv;
	int pos;
	const char* cur_arg;
} cmd_args_t;

typedef int (*cmd_switch_handler_fn_t)(cmd_args_t* args, void* param);

typedef struct
{
	const char* switch_str;
	cmd_switch_handler_fn_t switch_fn;
} cmd_switch_handler_t;

cmd_switch_handler_fn_t cmd_switch_fn_lookup(const cmd_switch_handler_t* switch_handlers, const char* switch_str);
int cmd_args_over(cmd_args_t* args);
const char* cmd_args_pop(cmd_args_t* args);
int cmd_process(int argc, const char* argv[], const cmd_switch_handler_t* switch_handlers, cmd_switch_handler_fn_t default_fn, void* param);

#endif // CMDLINE_H
