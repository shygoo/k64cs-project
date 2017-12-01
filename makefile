TDIR=tmp

PLAINHEXT=plainhext/bin/plainhext

ANIMDIR=animation-testing

ROM_IN=K64.z64
ROM_OUT=K64_MOD.z64
ASM_MAIN=main.asm

AS=armips

N64_WRAP=$(TDIR)/armips_n64_wrapper.asm

$(ROM_OUT): $(ROM_IN) $(N64_WRAP) $(ASM_MAIN) $(ANIMDIR)/kirby-neutral-stance.bin
	$(AS) $(N64_WRAP) -root .

$(ANIMDIR)/kirby-neutral-stance.bin: $(PLAINHEXT) $(ANIMDIR)/kirby-neutral-stance.hex
	$(PLAINHEXT) $(ANIMDIR)/kirby-neutral-stance.hex $(ANIMDIR)/kirby-neutral-stance.bin

$(PLAINHEXT):
	make -C ./plainhext

############

$(N64_WRAP): makefile | $(TDIR)
	echo ".n64" > $(N64_WRAP)
	echo ".open \"$(ROM_IN)\", \"$(ROM_OUT)\", 0" >> $(N64_WRAP)
	echo ".include \"$(ASM_MAIN)\"" >> $(N64_WRAP)
	echo ".close" >> $(N64_WRAP)

$(TDIR):
	mkdir $(TDIR)