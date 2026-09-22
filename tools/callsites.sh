#!/bin/sh
# callsites.sh <binary> <group/index> [count] - show how a syscall stub is called
F=$1; K=$2; N=${3:-2}
A=$(sh "$(dirname "$0")/stubmap.sh" "$F" | awk -v k="$K" '$2==k{print $1}' | head -1)
[ -n "$A" ] || { echo "  ($K not used by $(basename "$F"))"; exit 0; }
i960-intel-nindy-objdump -d "$F" 2>/dev/null > /tmp/cs.txt
echo "--- $(basename "$F") $K stub=0x$A ---"
for n in $(grep -n "call	0x$A" /tmp/cs.txt | cut -d: -f1 | head -$N); do
	awk -v s=$((n-5)) -v e=$((n+1)) 'NR>=s && NR<=e' /tmp/cs.txt; echo "   ."
done
