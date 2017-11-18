#ifndef OBJF3D_H
#define OBJF3D_H

#include "obj.h"
#include "dlist.h"
#include "vertex.h"
#include "gbuf.h"

int process_v(obj_v_command_t* cmd, vertex_info_t* vertex_info, double scale);
int process_vt(obj_vt_command_t* cmd, vertex_info_t* vertex_info);
int process_vn(obj_vn_command_t* cmd, vertex_info_t* vertex_info);
int process_f(obj_f_command_t* cmd, vertex_info_t* vertex_info, gbuf_t* gbuf,
    dlist_t* dlist, vertex_list_t* vlist, uint32_t vertex_base_address);
int obj_to_f3d(obj_t* obj, dlist_t* dlist, vertex_list_t* new_vertices, uint32_t vertex_base_address, double scale);
    
#endif // OBJF3D_H
