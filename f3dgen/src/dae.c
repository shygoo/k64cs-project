#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dae.h"
#include "xml.h"
#include "vecprim.h"

VECTOR_IMPLDEF(dae_geometry_t, dae_geometry_vec_t, dae_geometry_vec_)

int dae_load(dae_t* dae, const char* path)
{
	if(!xml_load(&dae->xml, path))
	{
		return 0;
	}
	dae_init_geometries(dae);
	return 1;
}

int dae_unload(dae_t* dae)
{
    dae_free_geometries(dae);
    xml_unload(&dae->xml);
    return 1;
}

int dae_init_geometries(dae_t* dae)
{
    dae_geometry_vec_init(&dae->geometries);
    
    xml_node_t* library_geometries_node = xml_node_query(&dae->xml.root_node, "library_geometries");

	xml_node_t* geometry_node;
	
	for(int i = 0; (geometry_node = xml_node_get_child(library_geometries_node, i)); i++)
	{
		dae_geometry_t dae_geometry;
		
		xml_node_t* polygons_node;
		//const char* sz_polygons_material;
		
		const char* id = xml_node_get_attr(geometry_node, "id");
		dae_geometry_init(&dae_geometry, id);
		
		polygons_node = xml_node_query(geometry_node, "polygons");
		
        if(polygons_node == NULL)
        {
            polygons_node = xml_node_query(geometry_node, "polylist"); // alternate tag
        }
		
		//sz_polygons_material = xml_node_get_attr(polygons_node, "material");
		
        xml_node_t* polygons_subnode;
        for(int j = 0; (polygons_subnode = xml_node_get_child(polygons_node, j)); j++)
        {
            if(xml_node_tag_eq(polygons_subnode, "vcount"))
            {
				// don't need this
                continue;
            }
            else if(xml_node_tag_eq(polygons_subnode, "input"))
            {
				xml_node_t* input_node = polygons_subnode;
			
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
				
                dae_geometry.num_inputs++;
				
                if(strcmp(sz_input_semantic, "POSITION") == 0)
                {
					dae_geometry.vertices_offset = input_offset;
					float_vec_push_from_string(&dae_geometry.vertices, array_node->inner_text);
				}
                else if(strcmp(sz_input_semantic, "NORMAL") == 0)
                {
                    dae_geometry.normals_offset = input_offset;
					float_vec_push_from_string(&dae_geometry.normals, array_node->inner_text);
                }
                else if(strcmp(sz_input_semantic, "TEXCOORD") == 0)
                {
                    dae_geometry.texcoords_offset = input_offset;
					float_vec_push_from_string(&dae_geometry.texcoords, array_node->inner_text);
                }
				else if(strcmp(sz_input_semantic, "COLOR") == 0)
				{
					dae_geometry.colors_offset = input_offset;
					float_vec_push_from_string(&dae_geometry.colors, array_node->inner_text);
				}
				else
				{
					printf("Warning: skipping unknown input semantic %s\n", xml_node_get_attr(polygons_subnode, "semantic"));
				}
            }
            else if(xml_node_tag_eq(polygons_subnode, "p"))
            {
				xml_node_t* p_node = polygons_subnode;
				int_vec_push_from_string(&dae_geometry.points, p_node->inner_text);
                continue;
            }
		}
		
		dae_geometry.num_points = (dae_geometry.points.length / dae_geometry.num_inputs);
		dae_geometry.num_faces = dae_geometry.num_points / 3;
		
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
	dae_geometry->num_inputs = 0;
	
	float_vec_init(&dae_geometry->vertices);
	float_vec_init(&dae_geometry->normals);
	float_vec_init(&dae_geometry->colors);
	float_vec_init(&dae_geometry->texcoords);
	
	int_vec_init(&dae_geometry->points);
	
	dae_geometry->vertices_offset = DAE_NO_OFFSET;
	dae_geometry->normals_offset = DAE_NO_OFFSET;
	dae_geometry->colors_offset = DAE_NO_OFFSET;
	dae_geometry->texcoords_offset = DAE_NO_OFFSET;
	
	return 1;
}

int dae_geometry_free(dae_geometry_t* dae_geometry)
{
	float_vec_free(&dae_geometry->vertices);
	float_vec_free(&dae_geometry->normals);
	float_vec_free(&dae_geometry->colors);
    float_vec_free(&dae_geometry->texcoords);
    int_vec_free(&dae_geometry->points);
    return 1;
}

int dae_geometry_get_vertex_vec3(dae_geometry_t* dae_geometry, int point_index, vec3f_t* vertex, double vscale)
{
	int polylist_offset = point_index * dae_geometry->num_inputs + dae_geometry->vertices_offset;
	int vertices_index = dae_geometry->points.array[polylist_offset] * 3;
	vertex->x = dae_geometry->vertices.array[vertices_index + 0] * vscale;
	vertex->y = dae_geometry->vertices.array[vertices_index + 1] * vscale;
	vertex->z = dae_geometry->vertices.array[vertices_index + 2] * vscale;
	return 1;
}

int dae_geometry_get_normal_vec3(dae_geometry_t* dae_geometry, int point_index, vec3f_t* normal)
{
	int polylist_offset = point_index * dae_geometry->num_inputs + dae_geometry->normals_offset;
	int normals_index = dae_geometry->points.array[polylist_offset] * 3;
	normal->x = dae_geometry->normals.array[normals_index + 0];
	normal->y = dae_geometry->normals.array[normals_index + 1];
	normal->z = dae_geometry->normals.array[normals_index + 2];
	return 1;
}

int dae_geometry_get_color_vec3(dae_geometry_t* dae_geometry, int point_index, vec3f_t* color)
{
	int polylist_offset = point_index * dae_geometry->num_inputs + dae_geometry->colors_offset;
	int colors_index = dae_geometry->points.array[polylist_offset] * 3;
	color->x = dae_geometry->colors.array[colors_index + 0];
	color->y = dae_geometry->colors.array[colors_index + 1];
	color->z = dae_geometry->colors.array[colors_index + 2];
	return 1;
}

int dae_geometry_get_face_vertices(dae_geometry_t* dae_geometry, int face_idx, vec3f_t* vertices, double vscale)
{
	int point_idx = face_idx * 3;
	dae_geometry_get_vertex_vec3(dae_geometry, point_idx + 0, &vertices[0], vscale);
	dae_geometry_get_vertex_vec3(dae_geometry, point_idx + 1, &vertices[1], vscale);
	dae_geometry_get_vertex_vec3(dae_geometry, point_idx + 2, &vertices[2], vscale);
	return 1;
}

int dae_geometry_get_face_normals(dae_geometry_t* dae_geometry, int face_idx, vec3f_t* normals)
{
	int point_idx = face_idx * 3;
	dae_geometry_get_normal_vec3(dae_geometry, point_idx + 0, &normals[0]);
	dae_geometry_get_normal_vec3(dae_geometry, point_idx + 1, &normals[1]);
	dae_geometry_get_normal_vec3(dae_geometry, point_idx + 2, &normals[2]);
	return 1;
}

int dae_geometry_get_face_colors(dae_geometry_t* dae_geometry, int face_idx, vec3f_t* colors)
{
	int point_idx = face_idx * 3;
	dae_geometry_get_color_vec3(dae_geometry, point_idx + 0, &colors[0]);
	dae_geometry_get_color_vec3(dae_geometry, point_idx + 1, &colors[1]);
	dae_geometry_get_color_vec3(dae_geometry, point_idx + 2, &colors[2]);
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
		
		printf("[%s]\n", dae_geom->id);
				
		printf("Num inputs: %d\n", dae_geom->num_inputs);
		printf("Num points: %d\n", dae_geom->num_points);
		printf("Num faces: %d\n\n", dae_geom->num_faces);
		
		for(int i = 0; i < dae_geom->num_faces; i++)
		{
			vec3f_t face_vertices[3];
			vec3f_t face_normals[3];
			vec3f_t face_colors[3];
			
			dae_geometry_get_face_vertices(dae_geom, i, face_vertices);
			dae_geometry_get_face_normals(dae_geom, i, face_normals);
			dae_geometry_get_face_colors(dae_geom, i, face_colors);
			
			printf("vertices: [%.7g %.7g %.7g] [%.7g %.7g %.7g] [%.7g %.7g %.7g]\n",
				face_vertices[0].x, face_vertices[0].y, face_vertices[0].z,
				face_vertices[1].x, face_vertices[1].y, face_vertices[1].z,
				face_vertices[2].x, face_vertices[2].y, face_vertices[2].z
			);
		}
		
		printf("Vertex coords: \n");
		float_vec_print(&dae_geom->vertices, 3, 100);
		printf("\n");
		
		printf("Vertex colors: \n");
		float_vec_print(&dae_geom->colors, 3, 100);
		printf("\n");
		
		printf("Vertex normals: \n");
		float_vec_print(&dae_geom->normals, 3, 100);
		printf("\n");
		
		printf("Points:\n");
		int_vec_print(&dae_geom->points, dae_geom->num_inputs, 3);
		
		printf("\n\n");
	}
    
	return 1;
}

