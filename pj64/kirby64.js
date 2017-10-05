/*

Kirby64 debugging script
shygoo 2017

Drop me in /Project64/Scripts/

*/

var typeNames = {
	0x00: "item/enemy",
	0x03: "collectible",
	0x05: "warp",
	0x08: "boss",
};

var entities = {
	0x00: {
		0x00: "N-Z",
		0x01: "Rocky",
		0x02: "Bronto Burt",
		0x03: "Skud",
		0x04: "Gordo",
		0x05: "Shotzo",
		0x06: "Spark-i",
		0x07: "Bouncy",
		0x08: "Glunk",
		0x09: "[?? explodes]",
		0x0A: "Chilly",
		0x0B: "Propeller",
		0x0C: "Glom",
		0x0D: "Mahall",
		0x0E: "Poppy Bros. Jr.",
		0x0F: "Bivolt",
		0x10: "Splinter",
		0x11: "Gobblin",
		0x12: "Kany",
		0x13: "Bivolt again?",
		0x14: "Sirkibble",
		0x15: "Gabon",
		0x16: "Mariel",
		0x17: "Large I3",
		0x18: "Snipper",
		0x19: "[?? explodes again?]",
		0x1A: "Bonehead",
		0x1B: "Squibbly",
		0x1C: "Bobo",
		0x1D: "Bo",
		0x1E: "Punc",
		0x1F: "Mite",
		0x20: "Sandman",
		0x21: "Flopper",
		0x22: "Kapar",
		0x23: "Maw",
		0x24: "Drop",
		0x25: "Pedo",
		0x26: "Noo",
		0x27: "Tick",
		0x28: "Cairn",
		0x29: "[?? invisible]",
		0x2A: "Pompey",
		0x2B: "Hack",
		0x2C: "Burnis",
		0x2D: "Fishbone",
		0x2E: "Frigis",
		0x2F: "Sawyer",
		0x30: "Turbite",
		0x31: "Plugg",
		0x32: "Ghost knight",
		0x33: "Zoos",
		0x34: "Kakti",
		0x35: "Rockn",
		0x36: "Chacha",
		0x37: "Galbo",
		0x38: "Bumber",
		0x39: "Scarfy",
		0x3a: "Nruff",
		0x3b: "Emp",
		0x3c: "Magoo",
		0x3d: "Yariko",
		0x3e: "invisible?",
		0x3f: "Wall Shotzo",
		0x40: "Keke",
		0x41: "Sparky",
		0x42: "Ignus", // falling rocks
		0x43: "Flora",
		0x44: "Putt",
		0x45: "Pteran",
		0x46: "Mumbies",
		0x47: "Pupa",
		0x48: "Mopoo",
		0x49: "Zebon",
		0x4a: "ivisible?",
		0x4b: "falling rocks sometimes blue",
		0x4c: "falling rocks sometimes blue bigger?",

	},
	0x01: {
		0x00: "Waddle Dee Boss",
		0x01: "Ado Boss",
		0x02: "DeeDeeDee Boss"
	},
	0x02: {
		0x00: "Whispy Woods",
		//0x01: "Waddle Dee (Boss)"
	},
	0x03: {
		0x00: "Maxim Tomato",
		0x01: "Sandwich",
		0x02: "Cake",
		0x03: "Steak",
		0x04: "Ice Cream Bar",
		0x05: "Invinsible Candy",
		0x06: "Yellow Star",
		0x07: "Blue Star",
		0x08: "[crashes]",
		0x09: "1up",
		0x0a: "[crashes]", // used in picnic area
		0x0b: "Flower",
		0x0c: "School of fish",
		0x0d: "Butterfly",
	},
	0x05: {
		// warps
	},
	0x07: {
		0x01: "Ado (Gives maxim tomato)" // param 1
	},
	0x08: {
		0x00: "N-Z Boss",
		0x01: "Bouncy Boss",
		0x02: "Kakti Boss",
		0x03: "?",
		0x04: "Spark-i Boss",
		0x05: "Tick Boss",
		0x06: "Kany Boss",
		0x07: "Kapar Boss",
		0x08: "Blowfish boss",
		0x09: "Galbo boss",
		0x0A: "drop boss?",
		//...
	}
}

var spawn = {
	enabled: false,
	type: 0x00,
	id: 0x00,
	param: 0x00,
	enable: function(cfg)
	{
		this.type = cfg.type;
		this.id = cfg.id;
		this.param = cfg.param;
		this.enabled = true;
	},
	disable: function()
	{
		this.enabled = false;
	}
}

// items/enemies

events.onexec(0x800FC9C0, function()
{
	var area  = mem.u8[gpr.a1 + 0x00];
	var type  = mem.u8[gpr.a1 + 0x01]; // entity class?
	var id    = mem.u8[gpr.a1 + 0x02];
	var param = mem.u8[gpr.a1 + 0x03];
	
	console.log("--------")
	console.log("Spawning -- area: " + area.hex(2) + ", type: " + type.hex(2) + " " + typeNames[type] + ", id: " + id.hex(2) + ", param: " + param.hex(2))
	
	if(spawn.enabled && type == spawn.type)
	{
		mem.u8[gpr.a1 + 0x01] = spawn.type;
		mem.u8[gpr.a1 + 0x02] = spawn.id;
		mem.u8[gpr.a1 + 0x03] = spawn.param;
		type = spawn.type;
		id = spawn.id;
		param = spawn.param;
		console.log("Forced: " + area.hex(2) + " " + type.hex(2) + " " + id.hex(2) + " " + param.hex(2))
	}
	
	var entityName;
	
	if (type in entities)
	{
		entityName = entities[type][id];
	}
	
	console.log("Spawned " + entityName);
})

// warps?

events.onexec(0x800F7C58, function()
{
	console.log(mem.u32[gpr.v1 + 0x00].hex(8))
})

events.onexec(0x800A9ABC, function()
{
	console.print("Resource: " + gpr.a0.hex() + " -> ");
})

events.onexec(0x800A9B2C, function()
{
	var romAddr = gpr.t3 + gpr.t2;
	var size = gpr.a2;
	console.log(romAddr.hex() + " [" + size.hex() + "]")
})