#obj-m += $(addsuffix .o, $(notdir $(basename $(wildcard $(BR2_EXTERNAL_KERNEL_MODULE_PATH)/*.c))))

obj-m := i2s8.o
#mymod-objs := mymod-test1.o mymod-test2.o

#obj-m += $(addsuffix .o, codec)
ccflags-y := -DDEBUG -g -std=gnu99 -Wno-declaration-after-statement

.PHONY: all clean

all:
	$(MAKE) -C '$(LINUX_DIR)' M='$(PWD)' modules

clean:
	$(MAKE) -C '$(LINUX_DIR)' M='$(PWD)' clean
