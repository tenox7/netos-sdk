# libnetos: netOS i960 entry point and raw system call stubs.
#
# A netOS service call is
#     ld slot,r4 ; lda IDX,r5 ; st r5,(r4) ; calls GROUP
# GROUP is the i960 system-procedure number; the sub-function index travels
# through a per-task word whose address comes back from calls 8 with g0=1.
# Group 8 is the bootstrap and takes its selector in g0 directly.
# Group 7 is the system call layer, group 10 the netOS C library.
#
# b.out prefixes C symbols with an underscore.

	.text
	.align	4

boot_slotptr:				# calls 8 (g0=1) -> &index slot
	mov	1,g0
	calls	8
	ret

boot_zero:				# calls 8 (g0=0)
	mov	0,g0
	calls	8
	ret

boot_rt:				# calls 13 / index 12
	ld	slot,r4
	lda	12,r5
	st	r5,(r4)
	calls	13
	ret

	.globl	___main
___main:				# gcc emits this for main(); libgcc would supply it
	ret

# syscall(group, index) stubs -----------------------------------------

	.macro	SYS name, grp, idx
	.globl	\name
\name:
	ld	slot,r4
	lda	\idx,r5
	st	r5,(r4)
	calls	\grp
	ret
	.endm

	SYS	_netos_open, 7, 0x0d
	SYS	_netos_close, 7, 0x02
	SYS	_netos_read, 7, 0x0e
	SYS	_netos_write, 7, 0x19
	SYS	_netos_lseek, 7, 0x29
	SYS	_netos_stat, 7, 0x23
	SYS	_netos_lstat, 7, 0x24
	SYS	_netos_readlink, 7, 0x45
	SYS	_netos_rmdir, 7, 0x47
	SYS	_netos_select, 7, 0x2c
	SYS	_netos_exit, 7, 0x06

	SYS	_netos_bcopy, 10, 0x07
	SYS	_netos_free, 10, 0x1c
	SYS	_netos_malloc, 10, 0x67
	SYS	_netos_sscanf, 10, 0x4c
	SYS	_sscanf, 10, 0x4c		# netOS C library, used directly
	SYS	_netos_getenv, 10, 0x20
	SYS	_netos_strlen, 10, 0x2a
	SYS	_netos_sprintf, 10, 0x49
	SYS	_netos_strcpy, 10, 0x4e
	SYS	_netos_strcmp, 10, 0x50
	SYS	_netos_strncmp, 10, 0x57
	SYS	_netos_fopen, 10, 0x17
	SYS	_qsort, 10, 0x3a		# netOS C library, used directly
	SYS	_netos_asctime, 10, 0x8b
	SYS	_netos_ctime, 10, 0x8c
	SYS	_netos_gmtime, 10, 0x8d
	SYS	_netos_strchr, 10, 0x8f
	SYS	_netos_printf, 10, 0x36
	SYS	_netos_fprintf, 10, 0x18
	SYS	_netos_time, 10, 0x85
	SYS	_netos_localtime, 10, 0x8e

# netOS enters here with g0=argc g1=argv g2=envp
	.globl	_start
_start:
	movt	g0,r4			# stash argc/argv/envp
	call	boot_slotptr
	st	g0,slot
	call	boot_zero
	st	g0,rt1
	call	boot_rt
	st	g0,rt2
	mov	r5,g0			# argv
	mov	r6,g1			# envp
	call	___netos_setargs
	movt	r4,g0			# restore argc/argv/envp for main
	call	_main
	call	_exit			# flushes stdio, then netos_exit
	ret

	.lcomm	slot,4
	.lcomm	rt1,4
	.lcomm	rt2,4
	.lcomm	pad,4
