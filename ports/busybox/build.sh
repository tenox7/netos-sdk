#!/bin/sh
# busybox 1.00 for netOS, with the applets and features listed in ./applets.
#   ./nsdk sh ports/busybox/build.sh      builds ports/busybox/busybox
set -e
V=1.00
D=$(cd "$(dirname "$0")" && pwd)
cd /tmp
[ -f busybox-$V.tar.bz2 ] || wget -q https://busybox.net/downloads/busybox-$V.tar.bz2
rm -rf busybox-$V && tar xjf busybox-$V.tar.bz2 && cd busybox-$V
patch -s -p1 < $D/netos.patch
make allnoconfig >/dev/null
sed -i 's/^\(CONFIG_FEATURE_BUFFERS_GO_ON_STACK\|CONFIG_FEATURE_SH_IS_NONE\)=y$/# \1 is not set/' .config
# an option shows up in .config only once what it depends on is on
for pass in 1 2 3; do
	for c in $(cat $D/applets); do
		sed -i "s/^# CONFIG_$c is not set$/CONFIG_$c=y/" .config
	done
	yes "" | make oldconfig >/dev/null
done
make CC=netos-gcc AR=i960-intel-nindy-ar STRIP=true
cp busybox $D/busybox

# apps.cfg lines for the applets, except names netOS already uses
sed 's|.*/||' busybox.links | grep -vxF -f $D/skip | while read a; do
	printf 'app.%s.path:\t///%%netOSdir%%/netOS/bin/%%cpu%%/busybox\n' $a
done > $D/apps.cfg
