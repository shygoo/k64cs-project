#include <stdint.h>
#include <stdio.h>

#include "obj.h"
#include "dlist.h"
#include "vertex.h"
#include "gbuf.h"

int process_v(obj_v_command_t* cmd, vertex_info_t* vertex_info, double scale)
{
    vertex_t vertex;

    vertex.x = (int16_t)(cmd->x * scale);
    vertex.y = (int16_t)(cmd->y * scale);
    vertex.z = (int16_t)(cmd->z * scale);

    vertex.color.a = 0xFF;

    vertex_list_push(&vertex_info->vertex_list, vertex);
    return 1;
}

int process_vt(obj_vt_command_t* cmd, vertex_info_t* vertex_info)
{
    tex_uv_t tex_uv;
    
    tex_uv.u = (int16_t)(cmd->u * INT16_MAX);
    tex_uv.v = (int16_t)(cmd->v * INT16_MAX);
    
    tex_uv_list_push(&vertex_info->tex_uv_list, tex_uv);
    return 1;
}

int process_vn(obj_vn_command_t* cmd, vertex_info_t* vertex_info)
{
    normal_t normal;
    
    normal.x = (int16_t)(cmd->i * INT16_MAX);
    normal.y = (int16_t)(cmd->j * INT16_MAX);
    normal.z = (int16_t)(cmd->k * INT16_MAX);
    
    normal_list_push(&vertex_info->normal_list, normal);
    return 1;
}

int process_f(obj_f_command_t* cmd, vertex_info_t* vertex_info, gbuf_t* gbuf,
	dlist_t* dlist, vertex_list_t* vlist, uint32_t vertex_base_address)
{
    vertex_t* v1 = vertex_list_ptr(&vertex_info->vertex_list, cmd->v1 - 1);
    vertex_t* v2 = vertex_list_ptr(&vertex_info->vertex_list, cmd->v2 - 1);
    vertex_t* v3 = vertex_list_ptr(&vertex_info->vertex_list, cmd->v3 - 1);

    tex_uv_t* vt1 = tex_uv_list_ptr(&vertex_info->tex_uv_list, cmd->vt1 - 1);
    tex_uv_t* vt2 = tex_uv_list_ptr(&vertex_info->tex_uv_list, cmd->vt2 - 1);
    tex_uv_t* vt3 = tex_uv_list_ptr(&vertex_info->tex_uv_list, cmd->vt3 - 1);

    normal_t* vn1 = normal_list_ptr(&vertex_info->normal_list, cmd->vn1 - 1);
    normal_t* vn2 = normal_list_ptr(&vertex_info->normal_list, cmd->vn2 - 1);
    normal_t* vn3 = normal_list_ptr(&vertex_info->normal_list, cmd->vn3 - 1);

    //int stride = cmd_max_vidx - cmd_min_vidx;
    //printf("%d %d %d\n", cmd->v1, cmd->v2, cmd->v3);

    if(!(v1 && v2 && v3))
    {
        printf("obj_to_f3d: invalid vertex index!\n");
        return 0;
    }

    // copy uv and normal information to the vertex

    v1->u = vt1->u;
    v1->v = vt1->v;
    v1->normal.x = vn1->x;
    v1->normal.y = vn1->y;
    v1->normal.z = vn1->z;

    v2->u = vt2->u;
    v2->v = vt2->v;
    v2->normal.x = vn2->x;
    v2->normal.y = vn2->y;
    v2->normal.z = vn2->z;

    v3->u = vt3->u;
    v3->v = vt3->v;
    v3->normal.x = vn3->x;
    v3->normal.y = vn3->y;
    v3->normal.z = vn3->z;

    // push vertex and tri info to gbuf

    // calculate space required in the vertex buffer
    int have_v1 = gbuf_has_vertex(gbuf, v1);
    int have_v2 = gbuf_has_vertex(gbuf, v2);
    int have_v3 = gbuf_has_vertex(gbuf, v3);
    int num_new_verts = 3 - (have_v1 + have_v2 + have_v3);

    if(gbuf->vidx + num_new_verts > gbuf->max_vertices)
    {
        // no more room in the vbuf for this triangle
        // flush commands to dlist, vertex copies to vlist
        gbuf_flush(gbuf, dlist, vlist, vertex_base_address);
    }

    int vidx1 = gbuf_push_unique_vertex(gbuf, *v1);
    int vidx2 = gbuf_push_unique_vertex(gbuf, *v2);
    int vidx3 = gbuf_push_unique_vertex(gbuf, *v3);

    gbuf_push_tri(gbuf, vidx1, vidx2, vidx3);

    //numtris++;

    return 1;
}

int obj_to_f3d(obj_t* obj, dlist_t* dlist, vertex_list_t* new_vertices, uint32_t vertex_base_address, double scale)
{
    gbuf_t gbuf;
    gbuf_init(&gbuf, 32);
	
	vertex_info_t obj_vertex_info;
    vertex_info_init(&obj_vertex_info); // obj file vertices

    dlist_push_pipesync(dlist);
    dlist_push_tilesync(dlist);

    dlist_push_geometrymode(dlist, G_ALL, G_LIGHTING); // kirby seems to do this
    dlist_push_command(dlist, (dlist_command_t){0xFCFFFFFF, 0xFFFE793C});
	
	// TODO g_vtx to load bounding box and culldl here

	for(int i = 0; i < obj->num_commands; i++)
	{
		obj_generic_command_t gen_command = obj->commands[i];
	
		switch(gen_command.id)
		{
			case OBJ_MTLLIB:
			{
				//obj_mtllib_command_t cmd = gen_command.gcmd.mtllib_command;
				//printf("MTLLIB [%s]\n", cmd.path);
				break;
            }
			case OBJ_V:
			{
                obj_v_command_t cmd = gen_command.gcmd.v_command;
                process_v(&cmd, &obj_vertex_info, scale);
				break;
            }
            case OBJ_VT:
            {
                obj_vt_command_t cmd = gen_command.gcmd.vt_command;
                process_vt(&cmd, &obj_vertex_info);
                break;
            }
            case OBJ_VN:
            {
                obj_vn_command_t cmd = gen_command.gcmd.vn_command;
                process_vn(&cmd, &obj_vertex_info);
                break;
            }
			case OBJ_F:
			{
                obj_f_command_t cmd = gen_command.gcmd.f_command;
                process_f(&cmd, &obj_vertex_info, &gbuf, dlist, new_vertices, vertex_base_address);
				break;
			}
            case OBJ_USEMTL:
            {
				//obj_usemtl_command_t cmd = gen_command.gcmd.usemtl_command;
                //printf("USEMTL [%s]\n", cmd.name);
                break;
            }
            case OBJ_O:
            case OBJ_G:
                break;
		}
    }

    // flush remainder in gbuf
    if(gbuf.vidx != 0)
    {
        gbuf_flush(&gbuf, dlist, new_vertices, vertex_base_address);
    }
	
	dlist_push_geometrymode(dlist, G_ALL, G_LIGHTING); // kirby seems to do this

    dlist_push_enddl(dlist);
    
    gbuf_free(&gbuf);
	vertex_info_free(&obj_vertex_info);

    return 1;
}
