#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dae.h"
#include "xml.h"
#include "vecprim.h"

VECTOR_IMPLDEF(dae_image_t, dae_image_vec_t, dae_image_vec_)
VECTOR_IMPLDEF(dae_polylist_t, dae_polylist_vec_t, dae_polylist_vec_)
VECTOR_IMPLDEF(dae_geometry_t, dae_geometry_vec_t, dae_geometry_vec_)
VECTOR_IMPLDEF(dae_effect_t, dae_effect_vec_t, dae_effect_vec_)
VECTOR_IMPLDEF(dae_material_t, dae_material_vec_t, dae_material_vec_)

int dae_load(dae_t* dae, const char* path)
{
	printf("[dae] loading xml file\n");
	if(!xml_load(&dae->xml, path))
	{
		return 0;
	}

	printf("[dae] initializing images\n");
	dae_init_images(dae);

	printf("[dae] initializing effects\n");
	dae_init_effects(dae);

	printf("[dae] initializing materials\n");
	dae_init_materials(dae);

	printf("[dae] initializing geometries\n");
	dae_init_geometries(dae);
	
	printf("[dae] load finished\n");
	return 1;
}

int dae_unload(dae_t* dae)
{
	printf("[dae] unloading\n");
	printf("[dae] freeing geometries\n");
	dae_free_geometries(dae);
	printf("[dae] freeing materials\n");
	dae_free_materials(dae);
	printf("[dae] freeing effects\n");
	dae_free_effects(dae);
	printf("[dae] freeing images\n");
	dae_free_images(dae);
	printf("[dae] unloading xml file\n");
	xml_unload(&dae->xml);
	printf("[dae] done unloading\n");
    return 1;
}

int dae_init_images(dae_t* dae)
{
	dae_image_vec_init(&dae->images);
	xml_node_t* library_images_node = xml_node_query(&dae->xml.root_node, "library_images");

	for(int i = 0; i < library_images_node->num_child_nodes; i++)
	{
		xml_node_t* image_node = xml_node_get_child(library_images_node, i);
		xml_node_t* init_from_node = xml_node_query(image_node, "init_from");

		dae_image_t dae_image;

		dae_image.name = xml_node_get_attr(image_node, "name");
		dae_image.id = xml_node_get_attr(image_node, "id");
		dae_image.init_from = init_from_node->inner_text;

		printf("[dae]   adding image '%s' (init_from: %s)\n", dae_image.id, dae_image.init_from);
	}

	return 1;
}

int dae_free_images(dae_t* dae)
{
	dae_image_vec_free(&dae->images);
	return 1;
}

int dae_init_materials(dae_t* dae)
{
	dae_material_vec_init(&dae->materials);
	xml_node_t* library_materials_node = xml_node_query(&dae->xml.root_node, "library_materials");

	for(int i = 0; i < library_materials_node->num_child_nodes; i++)
	{
		xml_node_t* material_node = xml_node_get_child(library_materials_node, i);
		xml_node_t* instance_effect_node = xml_node_query(material_node, "instance_effect"); // can there be more than one?

		dae_material_t dae_material;

		const char* effect_id_url = xml_node_get_attr(instance_effect_node, "url");

		dae_material.id = xml_node_get_attr(material_node, "id");
		dae_material.name = xml_node_get_attr(material_node, "name");
		dae_material.effect_id = &effect_id_url[1]; // skip # char

		printf("[dae]   adding material '%s' (effect: '%s')\n", dae_material.id, dae_material.effect_id);

		dae_material_vec_push(&dae->materials, dae_material);
	}

	return 1;
}

int dae_free_materials(dae_t* dae)
{
	dae_material_vec_free(&dae->materials);
	return 1;
}

