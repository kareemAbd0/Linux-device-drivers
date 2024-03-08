obj-m := pcd_sysfs.o
pcd_sysfs-objs += pcd_platform_driver_dt_sysfs.o  pcd_syscall.o
SRC := $(shell pwd)

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(SRC)

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


clean:


	$(MAKE) -C /lib/modules/5.15.148-2-MANJARO/build/ M=$(SRC) clean


