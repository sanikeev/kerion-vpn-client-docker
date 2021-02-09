ARCHIVE ?= drivers/ipf/linux/kvnet.tgz

kvnet-others = compile.cmd Makefile
kvnet-srcs = init.c io_dev.c io_read.c io_write.c net_dev.c queue.c utils.c vnet.c
kvnet-hdrs = defs.h hdrs.h init.h ioctl.h io_dev.h io_read.h io_write.h lin_specific.h net_dev.h queue.h utils.h vnet.h

kvnet-srcs-abs := $(patsubst %.c,drivers/vpn/linux/%.c,${kvnet-srcs})
kvnet-hdrs-abs := $(patsubst %.h,drivers/vpn/linux/%.h,${kvnet-hdrs})
kvnet-others-abs := $(patsubst %,drivers/vpn/linux/%,${kvnet-others})

obj-m		+= kvnet.o
kvnet-objs := $(patsubst %.c,%.o,${kvnet-srcs})

EXTRA_CFLAGS += -O -Wall -DMODULE -DWINNT=0 -DDBG=0 -D_LINUX
	
clean:
	-rm -f *.o *.ko *.cmd *.flags *.mod.c

archive:
	mkdir -p $(shell dirname $(ARCHIVE))
	cd ../../.. && tar -czvf $(ARCHIVE) $(kvnet-srcs-abs) $(kvnet-hdrs-abs) $(libkvnet-srcs) $(kvnet-others-abs) `find libkvnet -type f| grep -v CVS | grep -v CMakeLists.txt`

