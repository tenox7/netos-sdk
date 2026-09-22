#!/bin/sh
# Build libnetos against host syscalls and run a program through it.
# Run from the libnetos directory:  sh test/run.sh [prog.c]
set -e
SRC=${1:-../examples/demo.c}
R="-std=gnu89 -w -fcommon -O1 -I include -include test/rename.h"
mkdir -p /tmp/lt && rm -f /tmp/lt/*.o
for f in string stdlib stdio printf unistd; do gcc $R -c -o /tmp/lt/$f.o $f.c; done
gcc $R -c -o /tmp/lt/prog.o "$SRC"
gcc -std=gnu89 -w -O1 -c -o /tmp/lt/stubs.o test/stubs.c
gcc -o /tmp/lt/run /tmp/lt/*.o
/tmp/lt/run one two three
