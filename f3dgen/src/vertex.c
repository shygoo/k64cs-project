#include <stdint.h>
#include <stdio.h>

#include "vertex.h"
#include "bswap.h"

VECTOR_IMPLDEF ( vertex_t, vertex_list_t, vertex_list_ )
VECTOR_IMPLDEF ( tex_uv_t, tex_uv_list_t, tex_uv_list_ )
VECTOR_IMPLDEF ( normal_t, normal_list_t, normal_list_ )

int vertex_bswap(vertex_t* vertex)
{
	vertex->x = bswap16(vertex->x);
	vertex->y = bswap16(vertex->y);
	vertex->z = bswap16(vertex->z);
	
	vertex->u = bswap16(vertex->u);
	vertex->v = bswap16(vertex->v);
	return 1;
}

int vertex_list_bswap(vertex_list_t* vertex_list)
{
	return vertex_list_iterate(vertex_list, vertex_bswap);
}

int vertex_list_dump(vertex_list_t* vertex_list, const char* path)
{
	FILE* fp = fopen(path, "wb");
	fwrite(vertex_list->array, vertex_list_size_bytes(vertex_list), 1, fp);
	fclose(fp);
	return 1;
}

int vertex_info_init(vertex_info_t* gfx)
{
    vertex_list_init(&gfx->vertex_list);
    normal_list_init(&gfx->normal_list);
    tex_uv_list_init(&gfx->tex_uv_list);
    return 1;
}

int vertex_info_free(vertex_info_t* gfx)
{
    vertex_list_free(&gfx->vertex_list);
    normal_list_free(&gfx->normal_list);
    tex_uv_list_free(&gfx->tex_uv_list);
    return 1;
}
