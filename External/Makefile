TOPDIR	:= ..

sysroot:=$(shell pwd)/sysroot

unexport CFLAGS
unexport LDFLAGS

DIRS =

all: libuv
	@for dir in $(DIRS); do make -C $$dir; echo; done

clean:
	@for dir in $(DIRS); do make -C $$dir clean; echo; done

libuv:
	cd libuv-1.20.3; sh autogen.sh; sh configure --prefix=$(sysroot);
	$(MAKE) -C libuv-1.20.3 -j16
	$(MAKE) -C libuv-1.20.3 install
	#$(MAKE) -C libuv-1.19.2 check -j16
