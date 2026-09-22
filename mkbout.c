/* mkbout - turn a gld960 b.out link into a netOS-loadable image.
 *
 * netOS apps are b.out files that keep their relocations (the loader places
 * them anywhere) but carry no symbol table and start at e_entry.  ld gives us
 * the first two; this sets the entry from a named symbol and drops the rest.
 *
 *   mkbout <in.bout> <out> [entry-symbol]      default symbol: _start
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HDR 44
#define NLIST 12

static unsigned char *f;
static long flen;

static unsigned long g32(int o) {
	return f[o] | f[o+1]<<8 | (unsigned long)f[o+2]<<16 | (unsigned long)f[o+3]<<24;
}
static void p32(int o, unsigned long v) {
	f[o] = v; f[o+1] = v>>8; f[o+2] = v>>16; f[o+3] = v>>24;
}

int main(int argc, char **argv)
{
	const char *want = argc > 3 ? argv[3] : "_start";
	unsigned long text, data, bss, syms, trsize, drsize, symoff, stroff, entry = ~0UL;
	unsigned long i, fixed = 0;
	FILE *fp;

	if (argc < 3) { fprintf(stderr, "usage: mkbout in out [entry-symbol]\n"); return 1; }

	fp = fopen(argv[1], "rb");
	if (!fp) { perror(argv[1]); return 1; }
	fseek(fp, 0, SEEK_END); flen = ftell(fp); rewind(fp);
	f = malloc(flen);
	if (fread(f, 1, flen, fp) != (size_t)flen) { perror("read"); return 1; }
	fclose(fp);

	if (g32(0) != 0x10d) { fprintf(stderr, "not a b.out file (magic %#lx)\n", g32(0)); return 1; }

	text = g32(4); data = g32(8); bss = g32(12); syms = g32(16);
	trsize = g32(24); drsize = g32(28);
	symoff = HDR + text + data + trsize + drsize;
	stroff = symoff + syms;

	for (i = 0; i + NLIST <= syms; i += NLIST) {
		unsigned long strx = g32(symoff + i);
		const char *nm = (const char *)f + stroff + strx;
		if (strx && stroff + strx < (unsigned long)flen && !strcmp(nm, want)) {
			entry = g32(symoff + i + 8);
			break;
		}
	}
	if (entry == ~0UL) { fprintf(stderr, "entry symbol '%s' not found\n", want); return 1; }

	p32(20, entry);	/* e_entry */
	p32(16, 0);	/* e_syms  */
	f[43] = 4;	/* e_relaxable: every shipped netOS image carries 4 */

	/* netOS's loader implements only abs32 (0x04) and pcrel24 (0x05).
	   gcc's calls assemble to callx with the relaxable bsr/callj bits set,
	   which ld -relax would normally clear; b.out relaxation is broken in
	   binutils 2.11, so clear them here.  The callx operand word already
	   holds the absolute target, making abs32 the right fixup. */
	{
		unsigned long r, relocs = HDR + text + data;
		for (r = 0; r + 8 <= trsize + drsize; r += 8) {
			unsigned char *fl = f + relocs + r + 7;
			if (*fl & 0x08) {
				fprintf(stderr, "reloc at +%lu is extern; link with -fno-common\n", r);
				return 1;
			}
			if (*fl & 0x50) { *fl &= ~0x50; fixed++; }
		}
	}

	fp = fopen(argv[2], "wb");
	if (!fp) { perror(argv[2]); return 1; }
	fwrite(f, 1, symoff, fp);
	fclose(fp);

	printf("%s: text=%lu data=%lu bss=%lu trsize=%lu drsize=%lu entry=0x%lx (%lu relocs unrelaxed) -> %s (%lu bytes)\n",
	       argv[1], text, data, bss, trsize, drsize, entry, fixed, argv[2], symoff);
	return 0;
}
