#!/bin/sh
# vi: the traditional ex/vi 3.7 (Gunnar Ritter's ex-vi 050325) for netOS.
#   ./nsdk sh ports/vi/build.sh      builds ports/vi/vi
set -e
V=050325
D=$(cd "$(dirname "$0")" && pwd)
cd /tmp
[ -f ex-$V.tar.bz2 ] || wget -q https://downloads.sourceforge.net/project/ex-vi/ex-vi/$V/ex-$V.tar.bz2
rm -rf ex-$V && tar xjf ex-$V.tar.bz2 && cd ex-$V
patch -s -p1 < $D/netos.patch
touch ex_vars.h
make CC=netos-gcc FEATURES="-DLISPCODE -DCHDIR -DFASTTAG -DUCVISUAL -DBIT8" \
	REINC= RELIB= RETGT= OSTYPE="-DVMUNIX -DVFORK" MALLOC=netos.o STRIP= \
	TLIB= TERMLIB=termcap ex
cp ex $D/vi
