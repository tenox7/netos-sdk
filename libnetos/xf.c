/* long double <-> double, which newlib's printf and scanf need and libgcc's
   fp-bit lacks.  i960 extended reals are x87's: a 64-bit mantissa with an
   explicit integer bit, then a 15-bit exponent biased by 16383 and the sign.
   They travel in g0-g2, so three words stand in for the long double, which
   gcc 2.95 cannot build in C without an internal error. */
union df { double d; unsigned long w[2]; };

double __truncxfdf2(unsigned long lo, unsigned long hi, unsigned long se)
{
	union df r;
	int e = se & 0x7fff;

	r.w[1] = (se & 0x8000) << 16;
	r.w[0] = 0;
	if (e == 0x7fff) {
		r.w[1] |= 0x7ff00000;
		if ((hi << 1) | lo) r.w[1] |= 0x80000;
		return r.d;
	}
	if (!(hi | lo) || (e -= 16383 - 1023) <= 0) return r.d;
	if (e >= 0x7ff) {
		r.w[1] |= 0x7ff00000;
		return r.d;
	}
	r.w[1] |= (unsigned long) e << 20 | (hi >> 11 & 0xfffff);
	r.w[0] = hi << 21 | lo >> 11;
	if (lo & 0x400 && !++r.w[0]) r.w[1]++;
	return r.d;
}

/* for __extenddfxf2 in xfs.s: w[0..2] get the extended real */
void __netos_dftoxf(double d, unsigned long *w)
{
	union df u;
	unsigned long hi, lo;
	int e;

	u.d = d;
	hi = u.w[1] & 0xfffff;
	lo = u.w[0];
	e = u.w[1] >> 20 & 0x7ff;
	w[2] = (u.w[1] >> 31) << 15;
	if (!e && !(hi | lo)) {
		w[0] = w[1] = 0;
		return;
	}
	if (!e)
		for (e = 1; !(hi & 0x100000); e--) {
			hi = hi << 1 | lo >> 31;
			lo <<= 1;
		}
	w[2] |= e == 0x7ff ? 0x7fff : e + 16383 - 1023;
	w[1] = 0x80000000 | (hi & 0xfffff) << 11 | lo >> 21;
	w[0] = lo << 11;
}
