#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

#include "obj.h"

int obj_load(obj_t* obj, const char* path)
{
    // Load obj file into text buffer
    FILE* fp = fopen(path, "rb");
	
	if(fp == NULL)
	{
		obj->text = NULL;
		obj->text_tok = NULL;
		obj->commands = NULL;
		return 0;
	}
   
	fseek(fp, 0, SEEK_END);
    obj->text_len = ftell(fp);
    rewind(fp);

    obj->text = malloc(obj->text_len + 1);
    obj->text[obj->text_len] = '\0';

    fread(obj->text, 1, obj->text_len, fp);
    fclose(fp);

    obj->text_tok = (char*)malloc(obj->text_len + 1);
    obj->text_tok[obj->text_len] = '\0';
    memcpy(obj->text_tok, obj->text, obj->text_len);

    // Prepare command buffer
    obj->num_command_slots = 256;
    obj->num_commands = 0;
    obj->commands = (obj_generic_command_t*) malloc(obj->num_command_slots * sizeof(obj_generic_command_t));

    // Parse, fill command buffer
    obj_run_parser(obj);

    return 1;
}

int obj_unload(obj_t* obj)
{
    free(obj->text);
    free(obj->text_tok);
    free(obj->commands);
    return 1;
}

int obj_push_generic_command(obj_t* obj, obj_generic_command_t generic_command)
{
    if(obj->num_commands + 1 > obj->num_command_slots)
    {
        obj->num_command_slots *= 2;
        obj->commands = (obj_generic_command_t*) realloc(obj->commands, obj->num_command_slots * sizeof(obj_generic_command_t));
    }
    obj->commands[obj->num_commands++] = generic_command;
    return obj->num_commands;
}

int obj_push_v_command(obj_t* obj, float x, float y, float z, float w)
{
    obj_generic_command_t command;
    command.id = OBJ_V;
    command.gcmd.v_command.x = x;
    command.gcmd.v_command.y = y;
    command.gcmd.v_command.z = z;
	command.gcmd.v_command.w = w;
    return obj_push_generic_command(obj, command);
}

int obj_push_vn_command(obj_t* obj, float i, float j, float k)
{
    obj_generic_command_t command;
    command.id = OBJ_VN;
    command.gcmd.vn_command.i = i;
    command.gcmd.vn_command.j = j;
    command.gcmd.vn_command.k = k;
    return obj_push_generic_command(obj, command);
}

int obj_push_vt_command(obj_t* obj, float u, float v)
{
    obj_generic_command_t command;
    command.id = OBJ_VT;
    command.gcmd.vt_command.u = u;
    command.gcmd.vt_command.v = v;
    return obj_push_generic_command(obj, command);
}

int obj_push_mtllib_command(obj_t* obj, const char* path)
{
    obj_generic_command_t command;
    command.id = OBJ_MTLLIB;
    command.gcmd.mtllib_command.path = path;
    return obj_push_generic_command(obj, command);
}

int obj_push_usemtl_command(obj_t* obj, const char* name)
{
    obj_generic_command_t command;
    command.id = OBJ_USEMTL;
    command.gcmd.usemtl_command.name = name;
    return obj_push_generic_command(obj, command);
}

int obj_push_o_command(obj_t* obj, const char* name)
{
    obj_generic_command_t command;
    command.id = OBJ_O;
    command.gcmd.o_command.name = name;
    return obj_push_generic_command(obj, command);
}

int obj_push_g_command(obj_t* obj, const char* name)
{
    obj_generic_command_t command;
    command.id = OBJ_G;
    command.gcmd.g_command.name = name;
    return obj_push_generic_command(obj, command);
}

int obj_push_f_command(obj_t* obj, int v1, int vt1, int vn1, int v2, int vt2, int vn2, int v3, int vt3, int vn3)
{
    obj_generic_command_t command;
    command.id = OBJ_F;
	
    command.gcmd.f_command.v1 = v1;
    command.gcmd.f_command.vt1 = vt1;
    command.gcmd.f_command.vn1 = vn1;
	
    command.gcmd.f_command.v2 = v2;
    command.gcmd.f_command.vt2 = vt2;
    command.gcmd.f_command.vn2 = vn2;
	
    command.gcmd.f_command.v3 = v3;
    command.gcmd.f_command.vt3 = vt3;
    command.gcmd.f_command.vn3 = vn3;
	
    return obj_push_generic_command(obj, command);
}


