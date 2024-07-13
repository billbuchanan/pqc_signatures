BUILDDIR=$(CURDIR)/build

LIBGMP=build/gmp/lib/libgmp.so
LIBMPFR=build/mpfr/lib/libmpfr.so
LIBFLINT=build/flint/lib/libflint.so
LIBFPLLL=build/fplll/lib/libfplll.so

ALLLIBS=$(LIBGMP) $(LIBMPFR) $(LIBFLINT) $(LIBFPLLL)

.PHONY: clean all

all: build $(ALLLIBS)

build:
	mkdir build

$(LIBGMP):
	cd gmp-6.2.1 && ./configure --prefix=${BUILDDIR}/gmp \
    		--disable-static --enable-cxx --with-pic 
	make -C gmp-6.2.1 -j$(nproc)
	make -C gmp-6.2.1 install

$(LIBMPFR): $(LIBGMP)
	cd mpfr-4.2.0 && ./configure --prefix=${BUILDDIR}/mpfr \
		--disable-static --with-pic --with-gmp=${BUILDDIR}/gmp
	make -C mpfr-4.2.0 -j$(nproc)
	make -C mpfr-4.2.0 install

$(LIBFLINT): $(LIBGMP) $(LIBMPFR)
	cd flint-2.9.0 && ./configure --disable-static \
		--with-gmp=$(BUILDDIR)/gmp \
		--with-mpfr=$(BUILDDIR)/mpfr \
		--prefix=$(BUILDDIR)/flint
	make -C flint-2.9.0 -j$(nproc)
	make -C flint-2.9.0 install

$(LIBFPLLL): $(LIBGMP) $(LIBMPFR)
	cd fplll-5.4.4 && ./configure --disable-static \
		--with-gmp=$(BUILDDIR)/gmp \
		--with-mpfr=$(BUILDDIR)/mpfr \
		--without-qd \
		--prefix=$(BUILDDIR)/fplll   
	make -C fplll-5.4.4 -j$(nproc)
	make -C fplll-5.4.4 install

clean:
	rm -rf build
	make -C gmp-6.2.1 clean
	make -C mpfr-4.2.0 clean
	make -C flint-2.9.0 clean
	make -C fplll-5.4.4 clean
