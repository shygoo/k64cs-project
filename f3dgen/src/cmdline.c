#include <string.h>
#include <stdio.h>

#include "cmdline.h"

cmd_switch_handler_fn_t cmd_switch_fn_lookup(const cmd_switch_handler_t* switch_handlers, const char* switch_str)
{
	for(int i = 0; switch_handlers[i].switch_str != NULL; i++)
	{
		if(strcmp(switch_handlers[i].switch_str, switch_str) == 0)
		{
			return switch_handlers[i].switch_fn;
		}
	}
	return NULL;
}

int cmd_args_over(cmd_args_t* args)
{
	return args->pos >= args->argc;
}

const char* cmd_args_pop(cmd_args_t* args)
{
	if(!cmd_args_over(args))
	{
		args->cur_arg = args->argv[args->pos++];
		return args->cur_arg;
	}
	return NULL;
}

int cmd_process(int argc, const char* argv[], const cmd_switch_handler_t* switch_handlers, cmd_switch_handler_fn_t default_fn, void* param)
{
    cmd_args_t args;
	args.argc = argc;
	args.argv = argv;
	args.pos = 1;
	args.cur_arg = NULL;

    while(!cmd_args_over(&args))
    {
        const char* arg = cmd_args_pop(&args);
		
        if(arg[0] == '-' && arg[1])
        {
            const char* switch_str = &arg[1];
            cmd_switch_handler_fn_t switch_fn = cmd_switch_fn_lookup(switch_handlers, switch_str);

            if(switch_fn == NULL)
            {
                printf("error: invalid switch '%s'\n", switch_str);
				return 0;
            }

            if(!switch_fn(&args, param))
			{
				return 0;
			}
        }
		else
		{
			if(!default_fn(&args, param))
			{
				return 0;
			}
		}
    }
    return 1;
}