int dae_init_effects(dae_t* dae)
{
	dae_effect_vec_init(&dae->effects);
	xml_node_t* library_effects_node = xml_node_query(&dae->xml.root_node, "library_effects");

	for(int i = 0; i < library_effects_node->num_child_nodes; i++)
	{
		xml_node_t* effect_node = xml_node_get_child(library_effects_node, i);

		dae_effect_t dae_effect;

		dae_effect.id = xml_node_get_attr(effect_node, "id");

		printf("[dae]   adding effect '%s'\n", dae_effect.id);

		// will probably need revision
		xml_node_t* profile_node = xml_node_query(effect_node, "profile_COMMON");
		xml_node_t* phong_node = xml_node_query(profile_node, "technique phong");

		xml_node_t* emission_color_node = xml_node_query(phong_node, "emission color");
		xml_node_t* ambient_color_node = xml_node_query(phong_node, "ambient color");
		xml_node_t* diffuse_color_node = xml_node_query(phong_node, "diffuse color");
		xml_node_t* diffuse_texture_node = xml_node_query(phong_node, "diffuse texture");
		xml_node_t* specular_color_node = xml_node_query(phong_node, "specular color");

		xml_node_t* shininess_node = xml_node_query(phong_node, "shininess float");
		xml_node_t* index_of_refraction_node = xml_node_query(phong_node, "index_of_refraction float");

		if(diffuse_texture_node != NULL)
		{
			const char* sampler_id = xml_node_get_attr(diffuse_texture_node, "texture");

			xml_node_t* newparam_sampler_node = xml_node_get_by_attr_value(profile_node, "sid", sampler_id);
			xml_node_t* sampler2d_source_node = xml_node_query(newparam_sampler_node, "sampler2D source");

			const char* surface_source_id = sampler2d_source_node->inner_text;

			xml_node_t* newparam_surface_node = xml_node_get_by_attr_value(profile_node, "sid", surface_source_id);
			xml_node_t* init_from_node = xml_node_query(newparam_surface_node, "surface init_from");
			
			dae_effect.diffuse_texture = init_from_node->inner_text;
			printf("[dae]     using diffuse texture '%s'\n", dae_effect.diffuse_texture);
		}
		else
		{
			dae_col4f_init_from_string(&dae_effect.diffuse_color, diffuse_color_node->inner_text);
		}

		dae_col4f_init_from_string(&dae_effect.emission_color, emission_color_node->inner_text);
		dae_col4f_init_from_string(&dae_effect.ambient_color, ambient_color_node->inner_text);
		dae_col4f_init_from_string(&dae_effect.specular_color, specular_color_node->inner_text);

		dae_effect.shininess = atof(shininess_node->inner_text);
		dae_effect.index_of_refraction = atof(index_of_refraction_node->inner_text);

		dae_effect_vec_push(&dae->effects, dae_effect);
	}

	return 1;
}

int dae_free_effects(dae_t* dae)
{
    dae_effect_vec_free(&dae->effects);
    return 1;
}

static int dae__geometry_push_polylist_from_xml(dae_geometry_t* dae_geometry, xml_node_t* geometry_node, xml_node_t* polylist_node)
{
	const char* material_id = xml_node_get_attr(polylist_node, "material");

	dae_polylist_t dae_polylist;
	dae_polylist_init(&dae_polylist, material_id);

	for(int i = 0; i < polylist_node->num_child_nodes; i++)
	{
		xml_node_t* polylist_subnode = xml_node_get_child(polylist_node, i);

		//if(xml_node_tag_eq(polylist_subnode, "vcount"))
		//{
		//	// don't need this
		//	continue;
		//}
		
		if(xml_node_tag_eq(polylist_subnode, "input"))
		{
			xml_node_t* input_node = polylist_subnode;

			const char* sz_input_semantic = xml_node_get_attr(input_node, "semantic");
            const char* sz_input_offset = xml_node_get_attr(input_node, "offset");
			const char* sz_input_source = xml_node_get_attr(input_node, "source");

			int input_offset = atoi(sz_input_offset);

			xml_node_t* input_source_node = xml_node_query(geometry_node, sz_input_source);
			
			if(strcmp(input_source_node->tag, "vertices") == 0)
			{
				xml_node_t* positions_input_node = xml_node_query(input_source_node, "input");
				sz_input_semantic = xml_node_get_attr(positions_input_node, "semantic");
				sz_input_source = xml_node_get_attr(positions_input_node, "source");
				
				input_source_node = xml_node_query(geometry_node, sz_input_source);
			}
			
			xml_node_t* accessor_node = xml_node_query(input_source_node, "technique_common accessor");
			
			const char* array_url = xml_node_get_attr(accessor_node, "source");
			
			xml_node_t* array_node = xml_node_query(input_source_node, array_url);
			
            dae_polylist.num_inputs++;
			
            if(strcmp(sz_input_semantic, "POSITION") == 0)
            {
				dae_polylist.vertices_offset = input_offset;
				float_vec_push_from_string(&dae_polylist.vertices, array_node->inner_text);
			}
            else if(strcmp(sz_input_semantic, "NORMAL") == 0)
            {
                dae_polylist.normals_offset = input_offset;
				float_vec_push_from_string(&dae_polylist.normals, array_node->inner_text);
            }
            else if(strcmp(sz_input_semantic, "TEXCOORD") == 0)
            {
                dae_polylist.texcoords_offset = input_offset;
				float_vec_push_from_string(&dae_polylist.texcoords, array_node->inner_text);
            }
			else if(strcmp(sz_input_semantic, "COLOR") == 0)
			{
				dae_polylist.colors_offset = input_offset;
				float_vec_push_from_string(&dae_polylist.colors, array_node->inner_text);
			}
			else
			{
				printf("Warning: skipping unknown input semantic %s\n", sz_input_semantic);
			}
		}
        else if(xml_node_tag_eq(polylist_subnode, "p"))
        {
			xml_node_t* p_node = polylist_subnode;
			int_vec_push_from_string(&dae_polylist.points, p_node->inner_text);
        }
	}

	dae_polylist.num_points = (dae_polylist.points.length / dae_polylist.num_inputs);
	dae_polylist.num_faces = dae_polylist.num_points / 3;

	dae_polylist_vec_push(&dae_geometry->polylists, dae_polylist);

	return 1;
}

