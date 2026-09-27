#!/bin/sh
# Build libtermcap.a into the toolchain prefix.
set -e
P=${1:-/opt/i960}
for f in termcap tgoto tputs; do
	i960-intel-nindy-gcc -O2 -fno-common -D__netos__ -DCM_N -DCM_GT -DCM_B -DCM_D \
		-I $P/netos/include -c -o $f.o $f.c
done
i960-intel-nindy-ar rcs libtermcap.a termcap.o tgoto.o tputs.o
cp libtermcap.a $P/netos/lib/
cp termcap.h $P/netos/include/
rm -f *.o *.a
