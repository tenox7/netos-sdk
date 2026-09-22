#!/bin/bash
# Build gcc 2.95.3 (last series with an i960 back end) as a netOS cross compiler.
# Only the compiler proper: there is no target libc, so libgcc cannot be built.
set -e
V=2.95.3
T=i960-intel-nindy
LOG=${LOG:-/tmp}
D=/opt/i960/lib/gcc-lib/$T/$V
cd /tmp
[ -f gcc.tgz ] || wget -q -O gcc.tgz https://ftp.gnu.org/gnu/gcc/gcc-$V/gcc-core-$V.tar.gz
rm -rf gcc-$V g && tar xzf gcc.tgz
mkdir g && cd g
CFLAGS="-O2 -fcommon -std=gnu89 -w" \
../gcc-$V/configure --target=$T --prefix=/opt/i960 \
  --with-gnu-as --with-gnu-ld --disable-nls --enable-languages=c \
  > $LOG/gcc-cfg.log 2>&1

# skip libgcc1.cross, which needs a target libc we do not have
HOSTCF="-O2 -g -fcommon -std=gnu89 -w"
make -j"$(nproc)" CFLAGS="$HOSTCF" all-libiberty > $LOG/gcc-make.log 2>&1
make -C gcc -j"$(nproc)" CFLAGS="$HOSTCF" LANGUAGES=c xgcc cc1 cpp0 >> $LOG/gcc-make.log 2>&1

install -d $D
install -m755 gcc/cc1 $D/
install -m755 gcc/cpp0 $D/
[ -f gcc/specs ] && install -m644 gcc/specs $D/specs
install -m755 gcc/xgcc /opt/i960/bin/$T-gcc

# gcc's own headers: stdarg.h selects va-i960.h via __i960__, and varargs
# support is impossible without them (i960 passes the first 12 args in registers)
install -d $D/include
for h in stdarg.h varargs.h stddef.h va-i960.h iso646.h; do
	install -m644 ../gcc-$V/gcc/ginclude/$h $D/include/
done

# libgcc: 64-bit integer helpers, plus fp-bit.c for soft float.  Neither needs
# a target libc with -Dinhibit_libc, and libgcc1.null stands in for the
# machine-specific half.  The i960 config never built fp-bit, so do it by hand.
make -C gcc CFLAGS="$HOSTCF" LANGUAGES=c LIBGCC1=libgcc1.null \
     LIBGCC2_INCLUDES="-Dinhibit_libc" libgcc.a >> $LOG/gcc-make.log 2>&1
S=/tmp/gcc-$V/gcc
INC="-I. -I$S -I$S/config -I$S/../include"
cd gcc
./xgcc -B./ -O2 -Dinhibit_libc $INC -DFLOAT -c $S/config/fp-bit.c -o fp-sf.o
./xgcc -B./ -O2 -Dinhibit_libc $INC          -c $S/config/fp-bit.c -o fp-df.o
cd ..
rm -rf /tmp/ar && mkdir -p /tmp/ar
cd /tmp/ar
i960-intel-nindy-ar x /tmp/g/gcc/libgcc2.a
cp /tmp/g/gcc/fp-sf.o /tmp/g/gcc/fp-df.o .
install -d /opt/i960/netos/lib
i960-intel-nindy-ar rcs /opt/i960/netos/lib/libgcc.a *.o
cd / && rm -rf /tmp/ar