int dae_init_geometries(dae_t* dae)
{
    dae_geometry_vec_init(&dae->geometries);
    
    xml_node_t* library_geometries_node = xml_node_query(&dae->xml.root_node, "library_geometries");

	for(int i = 0; i < library_geometries_node->num_child_nodes; i++)
	{
		xml_node_t* geometry_node = xml_node_get_child(library_geometries_node, i);
		xml_node_t* mesh_node = xml_node_query(geometry_node, "mesh");
		const char* id = xml_node_get_attr(geometry_node, "id");

		dae_geometry_t dae_geometry;
		dae_geometry_init(&dae_geometry, id);

		printf("[dae]   adding geometry '%s'\n", id);

		// process all <polygons>/<polylist>
		for(int j = 0; j < mesh_node->num_child_nodes; j++)
		{
			xml_node_t* polylist_node = xml_node_get_child(mesh_node, j);
			
			if(!(xml_node_tag_eq(polylist_node, "polylist") || xml_node_tag_eq(polylist_node, "polygons")))
			{
				continue;
			}

			printf("[dae]     adding polylist\n");
			
			dae__geometry_push_polylist_from_xml(&dae_geometry, geometry_node, polylist_node);
		}
		
		dae_geometry_vec_push(&dae->geometries, dae_geometry);
	}
	return 1;
}

int dae_free_geometries(dae_t* dae)
{
    dae_geometry_vec_iterate(&dae->geometries, dae_geometry_free);
    dae_geometry_vec_free(&dae->geometries);
    return 1;
}

////////////////////////////////////

int dae_geometry_init(dae_geometry_t* dae_geometry, const char* id)
{
	dae_geometry->id = id;
	dae_polylist_vec_init(&dae_geometry->polylists);
	return 1;
}

int dae_geometry_free(dae_geometry_t* dae_geometry)
{
	dae_polylist_vec_free(&dae_geometry->polylists);
    return 1;
}

int dae_polylist_init(dae_polylist_t* dae_polylist, const char* material_id)
{
	dae_polylist->material_id = material_id;

	dae_polylist->num_inputs = 0;

	float_vec_init(&dae_polylist->vertices);
	float_vec_init(&dae_polylist->normals);
	float_vec_init(&dae_polylist->colors);
	float_vec_init(&dae_polylist->texcoords);

	int_vec_init(&dae_polylist->points);

	dae_polylist->vertices_offset = DAE_NO_OFFSET;
	dae_polylist->normals_offset = DAE_NO_OFFSET;
	dae_polylist->colors_offset = DAE_NO_OFFSET;
	dae_polylist->texcoords_offset = DAE_NO_OFFSET;

	return 1;
}