int obj_run_parser(obj_t* obj)
{
    // seek command token
	const char* token_str;
    while(obj_parser_extract_token(obj, &token_str, true))
    {
        // subparse command
        //printf("token: [%s]\n", token_str);
        if(!obj_parser_dispatch(obj, token_str))
        {
            //printf("UNHANDLED!\n");
        }
    }

    obj->text[obj->text_pos + 30] = 0;
    printf(obj->text + obj->text_pos);

    return 1;
}

int obj_parser_is_eof(obj_t* obj)
{
    return obj->text_pos >= obj->text_len;
}

int obj_parser_curc(obj_t* obj, char* curc)
{
    if(obj_parser_is_eof(obj))
    {
        return 0;
    }

    *curc = obj->text[obj->text_pos];
    return 1;
}

int obj_parser_inc_pos(obj_t* obj)
{
    if(obj_parser_is_eof(obj))
    {
        return 0;
    }
    obj->text_pos++;
    return 1;
}

// skip whitespace, comments
int obj_parser_seek_token(obj_t* obj, int incl_newline)
{
    char curc;

    while(obj_parser_curc(obj, &curc))
    {
        switch(curc)
        {
        case '\r':
        case '\n':
            if(!incl_newline)
            {
				// hit newline before token was found
                return 0; 
            }
        case '\t':
        case ' ':
            obj_parser_inc_pos(obj);
            break;
        case '#':
            // skip line comment
            while(obj_parser_inc_pos(obj) && obj_parser_curc(obj, &curc))
            {
                if(curc == '\n')
                {
                    break;
                }
            }
            break;
        default:
            return 1; // found token
        }
    }
    return 0; // reached eof
}

int obj_parser_require_char_token(obj_t* obj, char c)
{
	char curc;

	if(!obj_parser_seek_token(obj, false))
	{
		return 0;
	}
	
	if(obj_parser_curc(obj, &curc) && curc == c)
	{
		obj_parser_inc_pos(obj);
		return 1;
	}
	
	return 0;
}

int obj_parser_extract_token(obj_t* obj, const char** token_str, bool incl_newline)
{
	if(!obj_parser_seek_token(obj, incl_newline))
    {
        return 0;
    }

    int tok_pos = obj->text_pos;

    char curc;
    int end = 0;

    while(obj_parser_curc(obj, &curc) && !end)
    {
        switch(curc)
        {
        case '\r':
        case '\n':
        case '\t':
        case ' ':
        case '#':
            end = 1;
            break;
        default:
            // advance over token
            obj_parser_inc_pos(obj);
            break;
        }
    }

    // terminate the token in the text buffer copy
    obj->text_tok[obj->text_pos] = '\0';
    *token_str = &obj->text_tok[tok_pos];
    return 1;
}

int obj_parser_extract_int(obj_t* obj, int* value, bool incl_newline)
{
	if(!obj_parser_seek_token(obj, incl_newline))
    {
        return 0;
    }

	char* endptr;
	long int val = strtol(&obj->text[obj->text_pos], &endptr, 10);
	
	int length = endptr - &obj->text[obj->text_pos];
	
	if(length == 0)
	{
		return 0;
	}
	
	*value = val;
	
	obj->text_pos += length;
	return 1;
}

int obj_parser_handle_mtllib(obj_t* obj)
{
    const char* token_str;

   // printf("handling mtllib\n");
    if(!obj_parser_extract_token(obj, &token_str, false))
    {
        return 0;
    }
	
    obj_push_mtllib_command(obj, token_str);
    return 1;
}

int obj_parser_handle_v(obj_t* obj)
{
    const char* token_str;
    float x, y, z, w;

    //printf("handling v\n");
    if(!obj_parser_extract_token(obj, &token_str, false))
    {
        return 0;
    }
	
    x = atof(token_str);
	
    if(!obj_parser_extract_token(obj, &token_str, false))
    {
        return 0;
    }
	
    y = atof(token_str);

    if(!obj_parser_extract_token(obj, &token_str, false))
    {
        return 0;
    }

    z = atof(token_str);
	
	if(obj_parser_extract_token(obj, &token_str, false)) // optional w field
	{
		w = atof(token_str);
	}
	else
	{
		w = 1.0f;
	}
	//printf("done w\n");
    
    //printf("xyzw: %f %f %f %f\n", x, y, z, w);

    obj_push_v_command(obj, x, y, z, w);

    return 1;
}

int obj_parser_handle_vt(obj_t* obj)
{
    const char* token_str;
    float u, v;

    //printf("handling vt\n");
    if(!obj_parser_extract_token(obj, &token_str, false))
    {
        return 0;
    }
	
    u = atof(token_str);

    if(!obj_parser_extract_token(obj, &token_str, false))
    {
        return 0;
    }
	
    v = atof(token_str);
	
    //printf("uv: %f %f\n", u, v);
	obj_push_vt_command(obj, u, v);
	
	return 1;

}

