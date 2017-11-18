#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dlist.h"
#include "bswap.h"

VECTOR_IMPLDEF(dlist_command_t, dlist_command_vec_t, dlist_command_vec_)

int dlist_command_swap(dlist_command_t* command)
{
	command->a = bswap32(command->a);
	command->b = bswap32(command->b);
	return 1;
}

void dlist_init(dlist_t* dlist, ucode_t ucode)
{
	dlist->ucode = ucode;
	dlist_command_vec_init(&dlist->commands);
}

void dlist_free(dlist_t* dlist)
{
	dlist_command_vec_free(&dlist->commands);
}

void dlist_bswap(dlist_t* dlist)
{
	dlist_command_vec_iterate(&dlist->commands, dlist_command_swap);
}

void dlist_dump(dlist_t* dlist, const char* path)
{
	FILE* fp = fopen(path, "wb");
	fwrite(dlist->commands.array, dlist_command_vec_size_bytes(&dlist->commands), 1, fp);
	fclose(fp);
}

void dlist_print(dlist_t* dlist)
{
	for(int i = 0; i < dlist->commands.length; i++)
	{
		dlist_command_t command = dlist->commands.array[i];
		printf("%08X %08X [", command.a, command.b);

		uint32_t n = command.a;

		for(uint32_t s = INT32_MIN, i = 0; s != 0; s >>= 1, i++)
		{
			if(i > 0 && i % 4 == 0)
			{
				printf(" ");
			}
			printf(n & s ? "1" : "0");

		}

		printf(" | ");

		n = command.b;
		
		for(uint32_t s = INT32_MIN, i = 0; s != 0; s >>= 1, i++)
		{
			if(i > 0 && i % 4 == 0)
			{
				printf(" ");
			}
			printf(n & s ? "1" : "0");
		}

		printf("]\n");
	}
}

int dlist_size_bytes(dlist_t* dlist)
{
	return dlist_command_vec_size_bytes(&dlist->commands);
}

void dlist_copy(dlist_t* dlist, void* dest)
{
	memcpy(dest, dlist->commands.array, dlist_command_vec_size_bytes(&dlist->commands));
}

int dlist_push_command(dlist_t* dlist, dlist_command_t command)
{
	return dlist_command_vec_push(&dlist->commands, command);
}

int dlist_push_tri1(dlist_t* dlist, uint8_t v0, uint8_t v1, uint8_t v2)
{
	dlist_command_t command;

	if(dlist->ucode == UCODE_F3DEX2)
	{
		command.a = 0x05000000;
		command.a |= v0 << 17;
		command.a |= v1 << 9;
		command.a |= v2 << 1;
		command.b = 0;
		return dlist_push_command(dlist, command);
	}

	return -1;
}

int dlist_push_tri2(dlist_t* dlist, uint8_t v0, uint8_t v1, uint8_t v2, uint8_t v10, uint8_t v11, uint8_t v12)
{
	dlist_command_t command;

	if(dlist->ucode == UCODE_F3DEX2)
	{
		command.a = 0x06000000;
		command.a |= v0 << 17;
		command.a |= v1 << 9;
		command.a |= v2 << 1;
		command.b = v10 << 17;
		command.b |= v11 << 9;
		command.b |= v12 << 1;
		return dlist_push_command(dlist, command);
	}

	return -1;
}

int dlist_push_vtx(dlist_t* dlist, uint32_t vaddr, uint8_t numv, uint8_t vidx)
{
	dlist_command_t command;

	if(dlist->ucode == UCODE_F3DEX2)
	{
		command.a = 0x01000000;
		command.a |= numv << 12;
		command.a |= (vidx + numv) * 2;
		command.b = vaddr;
		return dlist_push_command(dlist, command);
	}

	return -1;
}

int dlist_push_enddl(dlist_t* dlist)
{
	dlist_command_t command;
	if(dlist->ucode == UCODE_F3DEX2)
	{
		command.a = 0xDF000000;
		command.b = 0x00000000;
		return dlist_push_command(dlist, command);
	}
	return -1;
}

int dlist_push_tilesync(dlist_t* dlist)
{
	dlist_command_t command;
	if(dlist->ucode == UCODE_F3DEX2)
	{
		command.a = 0xE8000000;
		command.b = 0x00000000;
		return dlist_push_command(dlist, command);
	}
	return -1;
}

int dlist_push_pipesync(dlist_t* dlist)
{
	dlist_command_t command;
	if(dlist->ucode == UCODE_F3DEX2)
	{
		command.a = 0xE7000000;
		command.b = 0x00000000;
		return dlist_push_command(dlist, command);
	}
	return -1;
}

int dlist_push_settile(dlist_t* dlist, int fmt, int siz, int line, int tmem, int tile,
	int palette, int cmT, int maskT, int shiftT, int cmS, int maskS, int shiftS)
{
	// (F5) fmt:010 siz:10 [0] line:000000000 tmem:000000000 [00000]
	//      tile:111 palette:0000
	//      cmT:00 maskT:0101 shiftT:0000
	//      cmS:00 maskS:0101 shiftS:0000 
	// fmt = 2 ci
	// siz = 2 16b
	// line = 0
	// tmem = 0
	// tile = 7 (tile descriptor)
	// palette = 0
	// cmT = 0 // no mirror,wrap

	// dlist_push_settile(dlist,
	//	G_IM_FMT_CI, G_IM_SIZ_16b, 0, 0, 7, 0,
	//	0, 0b0101, 0,
    //  0, 0b0101, 0);

	dlist_command_t command;
	if(dlist->ucode == UCODE_F3DEX2)
	{
		command.a = 0xF5000000;
		command.b = 0x00000000;

		command.a |= (fmt & 7) << 21;
		command.a |= (siz & 3) << 19;
		command.a |= (line & 0x1FF) << 9;
		command.a |= (tmem & 0x1FF);

		command.b |= (tile & 7) << 24;
		command.b |= (palette & 15) << 20;
		command.b |= (cmT & 3) << 18;
		command.b |= (maskT & 15) << 14;
		command.b |= (shiftT & 15) << 10;
		command.b |= (cmS & 3) << 8;
		command.b |= (maskS & 15) << 4;
		command.b |= (shiftS & 15);
		return dlist_push_command(dlist, command);
	}
	return -1;
}

int dlist_push_geometrymode(dlist_t* dlist, uint32_t clearbits, uint32_t setbits)
{
	dlist_command_t command;
	if(dlist->ucode == UCODE_F3DEX2)
	{
		command.a = 0xD9000000 | (clearbits & G_ALL);
		command.b = 0x00000000 | setbits;
		return dlist_push_command(dlist, command);
	}
	return -1;
}
