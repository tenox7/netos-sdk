#!/bin/sh
# Build libnetos.a and crt0.o into the toolchain prefix.
set -e
P=${1:-/opt/i960}
CC="i960-intel-nindy-gcc -O2 -fno-common -I include"
AS=i960-intel-nindy-as

$AS -o crt0.o crt0.s
for f in string stdlib stdio printf unistd socket errno process; do
	$CC -S -o $f.s $f.c
	$AS -o $f.o $f.s
done
i960-intel-nindy-ar rcs libnetos.a string.o stdlib.o stdio.o printf.o unistd.o socket.o errno.o process.o

mkdir -p $P/netos/lib $P/netos/include
cp crt0.o libnetos.a $P/netos/lib/
cp -r include/* $P/netos/include/
rm -f *.o *.a
[ "$KEEP_ASM" = 1 ] || rm -f string.s stdlib.s stdio.s printf.s unistd.s socket.s errno.s process.s
