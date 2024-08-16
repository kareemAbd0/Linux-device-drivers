obj-m := keypad_module.o


keypad_module-objs += keypad_platform_driver.o  keypad.o

SRC := $(shell pwd)

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(SRC)
#	make -C /lib/modules/5.15.150-1-MANJARO/build/ M=$(SRC)

	mkdir -p build
	mv *.ko build
	mv *.mod.c build
	mv *.o build
	mv *.symvers build
	mv *.order build
	mv .*.cmd build
	mv *.mod build


modules_install:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(SRC) modules_install
	#make -C /lib/modules/5.15.150-1-MANJARO/build/ M=$(SRC) modules_install


clean:


	$(MAKE) -C /lib/modules/$(shell uname -r)/build/ M=$(SRC) clean


