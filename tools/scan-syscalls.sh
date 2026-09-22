#!/bin/bash
# Extract netOS syscall numbers from PPC binaries: lis r0,HI / ori r0,r0,LO / sc
for f in "$@"; do
  objdump -d -m powerpc:common --target=elf32-powerpc "$f" 2>/dev/null |
  sed 's/[[:space:]]*$//' |
  awk -v name="$(basename "$f")" '
    /lis[[:space:]]+r0,/    { n=$0; sub(/.*r0,/,"",n); hi=n+0; ok=1; next }
    /ori[[:space:]]+r0,r0,/ { n=$0; sub(/.*r0,r0,/,"",n); lo=n+0; next }
    /nop$/                  { if (ok) lo=0; next }
    /[[:space:]]sc$/        { if (ok) printf "%08x %s\n", hi*65536+lo, name; ok=0; lo=0 }
  '
done
