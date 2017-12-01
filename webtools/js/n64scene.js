/*

Fast3D renderer
shygoo 2017

Uses three.js:
	https://github.com/mrdoob/three.js/

*/

function N64Scene(domSelector, config)
{
	config = config || {};

	this.rom_ab;
	this.rom_dv;
	
	this.segments_ab = []; // ArrayBuffers of segmented memory
	this.segments_dv = []; // DataViews of segmented memory
	
	// ucode processing
	this.ucp = new N64Scene.UCP(this);
	
	this.scene = new THREE.Scene();
	
	this.width = config.width || 960;
	this.height = config.height || 640;

	this.bgColor = config.bgColor || 0x000000;
	
	var renderer;
	renderer = new THREE.WebGLRenderer();	
	renderer.setPixelRatio(window.devicePixelRatio);
	renderer.setSize(this.width, this.height);
	renderer.setScissorTest(true);
	renderer.alpha = true;
	
	renderer.setClearColor(0x222222, 1.0);
	renderer.setViewport(0, 0, this.width, this.height);
	renderer.setScissor(0, 0, this.width, this.height);
	
	this.renderer = renderer;
	
	var domElement = this.domElement = renderer.domElement;
	domElement.width = this.width;
	domElement.height = this.height;
	domElement.style.width = this.width + "px";
	domElement.style.height = this.height + "px";
	domElement.tabIndex = '0'; // For keyboard input
	
	var domContainer = document.querySelector(domSelector);
	domContainer.appendChild(domElement);
	
	var aspectRatio = this.width / this.height;
	this.camera = new THREE.PerspectiveCamera(60, aspectRatio, 1, 15000);
	this.camera.rotation.order = "YXZ";
	
	this.resetCamera();
	
	this.animating = false;
	
	this.keysDown = {};
	
	var keysDown = this.keysDown;
	
	this.domElement.onkeydown = function(e)
	{
		keysDown[e.keyCode] = true;
		e.preventDefault();
		return false;
	}
	
	this.domElement.onkeyup = function(e)
	{
		delete keysDown[e.keyCode];
		e.preventDefault();
		return false;
	}
	
	this.mouseButtonsDown = {};
	var mouseButtonsDown = this.mouseButtonsDown;
	
	this.domElement.onmousedown = function(e)
	{
		this.focus();
		e.preventDefault();
		mouseButtonsDown[e.button] = true;
		
		if(e.button == 2)
		{
			this.style.cursor = 'move';
		}
		else if(e.button == 1)
		{
			this.style.cursor = 'none';
		}
	}
	
	this.domElement.onmouseup = function(e)
	{
		delete mouseButtonsDown[e.button];
		domElement.style.cursor = 'default';
	}
	
	this.domElement.oncontextmenu = function(e)
	{
		return false;
	}
	
	var mouseX = 0, mouseY = 0;
	
	var camera = this.camera;
	
	this.domElement.onmousemove = function(e)
	{
		var rect = this.getBoundingClientRect();
		
		var x = e.clientX - rect.left;
		var y = e.clientY - rect.top;
		
		var movementX = x - mouseX;
		var movementY = y - mouseY;
		
		if(mouseButtonsDown[2])
		{
			camera.translateX(-movementX);
			camera.translateY(movementY);
		}
		if(mouseButtonsDown[1])
		{
			camera.rotateY(-movementX / 100);
			camera.rotateX(-(movementY / 100));
			camera.rotation.z = 0;
		}
		
		mouseX = x;
		mouseY = y;
	}
	
	this.domElement.onwheel = function(e)
	{
		camera.translateZ(e.deltaY);
	}
	
	this.keyEvents = {
		27: function(_this){ _this.resetCamera() }, // esc - reset camera position
		87: function(_this){ _this.camera.translateZ(-10); }, // w - move forward
		65: function(_this){ _this.camera.translateX(-10); }, // a - pan left
		83: function(_this){ _this.camera.translateZ( 10); }, // s - move backward
		68: function(_this){ _this.camera.translateX( 10); }, // d - pan right
		90: function(_this){ _this.camera.translateY(-10); }, // z - move down
		88: function(_this){ _this.camera.translateY( 10); }, // x - move up
		40: function(_this){ _this.camera.rotateX( -0.05); }, // down - rotate down
		38: function(_this){ _this.camera.rotateX(  0.05); }, // up - rotate up
		37: function(_this){ _this.camera.rotateY(  0.05); _this.camera.rotation.z = 0;}, // left - rotate left
		39: function(_this){ _this.camera.rotateY( -0.05); _this.camera.rotation.z = 0;}  // right - rotate right
	}
	
	this.wireframe = false;
}

