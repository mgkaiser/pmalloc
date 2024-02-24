# Output name
PRG=pmalloc.pgz

# Sources
ASM_SRCS =
C_SRCS = example_basic.c pmalloc.c

# Model
MODEL = --code-model=large --data-model=large
LIB_MODEL = lc-ld

# Linker
LINKER=linker/c256-fmx-plain.scm

# Include files 
INC='/c/Program Files (x86)/Calypsi-65816/contrib/Foenix-SDK/include'

# Dest
DEST=~/SD/

# Debug
DEBUG=-D DEBUG

# Other stuff
VPATH = src
OBJS = $(ASM_SRCS:%.s=obj/%.o) $(C_SRCS:%.c=obj/%.o)
LIBS=clib-$(LIB_MODEL)-foenix.a

obj/%.o: %.s
	as65816 --core=65816 $(MODEL) $(DEBUG)--target=Foenix --list-file=$(@:%.o=%.lst) -o $@ $<

obj/%.o: %.c
	cc65816 --core=65816 $(MODEL) $(DEBUG) --target=Foenix -O2 --speed --list-file=$(@:%.o=%.lst) -I$(INC) -o $@ $<
	#cc65816 --core=65816 $(MODEL) $(DEBUG) --target=Foenix -O2 --speed --list-file=$(@:%.o=%.lst) -I$(INC) --assembly-source $(@:%.o=%.s) -o $@ $<

$(PRG): $(OBJS)
	ln65816 --target=Foenix $(LINKER) --output-format=pgz $^ -o $@ $(LIBS) --cross-reference --rtattr printf=nofloat --rtattr cstartup=Foenix --list-file=$(PRG:%.pgz=%.lst)

clean:
	-rm $(PRG:%.pgz=%.lst) $(OBJS:%.o=%.lst) $(OBJS) $(PRG) 

install: $(PRG)
	-cp $(PRG) $(DEST)