int obj_parser_handle_vn(obj_t* obj)
{
    const char* token_str;
    float i, j, k;
	
    if(!obj_parser_extract_token(obj, &token_str, false))
    {
        return 0;
    }
	
    i = atof(token_str);

    if(!obj_parser_extract_token(obj, &token_str, false))
    {
        return 0;
    }
	
    j = atof(token_str);

    if(!obj_parser_extract_token(obj, &token_str, false))
    {
        return 0;
    }
	
    k = atof(token_str);
    
    //printf("ijk: %f %f %f\n", i, j, k);
	
    obj_push_vn_command(obj, i, j, k);

    return 1;
}

int obj_parser_handle_usemtl(obj_t* obj)
{
    const char* token_str;
	
    if(!obj_parser_extract_token(obj, &token_str, false))
    {
        return 0;
    }
	
	//printf("usemtl: [%s]\n", token_str);
    obj_push_usemtl_command(obj, token_str);
    return 1;
}

int obj_parser_handle_o(obj_t* obj)
{
    const char* token_str;
	
    if(!obj_parser_extract_token(obj, &token_str, false))
    {
        return 0;
    }
	
	//printf("o: [%s]\n", token_str);
    return obj_push_o_command(obj, token_str);
}

int obj_parser_handle_g(obj_t* obj)
{
    const char* token_str;
	
    if(!obj_parser_extract_token(obj, &token_str, false))
    {
        return 0;
    }
	
	//printf("g: [%s]\n", token_str);
    return obj_push_g_command(obj, token_str);
}

int obj_parser_handle_f(obj_t* obj)
{
	//char curc;
	
	int v1 = 0, vt1 = 0, vn1 = 0;
	int v2 = 0, vt2 = 0, vn2 = 0;
	int v3 = 0, vt3 = 0, vn3 = 0;
	
	// vertex 1
	
	if(!obj_parser_extract_int(obj, &v1, false))
	{
		return 0;
	}
	
	if(!obj_parser_require_char_token(obj, '/'))
	{
		return 0;
	}
	
	obj_parser_extract_int(obj, &vt1, false); // optional
	
	if(!obj_parser_require_char_token(obj, '/'))
	{
		return 0;
	}
	
	obj_parser_extract_int(obj, &vn1, false); // optional
	
	// vertex 2
	
	if(!obj_parser_extract_int(obj, &v2, false))
	{
		return 0;
	}
	
	if(!obj_parser_require_char_token(obj, '/'))
	{
		return 0;
	}
	
	obj_parser_extract_int(obj, &vt2, false); // optional
	
	if(!obj_parser_require_char_token(obj, '/'))
	{
		return 0;
	}
	
	obj_parser_extract_int(obj, &vn2, false); // optional
	
	// vertex 3
	
	if(!obj_parser_extract_int(obj, &v3, false))
	{
		return 0;
	}
	
	if(!obj_parser_require_char_token(obj, '/'))
	{
		return 0;
	}
	
	obj_parser_extract_int(obj, &vt3, false); // optional
	
	if(!obj_parser_require_char_token(obj, '/'))
	{
		return 0;
	}
	
	obj_parser_extract_int(obj, &vn3, false); // optional
	
	//printf("f: [%d %d %d] [%d %d %d] [%d %d %d]\n", v1, vt1, vn1, v2, vt2, vn2, v3, vt3, vn3);
	
	return obj_push_f_command(obj, v1, vt1, vn1, v2, vt2, vn2, v3, vt3, vn3);
}

int obj_parser_dispatch(obj_t* obj, const char* command_name)
{
    obj_parser_command_dispatch_t table[] = {
        { "mtllib", obj_parser_handle_mtllib },
        { "v",      obj_parser_handle_v },
		{ "vt",     obj_parser_handle_vt },
		{ "vn",     obj_parser_handle_vn },
		{ "usemtl", obj_parser_handle_usemtl},
		{ "o",      obj_parser_handle_o },
		{ "g",      obj_parser_handle_g },
		{ "f",      obj_parser_handle_f },
        { NULL, NULL }
    };

    for(int i = 0; table[i].command_name != NULL; i++)
    {
        if(strcmp(table[i].command_name, command_name) == 0)
        {
            table[i].subparser_func(obj);
            return 1;
        }
    }

    return 0;
}