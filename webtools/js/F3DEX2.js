/*

F3DEX2 Class for n64scene.js
shygoo 2017

*/

var F3DEX2 = {
	g_dl: function(ucp)
	{
		ucp.log('%cg_dl: jumping to ' + ucp.cmd_b.toString(16), 'color: white; background-color: green');
		var bBranch = (ucp.cmd_a >>> 16) & 0xFF;
		if(bBranch == 0)
		{
			ucp.stack.push(ucp.segptr);
		}
		ucp.segptr = ucp.cmd_b;
	},
	g_enddl: function(ucp)
	{
		ucp.log('%cg_enddl: display list ended', 'color: white; background-color: orange');
		
		if(ucp.stack.length == 0)
		{
			ucp.ended = true;
			ucp.log('%c(done, no return address)', 'color: white; background-color: orange');
			return;
		}
		
		ucp.segptr = ucp.stack.pop();
	},
	g_rdphalf_1: function(ucp) // set wordhi
	{
		ucp.wordhi = ucp.cmd_b;
	},
	g_rdphalf_2: function(ucp) // set wordlo
	{
		ucp.wordlo = ucp.cmd_b;
	},
	g_branch_z: function(ucp)
	{
		//ucp.stack.push(ucp.segptr); // render both
		ucp.segptr = ucp.wordhi;
	},
	g_vtx: function(ucp) // load vertices
	{	
		var nVertices = (ucp.cmd_a >>> 12) & 0xFF;
		var vBufIdx = (((ucp.cmd_a >>> 1) & 0x7F) - nVertices) * 16;
		var vAddr = ucp.cmd_b;
		
		ucp.log('%cg_vtx: loading ' + nVertices + ' vertices from ' + vAddr.toString(16) + ' to vbuf+' + vBufIdx, 'color: blue');
		
		for(var i = 0; i < nVertices; i++)
		{
			for(var j = 0; j < 16; j += 4)
			{
				var offset = i * 16 + j;
				ucp.vbuf_dv.setUint32(vBufIdx + offset, ucp.n64scene.getU32(vAddr + offset));
			}
		}
	},
	g_tri1: function(ucp)
	{
		var v0 = ((ucp.cmd_a >>> 17) & 0x7F) * 16;
		var v1 = ((ucp.cmd_a >>> 9) & 0x7F) * 16;
		var v2 = ((ucp.cmd_a >>> 1) & 0x7F) * 16;
		
		ucp.log('%cg_tri: ' + [v0, v1, v2].map(v => v / 16).join(' '), 'color: green');
		
		var geo = ucp.threeGeometry;
		var vbuf = ucp.vbuf_dv;
		
		var v0_x = vbuf.getInt16(v0 + 0);
		var v0_y = vbuf.getInt16(v0 + 2);
		var v0_z = vbuf.getInt16(v0 + 4);
		var v0_c = vbuf.getUint32(v0 + 12) >> 8; // color
		
		var v1_x = vbuf.getInt16(v1 + 0);
		var v1_y = vbuf.getInt16(v1 + 2);
		var v1_z = vbuf.getInt16(v1 + 4);
		var v1_c = vbuf.getUint32(v1 + 12) >> 8; // color
		
		var v2_x = vbuf.getInt16(v2 + 0);
		var v2_y = vbuf.getInt16(v2 + 2);
		var v2_z = vbuf.getInt16(v2 + 4);
		var v2_c = vbuf.getUint32(v2 + 12) >> 8; // color
		
		var face_idx = geo.vertices.length;
		
		geo.vertices.push(new THREE.Vector3(v0_x, v0_y, v0_z));
		geo.vertices.push(new THREE.Vector3(v1_x, v1_y, v1_z));
		geo.vertices.push(new THREE.Vector3(v2_x, v2_y, v2_z));
		
		var face = new THREE.Face3(face_idx + 0, face_idx + 1, face_idx + 2);
		face.vertexColors[0] = new THREE.Color(v0_c);
		face.vertexColors[1] = new THREE.Color(v1_c);
		face.vertexColors[2] = new THREE.Color(v2_c);
		
		geo.faces.push(face);
	},
	g_tri2: function(ucp)
	{
		var v0 = ((ucp.cmd_a >>> 17) & 0x7F) * 16;
		var v1 = ((ucp.cmd_a >>> 9) & 0x7F) * 16;
		var v2 = ((ucp.cmd_a >>> 1) & 0x7F) * 16;
		
		var v3 = ((ucp.cmd_b >>> 17) & 0x7F) * 16;
		var v4 = ((ucp.cmd_b >>> 9) & 0x7F) * 16;
		var v5 = ((ucp.cmd_b >>> 1) & 0x7F) * 16;
		
		ucp.log('%cg_tri2: ' + [v0, v1, v2, v3, v4, v5].map(v => v / 16).join(' '), 'color: green');
		
		var geo = ucp.threeGeometry;
		var vbuf = ucp.vbuf_dv;
		
		var v0_x = vbuf.getInt16(v0 + 0);
		var v0_y = vbuf.getInt16(v0 + 2);
		var v0_z = vbuf.getInt16(v0 + 4);
		var v0_c = vbuf.getUint32(v0 + 12) >> 8; // color
		
		var v1_x = vbuf.getInt16(v1 + 0);
		var v1_y = vbuf.getInt16(v1 + 2);
		var v1_z = vbuf.getInt16(v1 + 4);
		var v1_c = vbuf.getUint32(v1 + 12) >> 8; // color
		
		var v2_x = vbuf.getInt16(v2 + 0);
		var v2_y = vbuf.getInt16(v2 + 2);
		var v2_z = vbuf.getInt16(v2 + 4);
		var v2_c = vbuf.getUint32(v2 + 12) >> 8; // color
		
		var v3_x = vbuf.getInt16(v3 + 0);
		var v3_y = vbuf.getInt16(v3 + 2);
		var v3_z = vbuf.getInt16(v3 + 4);
		var v3_c = vbuf.getUint32(v3 + 12) >> 8; // color
		
		var v4_x = vbuf.getInt16(v4 + 0);
		var v4_y = vbuf.getInt16(v4 + 2);
		var v4_z = vbuf.getInt16(v4 + 4);
		var v4_c = vbuf.getUint32(v4 + 12) >> 8; // color
		
		var v5_x = vbuf.getInt16(v5 + 0);
		var v5_y = vbuf.getInt16(v5 + 2);
		var v5_z = vbuf.getInt16(v5 + 4);
		var v5_c = vbuf.getUint32(v5 + 12) >> 8; // color
		
		var face_idx = geo.vertices.length;
		
		geo.vertices.push(new THREE.Vector3(v0_x, v0_y, v0_z));
		geo.vertices.push(new THREE.Vector3(v1_x, v1_y, v1_z));
		geo.vertices.push(new THREE.Vector3(v2_x, v2_y, v2_z));
		
		geo.vertices.push(new THREE.Vector3(v3_x, v3_y, v3_z));
		geo.vertices.push(new THREE.Vector3(v4_x, v4_y, v4_z));
		geo.vertices.push(new THREE.Vector3(v5_x, v5_y, v5_z));
		
		var face_a = new THREE.Face3(face_idx + 0, face_idx + 1, face_idx + 2);
		face_a.vertexColors[0] = new THREE.Color(v0_c);
		face_a.vertexColors[1] = new THREE.Color(v1_c);
		face_a.vertexColors[2] = new THREE.Color(v2_c);
		
		var face_b = new THREE.Face3(face_idx + 3, face_idx + 4, face_idx + 5);
		face_b.vertexColors[0] = new THREE.Color(v3_c);
		face_b.vertexColors[1] = new THREE.Color(v4_c);
		face_b.vertexColors[2] = new THREE.Color(v5_c);
		
		geo.faces.push(face_a);
		geo.faces.push(face_b);
	},
	g_geometrymode: function(ucp)
	{
		ucp.log('g_geometrymode [unimplemented]');
	},
	g_setenvcolor: function(ucp)
	{
		var color = (ucp.cmd_b >>> 8).toString(16);
		while(color.length!=6) color="0"+color;
		ucp.log('g_setenvcolor 0x' + ucp.cmd_b.toString(16) + " %c  ", 'background-color: #' + color);
		ucp.envcolor = ucp.cmd_b;
	},
	g_setblendcolor: function(ucp)
	{
		var color = (ucp.cmd_b >>> 8).toString(16);
		while(color.length!=6) color="0"+color;
		ucp.log('g_setblendcolor 0x' + ucp.cmd_b.toString(16) + " %c  ", 'background-color: #' + color);
		ucp.envcolor = ucp.cmd_b;
	},
	g_setprimcolor: function(ucp)
	{
		var color = (ucp.cmd_b >>> 8).toString(16);
		while(color.length!=6) color="0"+color;
		ucp.log('g_setprimcolor 0x' + ucp.cmd_b.toString(16) + " %c  ", 'background-color: #' + color);
		ucp.envcolor = ucp.cmd_b;
	},
	g_ucploadsync: function(ucp)
	{
		ucp.log('g_ucploadsync [unimplemented]');
	},
	g_ucppipesync: function(ucp)
	{
		ucp.log('g_ucppipesync [unimplemented]');
	},
	g_setothermode_l: function(ucp)
	{
		ucp.log('g_setothermode_l [unimplemented]');
	},
	g_setothermode_h: function(ucp)
	{
		ucp.log('g_setothermode_h [unimplemented]');
	},
	g_settilesize: function(ucp)
	{
		ucp.log('g_settilesize [unimplemented]');
	},
	g_loadblock: function(ucp)
	{
		ucp.log('g_loadblock [unimplemented]');
	},
	g_settile: function(ucp)
	{
		var fmt  = (ucp.cmd_a >>> 21) & 7
		var siz  = (ucp.cmd_a >>> 19) & 3;
		var line = (ucp.cmd_a >>> 9) & 0x1FF;
		var tmem = (ucp.cmd_a & 0x1FF);
		
		var tile     = (ucp.cmd_b >>> 24) & 7;
		var palette  = (ucp.cmd_b >>> 20) & 15;
		var cmT      = (ucp.cmd_b >>> 18) & 3;
		var maskT    = (ucp.cmd_b >>> 14) & 15;
		var shiftT   = (ucp.cmd_b >>> 10) & 15;
		var cmS      = (ucp.cmd_b >>>  8) & 3;
		var maskS    = (ucp.cmd_b >>>  4) & 15;
		var shiftS   = (ucp.cmd_b & 15);
		
		// 11110101 10 0100000000000000000000 00000111000000010100000001010000
		// 11110101 10 0000000000010000000000 00000000000010010100001001010000
		
		fmt = F3DEX2.im_formats[fmt]
		siz = F3DEX2.im_sizes[siz]
		
		var s2 = ucp.cmd_b.toString(2)
		
		while(s2.length!=32) s2="0"+s2;
		
		ucp.log('g_settile ' + fmt + " " + siz + " " + ucp.cmd_a.toString(2) + " " + s2)
	},
	g_settimg: function(ucp)
	{
		var fmt = (ucp.cmd_a >> 21) & 7;
		var siz = (ucp.cmd_a >> 19) & 3;
		fmt = F3DEX2.im_formats[fmt]
		siz = F3DEX2.im_sizes[siz]
		var width = (ucp.cmd_a & 0xFFF) + 1;
		ucp.log('g_settimg ' + fmt + ' ' + siz + ' ' + 'w:' +  width + ' ' + ucp.cmd_b.toString(16));
	},
	g_setcombine: function(ucp)
	{
		ucp.log('g_setcombine ' + ucp.cmd_a.toString(16) + " " + ucp.cmd_b.toString(16));
	}
}

