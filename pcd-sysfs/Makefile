obj-m := pcd_sysfs.o
pcd_sysfs-objs += pcd_platform_driver_dt_sysfs.o  pcd_syscall.o
SRC := $(shell pwd)

all:
	make -C /lib/modules/5.15.148-2-MANJARO/build/ M=$(PWD)

	mkdir - p build
	mv

modules_install:
	make -C /lib/modules/5.15.148-2-MANJARO/build/ M=$(PWD) modules_install


clean:


	$(MAKE) -C /lib/modules/5.15.148-2-MANJARO/build/ M=$(PWD) clean


