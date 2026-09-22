#!/bin/sh
# List the netOS syscall stubs in an i960 binary: address -> group/index.
# Stub shape:  ld slot,r4 ; lda IDX,r5 ; st r5,(r4) ; calls GROUP
i960-intel-nindy-objdump -d "$1" 2>/dev/null | awk '
  /:\t/ {
    line=$0; sub(/^[ \t]*/,"",line); split(line,a,":"); addr=a[1]
    if (line ~ /ld[ \t]+0x/)                       { start=addr; e="" }
    else if (line ~ /(lda|mov)[ \t]+[^,]+,r5/)     { n=line; sub(/.*[ \t]/,"",n); sub(/,r5/,"",n); e=n }
    else if (line ~ /addo[ \t]+[^,]+,[^,]+,r5/)    { n=line; sub(/.*addo[ \t]+/,"",n); sub(/,r5/,"",n); split(n,b,","); e=b[1]"+"b[2] }
    else if (line ~ /shlo[ \t]+[^,]+,[^,]+,r5/)    { n=line; sub(/.*shlo[ \t]+/,"",n); sub(/,r5/,"",n); split(n,b,","); e=b[2]"<<"b[1] }
    else if (line ~ /calls[ \t]+[0-9]+/ && e!="")  { g=line; sub(/.*calls[ \t]+/,"",g); print addr, start, g, e; e="" }
  }' | while read here start grp expr; do
    printf "%s %s/0x%x\n" "$start" "$grp" "$(( $expr ))"
  done
