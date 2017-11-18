#ifndef DAEF3D_H
#define DAEF3D_H

#include "vecdef.h"
#include "vertex.h"
#include "dae.h"
#include "dlist.h"
#include "gbuf.h"

VECTOR_TYPEDEF(dlist_t, dlist_vec_t, dlist_vec_)

typedef struct
{
    ucode_t ucode;
    vertex_list_t vertices; // heap of all vertices
    dlist_vec_t   dlists; // one dlist per geometry
} gfx_archive_t; // collection of dlists, vertices, images

int vertex_init_from_vectors(vertex_t* v, vec3f_t position, vec3f_t color_or_normal);
int gar_add_dae_geometry(gfx_archive_t* gar, dae_geometry_t* geometry, gbuf_t* gbuf, uint32_t vbase, double vscale);
int gar_init(gfx_archive_t* gar, ucode_t ucode);
int gar_load_dae(gfx_archive_t* gar, const char* path, uint32_t vbase, double vscale);

#endif // DAEF3D_H