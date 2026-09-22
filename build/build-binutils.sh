#!/bin/bash
# Build binutils 2.11.2 (last release whose gas emits i960 b.out) for netOS.
set -e
V=2.11.2
HERE=$(cd "$(dirname "$0")" && pwd)
LOG=${LOG:-/tmp}
cd /tmp
[ -f b.tgz ] || wget -q -O b.tgz https://ftp.gnu.org/gnu/binutils/binutils-$V.tar.gz
rm -rf binutils-$V b && tar xzf b.tgz

# 2001-era config.guess predates aarch64 build hosts
find binutils-$V -name config.guess -exec cp /usr/share/misc/config.guess {} \;
find binutils-$V -name config.sub   -exec cp /usr/share/misc/config.sub   {} \;
bash "$HERE/patch-binutils.sh" binutils-$V

mkdir b && cd b
CFLAGS="-O2 -fcommon -std=gnu89 -w" \
../binutils-$V/configure --target=i960-intel-nindy --prefix=/opt/i960 \
  --disable-nls --disable-shared > $LOG/cfg.log 2>&1

# old gettext po/ rules break on modern make+sed; nls is disabled anyway
for d in bfd opcodes binutils gas ld; do
  [ -f $d/Makefile ] && sed -i 's/^SUBDIRS[ \t]*=.*/SUBDIRS =/' $d/Makefile
done

make MAKEINFO=true -j"$(nproc)" all-gas all-ld all-binutils > $LOG/make.log 2>&1
make MAKEINFO=true install-gas install-ld install-binutils >> $LOG/make.log 2>&1
