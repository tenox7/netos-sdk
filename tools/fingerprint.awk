# Fingerprint netOS syscall stubs from an i960 disassembly.
#
# Pass 1 finds the stubs:   ld slot,r4 ; lda IDX,r5 ; st r5,(r4) ; calls GRP
# Pass 2 walks every call to one and records how it was set up: how many
# argument registers were loaded, whether the result is used, and whether any
# argument points at a string (a format string names the printf family at once).
#
# Addresses map to file offsets by +44 in a b.out image (text at VMA 0), so the
# string table built from `strings -t d` can be looked up directly.

function h2d(s,   i, c, v, d) {
	sub(/^0[xX]/, "", s)
	if (s == "") return -1
	v = 0
	for (i = 1; i <= length(s); i++) {
		c = tolower(substr(s, i, 1))
		d = index("0123456789abcdef", c) - 1
		if (d < 0) return -1
		v = v * 16 + d
	}
	return v
}

function num(s) { return (s ~ /^0[xX]/) ? h2d(s) : s + 0 }

# strings file: "<decimal offset> <text>"
FNR == NR && FILENAME == STR {
	off = $1 + 0
	t = $0
	sub(/^[ \t]*[0-9]+[ \t]+/, "", t)
	str[off - 44] = t
	next
}

{ line[++nl] = $0 }

END {
	# ---- pass 1: stubs -------------------------------------------------
	for (i = 1; i <= nl; i++) {
		l = line[i]
		if (l !~ /:\t/) continue
		split(l, a, ":"); addr = a[1]; gsub(/^[ \t]+/, "", addr)
		body = l; sub(/^[^\t]*\t[^\t]*\t/, "", body)

		if (body ~ /^ld[ \t]+0x[0-9a-f]+,r4/) { start = addr; idx = -1; continue }
		if (body ~ /^(lda|mov)[ \t]+[^,]+,r5$/) {
			v = body; sub(/^[a-z]+[ \t]+/, "", v); sub(/,r5$/, "", v)
			idx = num(v); continue
		}
		if (body ~ /^addo[ \t]+[^,]+,[^,]+,r5$/) {
			v = body; sub(/^addo[ \t]+/, "", v); sub(/,r5$/, "", v)
			split(v, b, ","); idx = num(b[1]) + num(b[2]); continue
		}
		if (body ~ /^shlo[ \t]+[^,]+,[^,]+,r5$/) {
			v = body; sub(/^shlo[ \t]+/, "", v); sub(/,r5$/, "", v)
			split(v, b, ","); idx = num(b[2]) * (2 ^ num(b[1])); continue
		}
		if (body ~ /^calls[ \t]+[0-9]+$/ && idx >= 0) {
			g = body; sub(/^calls[ \t]+/, "", g)
			stub[start] = g "/" idx
			idx = -1
		}
	}

	# ---- pass 2: call sites --------------------------------------------
	for (i = 1; i <= nl; i++) {
		l = line[i]
		if (l !~ /:\t/) continue
		body = l; sub(/^[^\t]*\t[^\t]*\t/, "", body)

		if (body ~ /^(call|callx)[ \t]+0x[0-9a-f]+$/) {
			tgt = body; sub(/^[a-z]+[ \t]+0x/, "", tgt)
			if (tgt in stub) emit(i, stub[tgt])
			barrier = i
			continue
		}
		if (body ~ /^(ret|bx|b)[ \t]*/) barrier = i
	}

}

# look back from a call to see which argument registers were set
function emit(at, key,   j, body, dst, seen, n, s, hit, r, ad, ru, nc) {
	delete seen
	for (j = at - 1; j > 0 && j > at - 14; j--) {
		body = line[j]
		if (body !~ /:\t/) continue
		sub(/^[^\t]*\t[^\t]*\t/, "", body)
		if (body ~ /^(call|callx|ret|bx)/) break
		if (body !~ /,(g[0-9]+)$/) continue
		dst = body; sub(/^.*,/, "", dst)
		if (dst !~ /^g[0-9]+$/) continue
		r = substr(dst, 2) + 0
		if (r > 11) continue
		seen[r] = 1
		# an lda of a literal address that resolves to a string is a good clue
		if (body ~ /^lda[ \t]+0x[0-9a-f]+,/) {
			s = body; sub(/^lda[ \t]+0x/, "", s); sub(/,.*$/, "", s)
			ad = h2d(s)
			if (ad in str && !hit) hit = str[ad]
		}
	}
	n = 0
	for (j = 0; j <= 11; j++) if (j in seen) n = j + 1

	# is the result consumed right after?
	ru = 0
	body = line[at + 1]
	if (body ~ /:\t/) {
		sub(/^[^\t]*\t[^\t]*\t/, "", body)
		if (body ~ /g0/) ru = 1
	}
	# malloc-shaped: result immediately null-checked
	nc = 0
	for (j = at + 1; j <= at + 2; j++) {
		body = line[j]
		if (body !~ /:\t/) continue
		sub(/^[^\t]*\t[^\t]*\t/, "", body)
		if (body ~ /cmp.*[ \t]0,g0/) nc = 1
	}
	gsub(/\t/, " ", hit)		# tabs in a string would shift the output fields
	printf "%s\t%d\t%d\t%s\t%d\n", key, n, ru, hit, nc
}
