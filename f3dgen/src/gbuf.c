#include <string.h>
#include <stdint.h>

#include "gbuf.h"
#include "vertex.h"
#include "dlist.h"

VECTOR_IMPLDEF(tri_t, tri_vector_t, tri_vector_)

int gbuf_init(gbuf_t* gbuf, int max_vertices)
{
    tri_vector_init(&gbuf->tris);
    gbuf->max_vertices = max_vertices;
    gbuf->vidx = 0;
    //gbuf->full = 0;
    return 1;
}

int gbuf_free(gbuf_t* gbuf)
{
    tri_vector_free(&gbuf->tris);
    return 1;
}

int gbuf_clear(gbuf_t* gbuf)
{
    gbuf->vidx = 0;
    //gbuf->full = 0;
    tri_vector_clear(&gbuf->tris);
    return 1;
}

int gbuf_get_vertex_index(gbuf_t* gbuf, vertex_t* vertex)
{
    for(int vidx = 0; vidx < gbuf->vidx; vidx++)
    {
        if(memcmp(&gbuf->vbuf[vidx], vertex, sizeof(vertex_t)) == 0)
        {
            return vidx;
        }
    }
    return -1;
}

int gbuf_has_vertex(gbuf_t* gbuf, vertex_t* vertex)
{
    return gbuf_get_vertex_index(gbuf, vertex) != -1;
}

int gbuf_push_unique_vertex(gbuf_t* gbuf, vertex_t vertex)
{
    int existing_vidx = gbuf_get_vertex_index(gbuf, &vertex);
    
    if(existing_vidx != -1)
    {
        return existing_vidx;
    }

    if(gbuf->vidx < gbuf->max_vertices)
    {
        // push vertex and return its index
        gbuf->vbuf[gbuf->vidx] = vertex;
        return gbuf->vidx++;
    }

    // buffer is full
    return -1;
}

int gbuf_push_tri(gbuf_t* gbuf, int v1, int v2, int v3)
{
    tri_t tri;
    tri.v1 = v1;
    tri.v2 = v2;
    tri.v3 = v3;
    return tri_vector_push(&gbuf->tris, tri);
}

void gbuf_flush(gbuf_t* gbuf, dlist_t* dlist, vertex_list_t* vlist, uint32_t vertex_base_address)
{
    int vertex_list_offset = vertex_base_address + (vlist->length * sizeof(vertex_t));

    // push vertices to the vertex list
    for(int i = 0; i < gbuf->vidx; i++)
    {
        vertex_t vertex = gbuf->vbuf[i];
        vertex_list_push(vlist, vertex);
    }
    
    // push g_vtx command
    dlist_push_vtx(dlist, vertex_list_offset, gbuf->vidx, 0);

    // push tri commands
    for(int i = 0; i < gbuf->tris.length; i += 2)
    {
        tri_t tri_a = gbuf->tris.array[i];

        if(i + 1 >= gbuf->tris.length)
        {
            dlist_push_tri1(dlist, tri_a.v1, tri_a.v2, tri_a.v3);
            break;
        }
        
        tri_t tri_b = gbuf->tris.array[i + 1];
        dlist_push_tri2(dlist, tri_a.v1, tri_a.v2, tri_a.v3, tri_b.v1, tri_b.v2, tri_b.v3);
    }

    // kirby does this after a group of tri commands
    dlist_push_pipesync(dlist);

    gbuf_clear(gbuf);
}

int gbuf_is_empty(gbuf_t* gbuf)
{
    return (gbuf->vidx == 0);
}

int gbuf_has_room3(gbuf_t* gbuf, vertex_t* v0, vertex_t* v1, vertex_t* v2)
{
    // calculate space required in the vertex buffer for new vertices
    int have_v0 = gbuf_has_vertex(gbuf, v0);
    int have_v1 = gbuf_has_vertex(gbuf, v1);
    int have_v2 = gbuf_has_vertex(gbuf, v2);
    int num_new_verts = 3 - (have_v0 + have_v1 + have_v2);

    return ((gbuf->vidx + num_new_verts) <= gbuf->max_vertices);
}
