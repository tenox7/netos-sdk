# double in g0:g1 -> extended real in g0-g2
	.text
	.align	4
	.globl	___extenddfxf2
___extenddfxf2:
	addo	16,sp,sp
	lda	0x40(fp),g2
	call	___netos_dftoxf
	ldt	0x40(fp),g0
	ret
