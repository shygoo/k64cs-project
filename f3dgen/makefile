CC=gcc
CFLAGS=-Wall -O3 -s -fno-asynchronous-unwind-tables -ffunction-sections -fdata-sections -fno-ident

LD=gcc
LDFLAGS=-s -Wl,--gc-sections

SDIR=src
ODIR=obj
BDIR=bin

SFILES=$(wildcard $(SDIR)/*.c)
HFILES=$(wildcard $(SDIR)/*.h)
OFILES=$(patsubst $(SDIR)/%.c,$(ODIR)/%.o,$(SFILES))

TARGET=$(BDIR)/f3dgen

$(TARGET): $(OFILES) | $(BDIR) $(HFILES)
	$(LD) $(LDFLAGS) $(OFILES) -o $(TARGET)

$(ODIR)/%.o: $(SDIR)/%.c | $(ODIR)
	$(CC) $(CFLAGS) -c $^ -o $@

$(ODIR):
	mkdir $(ODIR)
	
$(BDIR):
	mkdir $(BDIR)
	
.PHONY: clean
clean:
	rm -rf $(ODIR)
	rm -rf $(BDIR)