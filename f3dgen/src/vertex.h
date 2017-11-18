#ifndef VERTEX_H
#define VERTEX_H

#include <stdint.h>

#include "vecdef.h"

typedef struct {
    int16_t x, y, z;
    uint16_t _unused;
    int16_t u, v;
    union
    {
        struct
        {
            uint8_t r, g, b, a;
        } color;
        struct
        {
            int8_t x, y, z;
            uint8_t alpha;
        } normal;
    };
} vertex_t;

typedef struct
{
    int16_t u, v;
} tex_uv_t;

typedef struct
{
    int8_t x, y, z;
} normal_t;

VECTOR_TYPEDEF ( vertex_t, vertex_list_t, vertex_list_ )
VECTOR_TYPEDEF ( tex_uv_t, tex_uv_list_t, tex_uv_list_ )
VECTOR_TYPEDEF ( normal_t, normal_list_t, normal_list_ )

typedef struct
{
    vertex_list_t vertex_list;
    normal_list_t normal_list;
    tex_uv_list_t tex_uv_list;
} vertex_info_t;

int vertex_bswap(vertex_t* vertex);
int vertex_list_bswap(vertex_list_t* vertex_list);
int vertex_list_dump(vertex_list_t* vertex_list, const char* path);
int vertex_info_init(vertex_info_t* gfx);
int vertex_info_free(vertex_info_t* gfx);

#endif // VERTEX_H
