#ifndef GBUF_H
#define GBUF_H

// context for mapping vertices, g_vtx and g_tri commands

#include "vertex.h"
#include "vecdef.h"
#include "dlist.h"

typedef struct
{
    int v1, v2, v3;
} tri_t;

VECTOR_TYPEDEF(tri_t, tri_vector_t, tri_vector_)

typedef struct
{
    int max_vertices;
    int vidx;
    vertex_t vbuf[32];
    //int full; // non-zero = ready to flush to dlist and vlist
    tri_vector_t tris;
} gbuf_t;

int gbuf_init(gbuf_t* gbuf, int max_vertices);
int gbuf_free(gbuf_t* gbuf);
int gbuf_clear(gbuf_t* gbuf);
int gbuf_is_empty(gbuf_t* gbuf);
int gbuf_get_vertex_index(gbuf_t* gbuf, vertex_t* vertex);
int gbuf_has_vertex(gbuf_t* gbuf, vertex_t* vertex);
int gbuf_push_unique_vertex(gbuf_t* gbuf, vertex_t vertex);
int gbuf_push_tri(gbuf_t* gbuf, int v1, int v2, int v3);
void gbuf_flush(gbuf_t* gbuf, dlist_t* dlist, vertex_list_t* vlist, uint32_t vertex_base_address);
int gbuf_has_room3(gbuf_t* gbuf, vertex_t* v0, vertex_t* v1, vertex_t* v2);



#endif // GBUF_H