int dae_polylist_free(dae_polylist_t* dae_polylist)
{
	float_vec_free(&dae_polylist->vertices);
	float_vec_free(&dae_polylist->normals);
	float_vec_free(&dae_polylist->colors);
    float_vec_free(&dae_polylist->texcoords);
	int_vec_free(&dae_polylist->points);
	return 1;
}

int dae_polylist_get_vertex_vec3(dae_polylist_t* dae_polylist, int point_index, vec3f_t* vertex, double vscale)
{
	int polylist_offset = point_index * dae_polylist->num_inputs + dae_polylist->vertices_offset;
	int vertices_index = dae_polylist->points.array[polylist_offset] * 3;
	vertex->x = dae_polylist->vertices.array[vertices_index + 0] * -vscale; // flip horizontally
	vertex->y = dae_polylist->vertices.array[vertices_index + 2] * vscale; // swap y and z
	vertex->z = dae_polylist->vertices.array[vertices_index + 1] * vscale;
	return 1;
}

int dae_polylist_get_normal_vec3(dae_polylist_t* dae_polylist, int point_index, vec3f_t* normal)
{
	int polylist_offset = point_index * dae_polylist->num_inputs + dae_polylist->normals_offset;
	int normals_index = dae_polylist->points.array[polylist_offset] * 3;
	normal->x = dae_polylist->normals.array[normals_index + 0];
	normal->y = dae_polylist->normals.array[normals_index + 1];
	normal->z = dae_polylist->normals.array[normals_index + 2];
	return 1;
}

int dae_polylist_get_color_vec3(dae_polylist_t* dae_polylist, int point_index, vec3f_t* color)
{
	int polylist_offset = point_index * dae_polylist->num_inputs + dae_polylist->colors_offset;
	int colors_index = dae_polylist->points.array[polylist_offset] * 3;
	color->x = dae_polylist->colors.array[colors_index + 0];
	color->y = dae_polylist->colors.array[colors_index + 1];
	color->z = dae_polylist->colors.array[colors_index + 2];
	return 1;
}

int dae_polylist_get_face_vertices(dae_polylist_t* dae_polylist, int face_idx, vec3f_t* vertices, double vscale)
{
	int point_idx = face_idx * 3;
	dae_polylist_get_vertex_vec3(dae_polylist, point_idx + 0, &vertices[0], vscale);
	dae_polylist_get_vertex_vec3(dae_polylist, point_idx + 1, &vertices[1], vscale);
	dae_polylist_get_vertex_vec3(dae_polylist, point_idx + 2, &vertices[2], vscale);
	return 1;
}

int dae_polylist_get_face_normals(dae_polylist_t* dae_polylist, int face_idx, vec3f_t* normals)
{
	int point_idx = face_idx * 3;
	dae_polylist_get_normal_vec3(dae_polylist, point_idx + 0, &normals[0]);
	dae_polylist_get_normal_vec3(dae_polylist, point_idx + 1, &normals[1]);
	dae_polylist_get_normal_vec3(dae_polylist, point_idx + 2, &normals[2]);
	return 1;
}

int dae_polylist_get_face_colors(dae_polylist_t* dae_polylist, int face_idx, vec3f_t* colors)
{
	int point_idx = face_idx * 3;
	dae_polylist_get_color_vec3(dae_polylist, point_idx + 0, &colors[0]);
	dae_polylist_get_color_vec3(dae_polylist, point_idx + 1, &colors[1]);
	dae_polylist_get_color_vec3(dae_polylist, point_idx + 2, &colors[2]);
	return 1;
}

////////////////////////////////////

int vec3f_transform(vec3f_t* v, mat4f_t* m)
{
    float x = v->x;
    float y = v->y;
    float z = v->z;

    v->x = (m->m[0][0] * x) + (m->m[0][1] * y) + (m->m[0][2] * z) + (m->m[0][3] * 1);
    v->y = (m->m[1][0] * x) + (m->m[1][1] * y) + (m->m[1][2] * z) + (m->m[1][3] * 1);
    v->z = (m->m[2][0] * x) + (m->m[2][1] * y) + (m->m[2][2] * z) + (m->m[2][3] * 1);

    printf("z: %f\n", v->z);
	return 1;
}

