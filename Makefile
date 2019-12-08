CROSS_COMPILE 	= arm-linux-
AS 				= $(CROSS_COMPILE)as 
LD				= $(CROSS_COMPILE)ld 
CC				= $(CROSS_COMPILE)gcc
STRIP			= $(CROSS_COMPILE)strip
OBJCOPY			= $(CROSS_COMPILE)objcopy 
OBJDUMP 		= $(CROSS_COMPILE)objdump 

arch			= arm
CFLAGS 			= -g -Wall -I ./include -I ./arch/arm -nostdlib -fno-builtin
LDFLAGS 		= -Tyap.lds -Ttext 30000000 

# Usage:
#  $(call cmd, cc_o_c)              
quiet_cmd_cc_o_c = CC $(subst ../, ,$<)
cmd_cc_o_c = $(CC) $(CFLAGS)   -c $< -o $@
cmd = @echo  "$(quiet_cmd_cc_o_c)"; $(cmd_cc_o_c)


obj-y			=
TARGET 	 		= yap

TOP_DIR  		= .
FS_DIR  		= $(TOP_DIR)/fs
MM_DIR  		= $(TOP_DIR)/mm
IPC_DIR 		= $(TOP_DIR)/ipc
LIB_DIR 		= $(TOP_DIR)/lib
NET_DIR 		= $(TOP_DIR)/net
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
include $(NET_DIR)/Makefile
include $(TEST_DIR)/Makefile

DEPS = $(obj-y:.o=.d)

all: $(obj-y) $(DEPS)
	@echo LD $(TARGET)
	@$(CC) -static -nostartfiles -nostdlib $(LDFLAGS) $(obj-y) -o $(TARGET) -lgcc
	
	@echo OBJCOPY $(TARGET).bin
	@$(OBJCOPY) -O binary $(TARGET) $(TARGET).bin
	
	@echo OBJDUMP $(TARGET).dis
	@$(OBJDUMP) -D $(TARGET) > $(TARGET).dis 

-include $(DEPS)

%.o: %.c
	$(call cmd,cc_o_c)

%.o: %.s
	$(call cmd,cc_o_c)

%.o: %.S
	$(call cmd,cc_o_c)

%.d: %.c
	@gcc -MM -E $(CFLAGS) $< | sed 's,\(.*\)\.o[ :]*, $(<:.c=.o) $@: ,g' > $@

%.d: %.s
	@echo   > $@

clean:
	@echo rm *.o $(TARGET) *.bin *.dis *.d -rf
	@rm $(TARGET) $(obj-y) *.bin *.dis $(DEPS)

