CROSS_COMPILE = arm-elf-
NAME = TinyOs
#=============================================================================#
CFLAGS += -g -Wall -I ./include -nostdlib
LDFLAGS = -TTinyOs.lds -Ttext 30000000 

LD	= $(CROSS_COMPILE)ld
CC	= $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
#============================================================================#
OBJSss 	:= $(wildcard arch/arm/*.s) $(wildcard arch/arm/*.c) $(wildcard lib/*.c) \
		   $(wildcard proc/*.c) $(wildcard kernel/*.c) $(wildcard user_program/*.c) \
		   $(wildcard driver/*.c) $(wildcard ipc/*.c)  $(wildcard driver/tty/*.c)\
		   $(wildcard driver/serial/*.c) $(wildcard fs/*.c) $(wildcard mm/*.c) \
		   $(wildcard user_program/*.s)
OBJSs  	:= $(patsubst %.s, %.o, $(OBJSss))
OBJS 	:= $(patsubst %.c, %.o, $(OBJSs))
#============================================================================#
%.o: %.s
	$(CC) $(CFLAGS) -c -o  $@ $<
%.o: %.c
	$(CC) $(CFLAGS) -c -o  $@ $<

all: $(OBJS)
	$(CC) -static -nostartfiles -nostdlib $(LDFLAGS) $? -o $@ -lgcc
	$(OBJCOPY) -O binary $@ $(NAME).bin
	$(OBJDUMP) -D $@ > $(NAME).dis 
#============================================================================#
clean:
	rm -rf $(OBJS) *.elf *.bin *.dis *.o all log cscope.* $(NAME)
#============================================================================#
