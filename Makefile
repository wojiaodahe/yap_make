CROSS_COMPILE 	= arm-linux-
AS 				= $(CROSS_COMPILE)as 
LD				= $(CROSS_COMPILE)ld 
CC				= $(CROSS_COMPILE)gcc
STRIP			= $(CROSS_COMPILE)strip
OBJCOPY			= $(CROSS_COMPILE)objcopy 
OBJDUMP 		= $(CROSS_COMPILE)objdump 

arch			= arm
CFLAGS 			= -g -Wall -I ./include -I ./arch/arm -nostdlib
LDFLAGS 		= -Tyap.lds -Ttext 30000000 

obj-y			=
TARGET 	 		= yap

TOP_DIR  		= .
FS_DIR  		= $(TOP_DIR)/fs
MM_DIR  		= $(TOP_DIR)/mm
IPC_DIR 		= $(TOP_DIR)/ipc
LIB_DIR 		= $(TOP_DIR)/lib
ARCH_DIR 		= $(TOP_DIR)/arch
PROC_DIR 		= $(TOP_DIR)/proc
DRIVER_DIR 		= $(TOP_DIR)/driver
KERNEL_DIR 		= $(TOP_DIR)/kernel
USER_PRG_DIR    = $(TOP_DIR)/user_program
TEST_DIR		= $(TOP_DIR)/test

include $(ARCH_DIR)/Makefile
include $(LIB_DIR)/Makefile
include $(PROC_DIR)/Makefile
include $(KERNEL_DIR)/Makefile
include $(USER_PRG_DIR)/Makefile
include $(DRIVER_DIR)/Makefile
include $(IPC_DIR)/Makefile
include $(FS_DIR)/Makefile
include $(MM_DIR)/Makefile
include $(TEST_DIR)/Makefile

DEPS = $(obj-y:.o=.d)

all: $(obj-y) $(DEPS)
	$(CC) -static -nostartfiles -nostdlib $(LDFLAGS) $(obj-y) -o $@ -lgcc
	$(OBJCOPY) -O binary $@ $(TARGET).bin
	$(OBJDUMP) -D $@ > $(TARGET).dis 

-include $(DEPS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.s
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $<

%.d: %.c
	@gcc -MM -E $(CFLAGS) $< | sed 's,\(.*\)\.o[ :]*, $(<:.c=.o) $@: ,g' > $@

%.d: %.s
	@echo    

clean:
	rm all $(TARGET) $(obj-y) *.bin *.dis $(DEPS)

