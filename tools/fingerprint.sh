#!/bin/sh
# Fingerprint every netOS syscall used by the shipped i960 binaries.
#   fingerprint.sh <dir of i960 binaries>
D=${1:-/n/netos32/netOS/bin/i960}
for f in "$D"/*; do
	[ -f "$f" ] || continue
	strings -t d -n 3 "$f" > /tmp/s.txt 2>/dev/null
	i960-intel-nindy-objdump -d "$f" 2>/dev/null > /tmp/d.txt
	awk -v STR=/tmp/s.txt -f "$(dirname "$0")/fingerprint.awk" /tmp/s.txt /tmp/d.txt
done
