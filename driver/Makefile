
DRIVER_NET_DIR		= $(DRIVER_DIR)/net
DRIVER_TTY_DIR		= $(DRIVER_DIR)/tty
DRIVER_SPI_DIR		= $(DRIVER_DIR)/spi
DRIVER_SERIAL_DIR	= $(DRIVER_DIR)/serial

obj-y += $(DRIVER_DIR)/led.o
obj-y += $(DRIVER_DIR)/led_driver.o
obj-y += $(DRIVER_DIR)/led_device.o
obj-y += $(DRIVER_DIR)/key.o
obj-y += $(DRIVER_DIR)/nand.o
obj-y += $(DRIVER_DIR)/char_dev.o
obj-y += $(DRIVER_DIR)/device.o
obj-y += $(DRIVER_DIR)/timer.o
obj-y += $(DRIVER_DIR)/platform.o
obj-y += $(DRIVER_DIR)/block_dev.o
obj-y += $(DRIVER_DIR)/test_platform.o

include $(DRIVER_TTY_DIR)/Makefile
include $(DRIVER_SPI_DIR)/Makefile
include $(DRIVER_SERIAL_DIR)/Makefile
