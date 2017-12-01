#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "dae.h"
#include "obj.h"
#include "vertex.h"
#include "dlist.h"
#include "gbuf.h"
#include "objf3d.h"
#include "cmdline.h"
#include "daef3d.h"

typedef struct
{
	const char* input_path;
	const char* root_path;
	const char* output_path;
	const char* vertex_output_path;
	uint32_t    vertex_base_address;
	double      vertex_scale;
	ucode_t     ucode;
} options_t;

int switch_o(cmd_args_t* args, void* p)
{
    options_t* options = (options_t*)p;

	const char* path = cmd_args_pop(args);

	if(path == NULL)
	{
        printf("-o requires path");
        return 0;
    }
    options->output_path = path;

	return 1;
}

int switch_ov(cmd_args_t* args, void* p)
{
	options_t* options = (options_t*)p;
	
	const char* path = cmd_args_pop(args);
	
	if(path == NULL)
	{
		printf("-ov requires path");
		return 0;
	}
	options->vertex_output_path = path;
	return 1;
}

int switch_ucode(cmd_args_t* args, void* p)
{
	options_t* options = (options_t*)p;
	
	const char* ucode_name = cmd_args_pop(args);
	
	if(ucode_name == NULL)
	{
		printf("-ucode requires ucode name\n");
		return 0;
	}
	
	if(strcmp(ucode_name, "F3DEX2") == 0)
	{
		options->ucode = UCODE_F3DEX2;
	}
	else
	{
		printf("invalid ucode\n");
		return 0;
	}
	
	//printf("vertex ouput path: %s\n", path);
	//options->vertex_output_path = path;
	return 1;
}

int switch_root(cmd_args_t* args, void* p)
{
	options_t* options = (options_t*)p;
	
	const char* root_path = cmd_args_pop(args);
	
	if(root_path == NULL)
	{
		printf("-root requires path\n");
		return 0;
	}
	
	options->root_path = root_path;
	return 1;
}

int switch_vbase(cmd_args_t* args, void* p)
{
	options_t* options = (options_t*)p;
	const char* vbase_str = cmd_args_pop(args);
	
	if(vbase_str == NULL)
	{
		printf("error: -vbase requires vertex base address\n");
		return 0;
	}
	
	uint32_t addr = strtoll(vbase_str, NULL, 0);
	
	options->vertex_base_address = addr;
	return 1;
}

int switch_vscale(cmd_args_t* args, void* p)
{
	options_t* options = (options_t*)p;
	const char* vscale_str = cmd_args_pop(args);
	
	if(vscale_str == NULL)
	{
		printf("error: -vscale requires decimal vertex scale\n");
		return 0;
	}
	
	double scale = strtod(vscale_str, NULL);
	options->vertex_scale = scale;
	return 1;
}

int arg_default(cmd_args_t* args, void* p)
{
	options_t* options = (options_t*)p;
	if(options->input_path != NULL)
	{
		printf("error: input path already set\n");
		return 0;
	}
	options->input_path = args->cur_arg;
	return 1;
}

void print_usage_guide()
{
	printf (
		"f3dgen - Fast3D generator                                                        \n"
		"                                                                                 \n"
		"f3dgen <input_path> [options]                                                    \n"
		"                                                                                 \n"
		"  -uc            <ucode_class>                                                   \n"
		"                  Sets the microcode class                                       \n"
		"                  Valid options: F3DEX2                                          \n"
		"                                                                                 \n"
		"  -o             <ucode_output_path>                                             \n"
		"                  Outputs Fast3D microcode data                                  \n"
		"                                                                                 \n"
		"  -ov            <vertices_output_path>                                          \n"
		"                  Outputs vertex data                                            \n"
		"                                                                                 \n"
		"  -o-vtx-rel     <vtx_relocations_output_path>                                   \n"
		"                  Outputs list of offsets to every G_VTX command in the ucode    \n"
		"                                                                                 \n"
		"  -o-settimg-rel <settimg_relocations_ouput_path>                                \n"
		"                  Outputs list of offsets to every G_SETTIMG command in the ucode\n"
		"                                                                                 \n"
		"  -base          <vertex_base_address>                                           \n"
		"                  Sets the base address that G_VTX will use for vertices         \n"
		"                                                                                 \n"
		"Input may be one of the following formats: Collada DAE (*.dae)\n"
	);
}

int main(int argc, const char* argv[])
{
	options_t options;
	//dae_t dae;
	//dlist_t dlist;
	//vertex_list_t new_vertices; // output vertices
	gfx_archive_t gar;

	static cmd_switch_handler_t switches[] = {
		{ "o",      switch_o },
		{ "ov",     switch_ov },
		{ "uc",     switch_ucode },
		{ "root",   switch_root },
		{ "vbase",  switch_vbase },
		{ "vscale", switch_vscale },
		{ NULL, NULL }
    };
	
	if(argc < 2)
	{
		print_usage_guide();
		return EXIT_FAILURE;
	}
	
	options.root_path = NULL;
	options.input_path = NULL;
	options.output_path = NULL;
	options.vertex_output_path = NULL;
	options.vertex_base_address = 0;
	options.vertex_scale = 1.0f;
	options.ucode = UCODE_F3DEX2;
    
    if(!cmd_process(argc, argv, switches, arg_default, &options))
	{
		return EXIT_FAILURE;
	}

	if(options.input_path == NULL)
	{
		printf("error: no input path\n");
		return EXIT_FAILURE;
	}
	
	gar_init(&gar, options.ucode);
	gar_load_dae(&gar, options.input_path, options.vertex_base_address, options.vertex_scale);

	printf("%d dlists\n", gar.dlists.length);

	char path[256];
	for(int i = 0; i < gar.dlists.length; i++)
	{
		sprintf(path, "%s_f3d_%d.bin", options.output_path, i);
		printf("%s\n", path);
		dlist_bswap(&gar.dlists.array[i]);
		dlist_dump(&gar.dlists.array[i], path);
	}

	sprintf(path, "%s_vertices.bin", options.output_path);
	printf("%s\n", path);

	vertex_list_bswap(&gar.vertices);
	vertex_list_dump(&gar.vertices, path);

    //dlist_init(&dlist, options.ucode);
    //vertex_list_init(&new_vertices);
	
	//dae_report(&dae); // test
	//return EXIT_SUCCESS;
	
	//obj_to_f3d(&obj, &dlist, &new_vertices, options.vertex_base_address, options.vertex_scale);

	//if(options.output_path != NULL)
	//{
	//	dlist_bswap(&dlist);
	//	dlist_dump(&dlist, options.output_path);
	//}
	//
	//if(options.vertex_output_path != NULL)
	//{
	//	vertex_list_bswap(&new_vertices);
	//	vertex_list_dump(&new_vertices, options.vertex_output_path);
	//}

	//dae_unload(&dae);
	
    //vertex_list_free(&new_vertices);
   // dlist_free(&dlist);
}