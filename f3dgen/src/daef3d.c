#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "daef3d.h"

#include "vecdef.h"
#include "vertex.h"
#include "dae.h"
#include "dlist.h"
#include "gbuf.h"

VECTOR_IMPLDEF(dlist_t, dlist_vec_t, dlist_vec_)

int vertex_init_from_vectors(vertex_t* v, vec3f_t position, vec3f_t color_or_normal)
{
    memset(v, 0x00, sizeof(vertex_t));

    v->x = (int16_t)position.x;
    v->y = (int16_t)position.y;
    v->z = (int16_t)position.z;

    v->color.r = (uint8_t)(0xFF * color_or_normal.x);
    v->color.g = (uint8_t)(0xFF * color_or_normal.y);
    v->color.b = (uint8_t)(0xFF * color_or_normal.z);
    v->color.a = 0xFF; // :(

    return 1;
}

int gar_add_dae_geometry(gfx_archive_t* gar, dae_geometry_t* geometry, gbuf_t* gbuf, uint32_t vbase, double vscale)
{
    dlist_t dlist;

    gbuf_clear(gbuf);

    dlist_init(&dlist, gar->ucode);

    dlist_push_pipesync(&dlist);
    dlist_push_tilesync(&dlist);

    dlist_push_geometrymode(&dlist, G_ALL, G_LIGHTING); // kirby seems to do this
    dlist_push_command(&dlist, (dlist_command_t){0xFCFFFFFF, 0xFFFE793C});

    for(int i = 0; i < geometry->num_faces; i++)
    {
        vertex_t v0, v1, v2;

        vec3f_t face_colors[3] = {0};
        vec3f_t face_positions[3] = {0};
        vec3f_t face_normals[3] = {0};
        
        dae_geometry_get_face_vertices(geometry, i, face_positions);

        // todo prioritize colors over normals

        if(geometry->normals_offset != DAE_NO_OFFSET)
        {
            dae_geometry_get_face_normals(geometry, i, face_normals);
        }
        
        if(geometry->colors_offset != DAE_NO_OFFSET)
        {
            dae_geometry_get_face_colors(geometry, i, face_colors);
        }

        vertex_init_from_vectors(&v0, face_positions[0], face_colors[0]);
        vertex_init_from_vectors(&v1, face_positions[1], face_colors[1]);
        vertex_init_from_vectors(&v2, face_positions[2], face_colors[2]);
        
        if(!gbuf_has_room3(gbuf, &v0, &v1, &v2))
        {
            // no more room in the vbuf for this triangle
            // flush commands to dlist, vertex copies to vlist
            gbuf_flush(gbuf, &dlist, &gar->vertices, vbase);
        }

        int vidx0 = gbuf_push_unique_vertex(gbuf, v0);
        int vidx1 = gbuf_push_unique_vertex(gbuf, v1);
        int vidx2 = gbuf_push_unique_vertex(gbuf, v2);
        gbuf_push_tri(gbuf, vidx0, vidx1, vidx2);
    }

    if(!gbuf_is_empty(gbuf))
    {
        // flush remainder
        gbuf_flush(gbuf, &dlist, &gar->vertices, vbase);
    }

    dlist_push_geometrymode(&dlist, G_ALL, G_LIGHTING); // kirby seems to do this
    dlist_push_enddl(&dlist);

    // add the dlist to the graphics archive
    dlist_vec_push(&gar->dlists, dlist);

    return 1;
}

int gar_init(gfx_archive_t* gar, ucode_t ucode)
{
    gar->ucode = ucode;
    vertex_list_init(&gar->vertices);
    dlist_vec_init(&gar->dlists);
    return 1;
}

int gar_load_dae(gfx_archive_t* gar, const char* path, uint32_t vbase, double vscale)
{
    dae_t dae;
    gbuf_t gbuf;
    
    if(!dae_load(&dae, path))
    {
        return 0;
    }

    gbuf_init(&gbuf, 32);

    for(int i = 0; i < dae.geometries.length; i++)
    {
        dae_geometry_t* geometry = &dae.geometries.array[i];
        gar_add_dae_geometry(gar, geometry, &gbuf, vbase, vscale);
    }

    dae_unload(&dae);
    gbuf_free(&gbuf);

    return 1;
}
