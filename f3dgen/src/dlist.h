#ifndef DLIST_H
#define DLIST_H

#include "vecdef.h"

#define G_IM_FMT_RGBA  0
#define G_IM_FMT_YUV   1
#define G_IM_FMT_CI    2
#define G_IM_FMT_IA    3
#define G_IM_FMT_I     4

#define G_IM_SIZ_4b    0
#define G_IM_SIZ_8b    1
#define G_IM_SIZ_16b   2
#define G_IM_SIZ_32b   3

#define G_TX_NOMIRROR  0
#define G_TX_MIRROR    1
#define G_TX_WRAP      0
#define G_TX_CLAMP     2

// geo mode

#define G_ZBUFFER            0x00000001
#define G_SHADE              0x00000004
#define G_CULL_FRONT         0x00000200
#define G_CULL_BACK          0x00000400
#define G_FOG                0x00010000
#define G_LIGHTING           0x00020000
#define G_TEXTURE_GEN        0x00040000
#define G_TEXTURE_GEN_LINEAR 0x00080000
#define G_SHADING_SMOOTH     0x00200000
#define G_CLIPPING           0x00800000
#define G_ALL                0x00FFFFFF
#define G_NONE               0x00000000

typedef enum
{
	UCODE_F3DEX2
} ucode_t;

typedef struct
{
	uint32_t a;
	uint32_t b;
} dlist_command_t;

int dlist_command_swap(dlist_command_t* command);

VECTOR_TYPEDEF(dlist_command_t, dlist_command_vec_t, dlist_command_vec_)

typedef struct 
{
	ucode_t ucode;
	dlist_command_vec_t commands;
} dlist_t;

void dlist_init(dlist_t* dlist, ucode_t ucode);
void dlist_free(dlist_t* dlist);
int  dlist_push_command(dlist_t* dlist, dlist_command_t command);
void dlist_print(dlist_t* dlist);
int  dlist_size_bytes(dlist_t* dlist);
void dlist_copy(dlist_t* dlist, void* dest);

void dlist_bswap(dlist_t* dlist);
void dlist_dump(dlist_t* dlist, const char* path);

int dlist_push_tri1(dlist_t* dlist, uint8_t v0, uint8_t v1, uint8_t v2);
int dlist_push_tri2(dlist_t* dlist, uint8_t v0, uint8_t v1, uint8_t v2, uint8_t v10, uint8_t v11, uint8_t v12);
int dlist_push_vtx(dlist_t* dlist, uint32_t vaddr, uint8_t numv, uint8_t vidx);
int dlist_push_enddl(dlist_t* dlist);
int dlist_push_tilesync(dlist_t* dlist);
int dlist_push_pipesync(dlist_t* dlist);
int dlist_push_settile(dlist_t* dlist, int fmt, int siz, int line, int tmem, int tile,
	int palette, int cmT, int maskT, int shiftT, int cmS, int maskS, int shiftS);
int dlist_push_geometrymode(dlist_t* dlist, uint32_t clearbits, uint32_t setbits);

#endif // DLIST_H