N64Scene.prototype = {
	add: function(mesh)
	{
		this.scene.add(mesh);
	},
	resetCamera: function()
	{
		with(this.camera.position)
		{
			x = 0;
			y = 50;
			z = 500;
		}
		
		with(this.camera.rotation)
		{
			x = 0;
			y = 0;
			z = 0;
		}
	},
	draw: function()
	{
		var renderer = this.renderer;
		
		//var envcolor = this.ucp.envcolor >> 8;
		//var envcolor_alpha = (this.ucp.envcolor & 0xFF) / 255;
		
		renderer.render(this.scene, this.camera);
	},
	reset: function()
	{
		var scene = this.scene;
		while (scene.children.length)
		{
			scene.remove(scene.children[0]);
		}
		this.ucp.reset();
	},
	processKeyEvents: function()
	{
		for(var k in this.keysDown)
		{
			if(k in this.keyEvents)
			{
				this.keyEvents[k](this);
			}
		}
	},
	animate: function()
	{
		if(this.animating)
		{
			requestAnimationFrame(this.animate.bind(this));
			this.processKeyEvents();
			this.draw();
		}
	},
	startAnimation: function()
	{
		this.animating = true;
		this.animate();
	},
	stopAnimation: function()
	{
		this.animating = false;
	},
	addVertexLineMesh: function(vertexBuffer, vertexStructSize, xyzOffset) // test vertices
	{
		vertexStructSize = vertexStructSize || 16;
		xyzOffset = xyzOffset || 0;
		
		var dv = new DataView(vertexBuffer);
		
		var geometry = new THREE.Geometry();
		
		for(var i = 0; i < dv.byteLength; i += vertexStructSize)
		{
			var x = dv.getInt16(i + xyzOffset + 0);
			var y = dv.getInt16(i + xyzOffset + 2);
			var z = dv.getInt16(i + xyzOffset + 4);
		
			var pos = new THREE.Vector3(x, y, z);
		
			geometry.vertices.push(pos);
		}
		
		var pointMaterial = new THREE.PointsMaterial({ size: 5, sizeAttenuation: true, color: 0xffff00, opacity: 0.75, transparent: true });
		var lineMaterial = new THREE.LineBasicMaterial({ color: 0x00ffff, opacity: 0.2, transparent: true, linewidth: 10 });
		var line = new THREE.Line(geometry, lineMaterial);
		var dots = new THREE.Points(geometry, pointMaterial);
		
		n64scene.scene.add(line);
		n64scene.scene.add(dots);
	},
	setRom: function(rom_ab)
	{
		this.rom_ab = rom_ab;
		this.rom_dv = new DataView(rom_ab);
	},
	loadSegment: function(segno, romStart, romEnd)
	{
		this.segments_ab[segno] = this.rom_ab.slice(romStart, romEnd);
		this.segments_dv[segno] = new DataView(this.segments_ab[segno]);
		//console.log(this.segments_ab, this.segments_dv, this.rom_ab.slice(romStart, romEnd));
	},
	loadSegmentDecode: function(segno, romStart, romEnd, cbDecode)
	{
		// cbDecode(ArrayBuffer encoded) -> ArrayBuffer decoded
		// For use with mio0, yay0, etc
		this.loadSegmentRaw(segno, romStart, romEnd);
		this.segments_ab[segno] = cbDecode(this.segments[segno]);
		this.segments_dv[segno] = new DataView(this.segments_ab[segno]);
	},
	// Fetch data from segmented memory
	getU32: function(segptr)
	{
		var segno = segptr >> 24;
		var offset = segptr & 0x00FFFFFF;
		
		if(!this.segments_dv[segno])
		{
			return false;
		}
		
		return this.segments_dv[segno].getUint32(offset);
	},
	getU16: function(segptr)
	{
		var segno = segptr >> 24;
		var offset = segptr & 0x00FFFFFF;
		return this.segments_dv[segno].getUint16(offset);
	},
	getU8: function(segptr)
	{
		var segno = segptr >> 24;
		var offset = segptr & 0x00FFFFFF;
		return this.segments_dv[segno].getUint8(offset);
	},
	getS16: function(segptr) // vert
	{
		var segno = segptr >> 24;
		var offset = segptr & 0x00FFFFFF;
		return this.segments_dv[segno].getInt16(offset);
	},
	getFloat: function(segptr)
	{
		var segno = segptr >> 24;
		var offset = segptr & 0x00FFFFFF;
		return this.segments_dv[segno].getFloat32(offset);
	},
	// microcode processing
	ucSetClass: function(ucode)
	{
		this.ucp.ucode = ucode;
	},
	ucExecute: function(segptr)
	{
		this.ucp.exec(segptr);
	},
	// Returns the internal geometry object
	ucFlushMesh: function()
	{
		var geometry = this.ucp.threeGeometry;
		var material = new THREE.MeshBasicMaterial({ side: THREE.DoubleSide });
		
		if(this.wireframe)
		{
			material.wireframe = true;
			material.color = new THREE.Color(255,255,255);
		}
		else
		{
			material.vertexColors = THREE.VertexColors;
		}
		
		this.ucp.threeGeometry = new THREE.Geometry();
		
		return new THREE.Mesh(geometry, material);
	},
	ucSetVerbose: function(bVerbose)
	{
		this.ucp.bVerbose = bVerbose;
	},
	setWireframe: function(bWireframe)
	{
		this.wireframe = bWireframe;
	}
};

