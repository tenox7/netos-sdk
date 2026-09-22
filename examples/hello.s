# netOS i960 hello world.
#
# netOS publishes its libc/X11 through i960 system calls.  Each call is
#     ld slot,r4 ; lda IDX,r5 ; st r5,(r4) ; calls GROUP
# GROUP is the system-procedure number; the sub-function index travels
# through a per-task word whose address comes back from calls 8 with g0=1.
# Group 8 is the bootstrap and takes its selector in g0 directly.

	.text
	.align	4

msg:
	.asciz	"Hello from netOS\n"
	.align	4

# ---- syscall stubs ---------------------------------------------------
sys_slotptr:				# calls 8 (g0=1) -> &index slot
	mov	1,g0
	calls	8
	ret

sys_boot0:				# calls 8 (g0=0)
	mov	0,g0
	calls	8
	ret

sys_13_12:				# calls 13 / index 12
	ld	slot,r4
	lda	12,r5
	st	r5,(r4)
	calls	13
	ret

printf:					# calls 10 / index 0x36
	ld	slot,r4
	lda	0x36,r5
	st	r5,(r4)
	calls	10
	ret

exit:					# calls 7 / index 6
	ld	slot,r4
	lda	6,r5
	st	r5,(r4)
	calls	7
	ret

# ---- program ---------------------------------------------------------
main:
	lda	msg,g0
	call	printf
	mov	0,g0
	call	exit
	ret

# ---- crt0: netOS enters here with g0=argc g1=argv g2=envp ------------
	.globl	_start
_start:
	movt	g0,r4			# stash argc/argv/envp
	call	sys_slotptr
	st	g0,slot
	call	sys_boot0
	st	g0,rt1
	call	sys_13_12
	st	g0,rt2
	movt	r4,g0			# restore them for main
	call	main
	ret

	.lcomm	slot,4
	.lcomm	rt1,4
	.lcomm	rt2,4
	.lcomm	pad,4
