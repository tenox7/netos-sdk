#!/bin/bash
# Build newlib 1.18.0 as the netOS C library.  libnetos supplies the system
# calls, malloc and the netOS-specific headers; newlib supplies the rest.
set -e
V=1.18.0
T=i960-intel-nindy
P=/opt/i960
H=${1:-/build/libnetos/include}
LOG=${LOG:-/tmp}
cd /tmp
[ -f newlib.tgz ] || wget -q -O newlib.tgz https://sourceware.org/pub/newlib/newlib-$V.tar.gz
rm -rf newlib-$V nl && tar xzf newlib.tgz
N=newlib-$V/newlib
find newlib-$V -name config.guess -exec cp /usr/share/misc/config.guess {} \;
find newlib-$V -name config.sub -exec cp /usr/share/misc/config.sub {} \;

# netOS: malloc is the kernel heap, rename is a system call, ino_t is 32 bits
sed -i '/^# Host specific flag settings/,$ s|^  \*)$|  i960-*-*)\n\tnewlib_cflags="${newlib_cflags} -DMISSING_SYSCALL_NAMES -DMALLOC_PROVIDED -DHAVE_RENAME -DHAVE_FCNTL -DHAVE_BLKSIZE -DHAVE_OPENDIR -D__netos__"\n\tsyscall_dir=\n\t;;\n  *)|' $N/configure.host
sed -i 's|defined(__sparc__) \|\| defined(__SPU__)$|defined(__sparc__) \|\| defined(__SPU__) \|\| defined(__netos__)|' $N/libc/include/sys/types.h
# declare the POSIX calls libnetos provides
sed -i 's|defined(__rtems__)|defined(__rtems__) \|\| defined(__netos__)|g' $N/libc/include/sys/unistd.h
sed -i "s/^#include <sys\/_types.h>$/&\n#include <stdint.h>/" $N/libc/include/sys/unistd.h
sed -i '138s|defined(__rtems__)|defined(__rtems__) \|\| defined(__netos__)|' $N/libc/include/sys/signal.h
sed -i "s/^#if defined(__CYGWIN__) || defined(__rtems__)$/& || defined(__netos__)/" $N/libc/include/machine/setjmp.h
sed -i '147s|defined(__rtems__)|defined(__rtems__) \|\| defined(__netos__)|' $N/libc/include/sys/stat.h
sed -i "s/ _ATTRIBUTE ((warning ([^)]*)))//" $N/libc/include/stdlib.h
cp $H/sys/dirent.h $H/sys/termios.h $N/libc/include/sys/
# glibc's getopt starts at optind 1, and busybox reads optind without getopt
sed -i "s/^int optind = 0;$/int optind = 1;/; s/^static int optwhere = 0;$/static int optwhere = 1;/" $N/libc/stdlib/getopt.c

mkdir nl && cd nl
../newlib-$V/configure --target=$T --prefix=$P --disable-newlib-supplied-syscalls \
  --disable-multilib --disable-nls CFLAGS_FOR_TARGET="-O2 -fno-common" > $LOG/newlib-cfg.log 2>&1
make -j"$(nproc)" all-target-newlib > $LOG/newlib-make.log 2>&1
make install-target-newlib >> $LOG/newlib-make.log 2>&1

# posix pieces newlib only builds for hosted targets, plus the i960 setjmp
L=$P/$T/lib/libc.a
C="$T-gcc -O2 -fno-common -fno-builtin -D_GNU_SOURCE -DMISSING_SYSCALL_NAMES -D__netos__ -I $H -I $P/$T/include"
S=/tmp/newlib-$V/newlib/libc
mkdir posix && cd posix
for f in collate collcmp fnmatch glob regcomp regerror regexec regfree \
         execl execle execlp execv execvp popen; do
	$C -I $S/posix -c $S/posix/$f.c -o $f.o
done
for f in basename dirname pread pwrite; do $C -c $S/unix/$f.c -o $f.o; done
$C -c $S/machine/i960/setjmp.S -o setjmp.o
$T-ar rs $L *.o
