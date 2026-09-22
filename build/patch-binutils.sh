#!/bin/bash
# Three gcc-12 incompatibilities in 2001-era binutils sources.
set -e
S=$1

# 1. cpu-i960.c lowercases its own const argument in place
sed -i \
 -e 's|^  int fail_because_not_80960 = false;$|  int fail_because_not_80960 = false;\n  char lower[64];|' \
 -e 's|^  for (i = 0; i < strlen (string); i ++)$|  for (i = 0; i + 1 < (int) sizeof (lower) \&\& string[i]; i ++)|' \
 -e "s|^    string\[i\] = tolower (string\[i\]);$|    lower[i] = tolower ((unsigned char) string[i]);\n  lower[i] = 0;\n  string = lower;|" \
 $S/bfd/cpu-i960.c

# 2/3. tc-i960.h declares an array of the still-incomplete struct relax_type;
#      move the declaration to write.c, its only user, where tc.h is in scope.
sed -i 's|^extern const struct relax_type md_relax_table\[\];$||' $S/gas/config/tc-i960.h
sed -i '0,/^#ifdef TC_GENERIC_RELAX_TABLE$/s||#ifdef TC_GENERIC_RELAX_TABLE\nextern const relax_typeS md_relax_table[];|' $S/gas/write.c
