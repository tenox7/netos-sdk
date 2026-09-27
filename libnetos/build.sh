#!/bin/sh
# Build crt0.o and libnetos.a, newlib's system layer, into the toolchain prefix.
set -e
P=${1:-/opt/i960}
O="init err warn file cwd misc tty dirent malloc exec proc time pwd poll socket syslog xf"
i960-intel-nindy-as -o crt0.o crt0.s
i960-intel-nindy-as -o xfs.o xfs.s
for f in $O; do i960-intel-nindy-gcc -O2 -fno-common -D__netos__ -I include -c -o $f.o $f.c; done
i960-intel-nindy-ar rcs libnetos.a xfs.o $(for f in $O; do echo $f.o; done)
mkdir -p $P/netos/lib $P/netos/include
cp crt0.o libnetos.a $P/netos/lib/
cp -r include/* $P/netos/include/
rm -f *.o *.a