F3DEX2.im_formats = ['G_IM_FMT_RGBA', 'G_IM_FMT_YUV', 'G_IM_FMT_CI', 'G_IM_FMT_IA', 'G_IM_FMT_I'];
F3DEX2.im_sizes = ['G_IM_SIZ_4b', 'G_IM_SIZ_8b', 'G_IM_SIZ_16b', 'G_IM_SIZ_32b'];

F3DEX2.g_quad = F3DEX2.g_tri2;

F3DEX2._class = {
	0x01: F3DEX2.g_vtx,
	0x04: F3DEX2.g_branch_z,
	0x05: F3DEX2.g_tri1,
	0x06: F3DEX2.g_tri2,
	0x07: F3DEX2.g_quad,
	0xD9: F3DEX2.g_geometrymode,
	0xDE: F3DEX2.g_dl,
	0xDF: F3DEX2.g_enddl,
	0xE1: F3DEX2.g_rdphalf_1,
	0xE2: F3DEX2.g_setothermode_l,
	0xE3: F3DEX2.g_setothermode_h,
	0xE6: F3DEX2.g_ucploadsync,
	0xE7: F3DEX2.g_ucppipesync,
	0xF1: F3DEX2.g_rdphalf_2,
	0xF2: F3DEX2.g_settilesize,
	0xF3: F3DEX2.g_loadblock,
	0xF5: F3DEX2.g_settile,
	0xF9: F3DEX2.g_setblendcolor,
	0xFA: F3DEX2.g_setprimcolor,
	0xFB: F3DEX2.g_setenvcolor,
	0xFC: F3DEX2.g_setcombine,
	0xFD: F3DEX2.g_settimg,
};
