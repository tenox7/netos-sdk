# Merge fingerprint records into one profile per group/index.
function hex(n,   d, s) {
	if (n == 0) return "0x00"
	s = ""
	while (n > 0) { d = n % 16; s = substr("0123456789abcdef", d + 1, 1) s; n = int(n / 16) }
	return "0x" (length(s) < 2 ? "0" s : s)
}
{
	split($1, k, "/"); grp = k[1]; idx = k[2]
	key = grp "/" hex(idx)
	n = $2; ru = $3; nc = $5
	s = $0; sub(/^[^\t]*\t[^\t]*\t[^\t]*\t/, "", s); sub(/\t[0-9]*$/, "", s)
	sites[key]++; argc[key, n]++; ret[key] += ru; null[key] += nc
	if (s != "" && nstr[key] < 4 && index(seen[key], "{" s "}") == 0) {
		seen[key] = seen[key] "{" s "}"
		ex[key] = ex[key] (nstr[key] ? " | " : "") substr(s, 1, 22)
		nstr[key]++
	}
}
END {
	for (kk in sites) {
		best = 0; bn = -1; dist = ""
		for (j = 0; j <= 11; j++) if ((kk, j) in argc) {
			if (argc[kk, j] > bn) { bn = argc[kk, j]; best = j }
		}
		printf "%-10s n=%-5d args=%d ret=%3d%% null=%3d%%  %s\n",
		       kk, sites[kk], best, int(100*ret[kk]/sites[kk]),
		       int(100*null[kk]/sites[kk]), ex[kk]
	}
}