int float_vec_push_from_string(float_vec_t* fvec, const char* str)
{
	char* end;

	do
	{
		float value = strtof(str, &end);
		
		if(str == end)
		{
			return 0;
		}
		
		str = end;
		
		float_vec_push(fvec, value);
		
	} while(*end);
	
	return 1;
}

int float_vec_print(float_vec_t* fvec, int width, int height)
{
	for(int x = 0; x < fvec->length; x++)
	{
		printf("%.7g ", fvec->array[x]);
		if((x+1) % width == 0)
		{
			printf("\n");
			
			if((x+1) % (width*height) == 0)
			{
				printf("\n");
			}
		}
	}
	printf("\n");
	return 1;
}

int int_vec_push_from_string(int_vec_t* ivec, const char* str)
{
	char* end;

	do
	{
		int value = strtol(str, &end, 10);
		
		if(str == end)
		{
			return 0;
		}
		
		str = end;
		
		int_vec_push(ivec, value);
		
	} while(*end);
	
	return 1;
}

int int_vec_print(int_vec_t* ivec, int width, int height)
{
	for(int x = 0; x < ivec->length; x++)
	{
		printf("%d ", ivec->array[x]);
		if((x+1) % width == 0)
		{
			printf("\n");
			
			if((x+1) % (width*height) == 0)
			{
				printf("\n");
			}
		}
	}
	printf("\n");
	return 1;
}

int dae_col4f_init_from_string(dae_col4f_t* c, const char* str)
{
	float* carr = (float*)c;

	for(int i = 0; i < 4; i++)
	{
		char* end;
		carr[i] = strtof(str, &end);
		
		if(str == end)
		{
			return 0;
		}
		
		str = end;
	}
	
	return 1;
}

int mat4f_init_from_string(mat4f_t* m, const char* str)
{
	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			char* end;
			m->m[i][j] = strtof(str, &end);
			
			if(str == end)
			{
				return 0;
			}
			
			str = end;
		}
	}
	
	return 1;
}

void mat4f_print(mat4f_t* m)
{
	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			printf("%f, ", m->m[i][j]);
		}
		printf("\n");
	}
}

int dae_report(dae_t* dae)
{
	for(int i = 0; i < dae->geometries.length; i++)
	{
		dae_geometry_t* dae_geom = &dae->geometries.array[i];
		
		printf("[geometry: %s]\n", dae_geom->id);

		for(int j = 0; j < dae_geom->polylists.length; j++)
		{
			dae_polylist_t* dae_polylist = &dae_geom->polylists.array[j];

			printf("[Polylist %d]", j);

			printf("Num inputs: %d\n", dae_polylist->num_inputs);
			printf("Num points: %d\n", dae_polylist->num_points);
			printf("Num faces: %d\n\n", dae_polylist->num_faces);
			
			for(int k = 0; k < dae_polylist->num_faces; k++)
			{
				vec3f_t face_vertices[3];
				vec3f_t face_normals[3];
				vec3f_t face_colors[3];

				dae_polylist_get_face_vertices(dae_polylist, k, face_vertices, 1.0);
				dae_polylist_get_face_normals(dae_polylist, k, face_normals);
				dae_polylist_get_face_colors(dae_polylist, k, face_colors);

				printf("vertices: [%.7g %.7g %.7g] [%.7g %.7g %.7g] [%.7g %.7g %.7g]\n",
					face_vertices[0].x, face_vertices[0].y, face_vertices[0].z,
					face_vertices[1].x, face_vertices[1].y, face_vertices[1].z,
					face_vertices[2].x, face_vertices[2].y, face_vertices[2].z
				);
			}

			printf("Vertex coords: \n");
			float_vec_print(&dae_polylist->vertices, 3, 100);
			printf("\n");

			printf("Vertex colors: \n");
			float_vec_print(&dae_polylist->colors, 3, 100);
			printf("\n");

			printf("Vertex normals: \n");
			float_vec_print(&dae_polylist->normals, 3, 100);
			printf("\n");

			printf("Points:\n");
			int_vec_print(&dae_polylist->points, dae_polylist->num_inputs, 3);

			printf("\n\n");
		}
	}
    
	return 1;
}

