#!/bin/bash
# ncurses 5.9 for netOS: ncurses, panel, menu and form.  netOS has no
# terminfo database, so the common terminals are compiled in.
set -e
V=5.9
P=/opt/i960
H=/tmp/hnc
LOG=${LOG:-/tmp}
cd /tmp
[ -f ncurses.tgz ] || wget -q -O ncurses.tgz https://ftp.gnu.org/gnu/ncurses/ncurses-$V.tar.gz
rm -rf ncurses-$V && tar xzf ncurses.tgz && cd ncurses-$V
cp /usr/share/misc/config.guess /usr/share/misc/config.sub .

# the same version's tic and infocmp compile the built-in terminals
mkdir host && cd host
CPPFLAGS=-P ../configure --prefix=$H --without-cxx --without-cxx-binding --without-ada \
	--without-debug --without-shared --without-manpages --without-tests > $LOG/nc-host.log 2>&1
make -j"$(nproc)" >> $LOG/nc-host.log 2>&1
make install >> $LOG/nc-host.log 2>&1

mkdir ../cross && cd ../cross
PATH=$H/bin:$PATH ../configure --host=i960-intel-nindy --build=$(../config.guess) --prefix=$P/netos \
	CC=netos-gcc AR=i960-intel-nindy-ar RANLIB=i960-intel-nindy-ranlib \
	--with-build-cc=gcc --with-build-cppflags=-P \
	--without-cxx --without-cxx-binding --without-ada --without-progs --without-tests \
	--without-manpages --without-debug --without-shared --without-profile --without-gpm \
	--disable-database --disable-home-terminfo --disable-db-install \
	--with-fallbacks=vt220,vt100,vt102,xterm,xterm-color,xterm-256color,screen,linux,ansi,dumb \
	> $LOG/nc-cfg.log 2>&1
PATH=$H/bin:$PATH make -j"$(nproc)" libs > $LOG/nc-make.log 2>&1
make install.libs install.includes >> $LOG/nc-make.log 2>&1
cp $P/netos/lib/libncurses.a $P/netos/lib/libcurses.a
