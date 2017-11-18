#ifndef DAE_H
#define DAE_H

#include "vecprim.h"
#include "xml.h"

#define DAE_NO_OFFSET (-1)

typedef struct
{
	// todo material index
	const char* id;
	float_vec_t vertices, normals, colors, texcoords;
	int         vertices_offset, normals_offset, colors_offset, texcoords_offset;
	int         num_inputs;
	int_vec_t   points;
	int num_points;
	int num_faces;
} dae_geometry_t;

VECTOR_TYPEDEF(dae_geometry_t, dae_geometry_vec_t, dae_geometry_vec_)

typedef struct
{
    xml_t xml;
	dae_geometry_vec_t geometries;
} dae_t;

typedef struct
{
    float x, y, z;
} vec3f_t;

typedef struct
{
    float m[4][4];
} mat4f_t;

int dae_load(dae_t* dae, const char* path);
int dae_unload(dae_t* dae);

int dae_report(dae_t* dae); // debug

int dae_geometry_init(dae_geometry_t* dae_geometry, const char* id);
int dae_geometry_free(dae_geometry_t* dae_geometry);

int dae_init_geometries(dae_t* dae);
int dae_free_geometries(dae_t* dae);

int dae_geometry_get_vertex_vec3(dae_geometry_t* dae_geometry, int point_index, vec3f_t* vertex, double vscale);
int dae_geometry_get_normal_vec3(dae_geometry_t* dae_geometry, int point_index, vec3f_t* normal);
int dae_geometry_get_color_vec3(dae_geometry_t* dae_geometry, int point_index, vec3f_t* color);
int dae_geometry_get_face_vertices(dae_geometry_t* dae_geometry, int face_idx, vec3f_t* vertices, double vscale);
int dae_geometry_get_face_normals(dae_geometry_t* dae_geometry, int face_idx, vec3f_t* normals);
int dae_geometry_get_face_colors(dae_geometry_t* dae_geometry, int face_idx, vec3f_t* colors);

int float_vec_push_from_string(float_vec_t* fvec, const char* str);
int int_vec_push_from_string(int_vec_t* ivec, const char* str);
int mat4f_init_from_string(mat4f_t* m, const char* str);
int vec3f_transform(vec3f_t* v, mat4f_t* m);

int float_vec_print(float_vec_t* fvec, int width, int height); // debug
int int_vec_print(int_vec_t* ivec, int width, int height); // debug
void mat4f_print(mat4f_t* m); // debug

#endif // DAE_H
