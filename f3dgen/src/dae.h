#ifndef DAE_H
#define DAE_H

#include "vecprim.h"
#include "xml.h"

#define DAE_NO_OFFSET (-1)

typedef struct
{
	const char* name;
	const char* id;
	const char* init_from;
} dae_image_t;

VECTOR_TYPEDEF(dae_image_t, dae_image_vec_t, dae_image_vec_)

typedef struct
{
	const char* material_id;
	float_vec_t vertices, normals, colors, texcoords;
	int         vertices_offset, normals_offset, colors_offset, texcoords_offset;
	int         num_inputs;
	int_vec_t   points;
	int num_points;
	int num_faces;
} dae_polylist_t;

VECTOR_TYPEDEF(dae_polylist_t, dae_polylist_vec_t, dae_polylist_vec_)

typedef struct
{
	// todo material index
	const char* id;
	dae_polylist_vec_t polylists;
} dae_geometry_t;

VECTOR_TYPEDEF(dae_geometry_t, dae_geometry_vec_t, dae_geometry_vec_)

typedef struct
{
	float r, g, b, a;
} dae_col4f_t;

typedef struct
{
	const char* id;
	// todo union for other lighting types?
	// <phong>:
	dae_col4f_t emission_color;
	dae_col4f_t ambient_color;
	dae_col4f_t diffuse_color;
	dae_col4f_t specular_color;
	const char* diffuse_texture;
	float shininess;
	float index_of_refraction;
} dae_effect_t;

VECTOR_TYPEDEF(dae_effect_t, dae_effect_vec_t, dae_effect_vec_)

typedef struct
{
	const char* id;
	const char* name;
	const char* effect_id;
} dae_material_t;

VECTOR_TYPEDEF(dae_material_t, dae_material_vec_t, dae_material_vec_)

typedef struct
{
	xml_t xml;
	dae_image_vec_t images;
	dae_geometry_vec_t geometries;
	dae_effect_vec_t effects;
	dae_material_vec_t materials;
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

int dae_init_images(dae_t* dae);
int dae_free_images(dae_t* dae);
int dae_init_geometries(dae_t* dae);
int dae_free_geometries(dae_t* dae);
int dae_init_effects(dae_t* dae);
int dae_free_effects(dae_t* dae);
int dae_init_materials(dae_t* dae);
int dae_free_materials(dae_t* dae);

int dae_report(dae_t* dae); // debug

int dae_geometry_init(dae_geometry_t* dae_geometry, const char* id);
int dae_geometry_free(dae_geometry_t* dae_geometry);

int dae_polylist_init(dae_polylist_t* dae_polylist, const char* material_id);
int dae_polylist_free(dae_polylist_t* dae_polylist);

int dae_polylist_get_vertex_vec3(dae_polylist_t* dae_polylist, int point_index, vec3f_t* vertex, double vscale);
int dae_polylist_get_normal_vec3(dae_polylist_t* dae_polylist, int point_index, vec3f_t* normal);
int dae_polylist_get_color_vec3(dae_polylist_t* dae_polylist, int point_index, vec3f_t* color);
int dae_polylist_get_face_vertices(dae_polylist_t* dae_polylist, int face_idx, vec3f_t* vertices, double vscale);
int dae_polylist_get_face_normals(dae_polylist_t* dae_polylist, int face_idx, vec3f_t* normals);
int dae_polylist_get_face_colors(dae_polylist_t* dae_polylist, int face_idx, vec3f_t* colors);

int float_vec_push_from_string(float_vec_t* fvec, const char* str);
int int_vec_push_from_string(int_vec_t* ivec, const char* str);
int mat4f_init_from_string(mat4f_t* m, const char* str);
int vec3f_transform(vec3f_t* v, mat4f_t* m);

int dae_col4f_init_from_string(dae_col4f_t* c, const char* str);

int float_vec_print(float_vec_t* fvec, int width, int height); // debug
int int_vec_print(int_vec_t* ivec, int width, int height); // debug
void mat4f_print(mat4f_t* m); // debug

#endif // DAE_H
