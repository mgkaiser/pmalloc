# Output name
ROOT			= pmalloc
PRG				= $(ROOT)_test.pgz
LIB				= lib/$(ROOT).a
LIB_H			= inc/$(ROOT).h

# Sources
ASM_SRCS 		=
C_SRCS 			= $(ROOT)_test.c
LIB_ASM_SRCS 	=
LIB_C_SRCS 		= $(ROOT).c

# Model
MODEL 			= --code-model=large --data-model=large
LIB_MODEL 		= lc-ld

# Linker
LINKER			= linker/c256-fmx-plain.scm

# Include files 
INC				= '/c/Program Files (x86)/Calypsi-65816/contrib/Foenix-SDK/include' 
LIB_INC			= ~/src/foenix_lib/inc

# Dest
DEST			= ~/SD/
LIB_DEST        = ../foenix_lib/

# Debug
DEBUG			 = -D DEBUG

# Other stuff
VPATH 			= src
LIB_OBJS 		= $(LIB_ASM_SRCS:%.s=obj/%.o) $(LIB_C_SRCS:%.c=obj/%.o)
OBJS 			= $(ASM_SRCS:%.s=obj/%.o) $(C_SRCS:%.c=obj/%.o)
LIBS			= clib-$(LIB_MODEL)-foenix.a ../foenix_lib/$(LIB)

obj/%.o: %.s
	as65816 --core=65816 $(MODEL) $(DEBUG)--target=Foenix --list-file=$(@:%.o=%.lst) -o $@ $<

obj/%.o: %.c
	cc65816 --core=65816 $(MODEL) $(DEBUG) --target=Foenix -O2 --speed --list-file=$(@:%.o=%.lst) -Iinc -I$(LIB_INC) -I$(INC) -o $@ $<
	#cc65816 --core=65816 $(MODEL) $(DEBUG) --target=Foenix -O2 --speed --list-file=$(@:%.o=%.lst) -Iinc -I$(LIB_INC) -I$(INC)  --assembly-source $(@:%.o=%.s) -o $@ $<

$(LIB): $(LIB_OBJS)	
	nlib $(LIB) $(LIB_OBJS)	

$(PRG): $(OBJS) 
	ln65816 --target=Foenix $(LINKER) --output-format=pgz $^ -o $@ $(LIBS) --cross-reference --rtattr printf=nofloat --rtattr cstartup=Foenix --list-file=$(PRG:%.pgz=%.lst)

all: $(LIB) install_lib $(PRG) 

clean:
	-rm $(PRG:%.pgz=%.lst) $(OBJS:%.o=%.lst) $(OBJS) $(PRG)
	-rm $(LIB:%.pgz=%.lst) $(LIB_OBJS:%.o=%.lst) $(LIB_OBJS) $(LIB)
	-rm $(DEST)$(PRG)
	-rm $(LIB_DEST)$(LIB)
	-rm $(LIB_DEST)$(LIB_H)

install_lib: $(LIB)	
	-cp $(LIB) $(LIB_DEST)lib/
	-cp $(LIB_H) $(LIB_DEST)inc/	

install: all
	-cp $(PRG) $(DEST)