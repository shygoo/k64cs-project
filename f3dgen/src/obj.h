#ifndef OBJ_H
#define OBJ_H

#include <stdbool.h>

#include "vecdef.h"

typedef enum
{
    OBJ_MTLLIB,
    OBJ_V,
    OBJ_VT,
    OBJ_VN,
    OBJ_USEMTL,
    OBJ_O,
    OBJ_G,
    OBJ_F // f  v1/vt1/vn1   v2/vt2/vn2   v3/vt3/vn3
} obj_command_id_t;

typedef struct
{
    float x, y, z, w;
} obj_v_command_t;

typedef struct
{
    float i, j, k;
} obj_vn_command_t;

typedef struct
{
	float u, v;
} obj_vt_command_t;

typedef struct
{
    const char* path;
} obj_mtllib_command_t;

typedef struct
{
    const char* name;
} obj_g_command_t;

typedef struct
{
    const char* name;
} obj_o_command_t;

typedef struct
{
    const char* name;
} obj_usemtl_command_t;

typedef struct
{
	int v1, vt1, vn1;
	int v2, vt2, vn2;
	int v3, vt3, vn3;
} obj_f_command_t;

typedef struct
{
    obj_command_id_t id;
    union
    {
        obj_v_command_t      v_command;
        obj_mtllib_command_t mtllib_command;
		obj_vt_command_t     vt_command;
		obj_vn_command_t     vn_command;
		obj_g_command_t      g_command;
		obj_o_command_t      o_command;
		obj_usemtl_command_t usemtl_command;
		obj_f_command_t      f_command;
    } gcmd;
} obj_generic_command_t;

typedef struct
{
    char* text;
    char* text_tok;
    int text_len;
    int text_pos;

    obj_generic_command_t* commands;
    int num_commands;
    int num_command_slots;
} obj_t;

typedef int (*obj_subparser_func_t)(obj_t* obj);

typedef struct
{
    const char* command_name;
    obj_subparser_func_t subparser_func;
} obj_parser_command_dispatch_t;

int obj_load(obj_t* obj, const char* path);
int obj_unload(obj_t* obj);
int obj_push_generic_command(obj_t* obj, obj_generic_command_t generic_command);
int obj_push_v_command(obj_t* obj, float x, float y, float z, float w);
int obj_push_mtllib_command(obj_t* obj, const char* path);
int obj_push_vt_command(obj_t* obj, float u, float v);
int obj_push_vn_command(obj_t* obj, float i, float j, float k);
int obj_push_f_command(obj_t* obj, int v1, int vt1, int vn1, int v2, int vt2, int vn2, int v3, int vt3, int vn3);

int obj_run_parser(obj_t* obj);
int obj_parser_is_eof(obj_t* obj);
int obj_parser_curc(obj_t* obj, char* curc);
int obj_parser_inc_pos(obj_t* obj);
int obj_parser_seek_token(obj_t* obj, int incl_newline);
int obj_parser_extract_token(obj_t* obj, const char** token_str, bool incl_newline);
int obj_parser_extract_int(obj_t* obj, int* value, bool incl_newline);
int obj_parser_require_char_token(obj_t* obj, char c);

int obj_parser_handle_mtllib(obj_t* obj);
int obj_parser_handle_v(obj_t* obj);
int obj_parser_handle_vt(obj_t* obj);
int obj_parser_handle_vn(obj_t* obj);
int obj_parser_handle_usemtl(obj_t* obj);
int obj_parser_handle_o(obj_t* obj);
int obj_parser_handle_g(obj_t* obj);
int obj_parser_handle_f(obj_t* obj);

int obj_parser_dispatch(obj_t* obj, const char* command_name);

#endif // OBJ_H