N64Scene.UCP = function(n64scene)
{
	this.n64scene = n64scene;
	this.ucode; // Microcode class
	
	this.bVerbose = false;
	
	this.wordhi = 0;
	this.wordlo = 0;
	
	this.cmd_a; // First word of command
	this.cmd_b; // Second word of command
	this.geometryMode;
	
	this.reset();
}

N64Scene.UCP.prototype.reset = function()
{
	this.segptr = 0x00000000;
	this.envcolor = 0x00000000;
	this.stack = [];
	
	this.vbuf_ab = new ArrayBuffer(512);
	this.vbuf_dv = new DataView(this.vbuf_ab);
	this.ended = false;
	
	this.threeGeometry = new THREE.Geometry();
}

N64Scene.UCP.prototype.exec = function(segptr)
{
	this.segptr = segptr;
	this.ended = false;
	
	while(!this.ended)
	{
		this.cmd_a = n64scene.getU32(this.segptr);
		this.cmd_b = n64scene.getU32(this.segptr + 4);
		
		this.curptr = this.segptr; // for logging
		
		if(this.cmd_a == false)
		{
			console.log(this.curptr.toString(16) + ": %cucp ran into bad address, returning early", "color: white; background-color: red;");
			if(this.ucode.g_enddl)
			{
				this.ucode.g_enddl(this);
			}
			continue;
		}
		
		this.segptr += 8;
		
		var cmdByte = this.cmd_a >>> 24;
		
		if(cmdByte in this.ucode._class)
		{
			this.ucode._class[cmdByte](this);
			continue;
		}
		
		this.log(cmdByte.toString(16) + ' unhandled by microcode class');
	}
}

N64Scene.UCP.prototype.log = function(str, style)
{
	if(!this.bVerbose)
	{
		return;
	}
	
	var addr = this.curptr.toString(16);
	
	while(addr.length < 8)
	{
		addr = "0" + addr;
	}
	
	console.log(addr.toUpperCase() + ": " + str, style || '');
}

//todo move to utility class
// Convert hex string to ArrayBuffer
N64Scene.parseHexString = function(str)
{
	if(/[^a-fA-F\d\t\r\n ]/.test(str))
	{
		return null; // Text contains invalid characters
	}
	var arr = str.match(/(\d|[a-fA-F]){2}/g).map(function(a){return parseInt(a, 16)});
	return new Uint8Array(arr).buffer;
}