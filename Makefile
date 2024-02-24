# Output name
PRG				= pmalloc_test.pgz
LIB				= lib/pmalloc.a

# Sources
ASM_SRCS 		=
C_SRCS 			= pmalloc_test.c
LIB_ASM_SRCS 	=
LIB_C_SRCS 		= pmalloc.c

# Model
MODEL 			= --code-model=large --data-model=large
LIB_MODEL 		= lc-ld

# Linker
LINKER			= linker/c256-fmx-plain.scm

# Include files 
INC				= '/c/Program Files (x86)/Calypsi-65816/contrib/Foenix-SDK/include'

# Dest
DEST			= ~/SD/

# Debug
#DEBUG			 = -D DEBUG

# Other stuff
VPATH 			= src
LIB_OBJS 		= $(LIB_ASM_SRCS:%.s=obj/%.o) $(LIB_C_SRCS:%.c=obj/%.o)
OBJS 			= $(ASM_SRCS:%.s=obj/%.o) $(C_SRCS:%.c=obj/%.o)
LIBS			= clib-$(LIB_MODEL)-foenix.a $(LIB)

obj/%.o: %.s
	as65816 --core=65816 $(MODEL) $(DEBUG)--target=Foenix --list-file=$(@:%.o=%.lst) -o $@ $<

obj/%.o: %.c
	cc65816 --core=65816 $(MODEL) $(DEBUG) --target=Foenix -O2 --speed --list-file=$(@:%.o=%.lst) -I$(INC) -Iinc -o $@ $<
	#cc65816 --core=65816 $(MODEL) $(DEBUG) --target=Foenix -O2 --speed --list-file=$(@:%.o=%.lst) -I$(INC) -Iinc --assembly-source $(@:%.o=%.s) -o $@ $<

$(LIB): $(LIB_OBJS)	
	nlib $(LIB) $(LIB_OBJS)	

$(PRG): $(OBJS) 
	ln65816 --target=Foenix $(LINKER) --output-format=pgz $^ -o $@ $(LIBS) --cross-reference --rtattr printf=nofloat --rtattr cstartup=Foenix --list-file=$(PRG:%.pgz=%.lst)

all: $(LIB) $(PRG) 

clean:
	-rm $(PRG:%.pgz=%.lst) $(OBJS:%.o=%.lst) $(OBJS) $(PRG) $(LIB)

install: all
	-cp $(PRG) $(DEST)
