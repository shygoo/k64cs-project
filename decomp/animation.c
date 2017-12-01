/*
802AC168 kirby
802AC410 kirby foot

80011180 huge routine, has animation related stuff
*/

// 8000D0AC
void animate_bone(gfx_entity_t* bone)
{
	if(bone->_74 == *(float*)0x80040708)
	{
		return; // 338
	}

	s0 = bone->transform

	if(s0 == 0) goto 310;

	f30 = 0;
	f28 = 0x3f800000; // 1.0

	f26 = sp_54 // uninitialized?
	
	loop:	
	t6 = s0->byte_05; // 8000D10C

	if(t6 == 0)
	{
		s0 = s0->_00 // word
		goto 300;
	}

	if(bone->f_74 != *(float*)0x8004070C)
	{
		s0->f_0c = s0->f_0c + bone->f_78
		// t7 = bone->_04 // not needed
	}

	if((bone->p_04->_44 & 0x02) != 0)
	{
		s0 = s0->_00
		goto 300;
	}

	switch(s0->byte_05)
	{
		case 1: //228:
			if(!(s0->f_08 < s0->f_0c))
			{
				f26 = s0->_10;
				break;
			}
			f26 = s0->f_14;
			break;
		case 2: //180:
			f26 = s0->f_10 + (s0->f_0c * s0->f_18)
			break;
		case 3: // 19C:
			f2 = s0->f_08
			f0 = s0->f_0c
			f16 = f2 * f2
			f6 = 0x40400000 // 3.0
			f12 = f0 * f0
			f18 = f2 * f12
			f8 = f0 * f12
			f14 = f8 * f16
			f8 = s0->f_10
			f10 = f14 + f14
			f24 = f14 - f18
			f20 = f10 * f2
			f4 = f6 * f12
			f22 = f4 * f16
			f10 = f20 - f22
			f6 = f10 + f28
			f10 = s0->f_14
			f4 = f8 * f6
			f8 = f22 - f20
			f6 = f10 * f8
			f8 = s0->f_18
			f10 = f4 + f6
			f4 = f24 - f18
			f6 = f4 + f0
			f4 = f8 * f6
			f6 = s0->f_1C
			f8 = f10 + f4
			f10 = f6 * f24
			f26 = f8 + f10
			break;
		//default:
		//	t0 = s0->_04;
		//	goto 250
	}

	24c:
	// this switch uses a dispatch table @ 80040710
	// note: omitted if(val>=10) skip
	switch(s0->byte_04 - 1)
	{
		case 0: // 274
			bone->rotation.x = f26;
			break; // goto 2fc
		case 1: // 27c
			bone->rotation.y = f26;
			break;
		case 2: // 284
			bone->rotation.z = f26;
			break;
		case 3: // 28C
			a0 = a3 + 0x1C	
			if(f26 < f30)
			{

			}
			
		case 4: // 2d0
			bone->position.x = f26;
			break;
		case 5: // 2d8
			bone->position.y = f26;
			break;
		case 6: // 2e0
			bone->position.z = f26;
			break;
		case 7: // 2e8
			bone->scale.x = f26;
			break;
		case 8: // 2f0
			bone->scale.y = f26;
			break;
		case 9: // 2f8
			bone->scale.z = f26;
			break;
	}

	2fc:
	s0 = s0->w_00;
	if(s0 != 0) /*t6 = s0->byte_05 */ goto loop;

	sp_54 = f26

	if(bone->_74 == *(float*)0x80040738)
	{
		bone->f_74 = *(float*)0x8004073C;
	}
}

// 800097E0
int clear_next_transform(a0)
{
	// zeros out a transformation struct
}


/*

800097E0 write the animation command number

load animation command info from 8029E4CC

800087D8 stuff

*/

/*

gfx_entity_t (0x88 bytes) // kirby @ 802AC168

	_00 gfx_entity_t*
	_04 ptr (802964b8)
	_08 ptr (null for kirby)
	_0C gfx_entity_t*
	_10 gfx_entity_t* (same as _00 for kirby)
	_14 gfx_entity_t*
	_18 ptr
	_1C vec3_t position
	_28 ptr (same as _18)
	_2c ? not touched for kirby
	_30 vec3_t rotation
	_3C dunno
	_40 vec3_t scale
	_4C
	_50
	_54 byte
	_55 ?
	_56 byte
	_58 ptr
	_5C (passed to 800087AC)
	_60 (passed to 800087AC)
	_64 (passed to 800087AC)
	_68 (passed to 800087AC)
	_6c gfx_tranform_t* transform // pointer to animation transformations
	_70 not touched?
	_74 float (0xff7fffff ??)
	_78 float
	_7C not touched?
	_80 ?

gfx_transform_t

	_00 gfx_transform_t* pointer to next transformation?
	_04 u8 vector_num - index of vector to update 0-9
	_05 u8 operation_num - transformation operation 0,1-3
	_06 u16 unused padding
	_08 float
	_0C float
	_10 float
	_14 float
	_18 float
	_1C float
	_20 u32 unused?

*/