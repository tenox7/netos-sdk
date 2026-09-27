# libnetos: netOS i960 entry point and raw system call stubs.
#
# A netOS service call is
#     ld slot,r4 ; lda IDX,r5 ; st r5,(r4) ; calls GROUP
# GROUP is the i960 system-procedure number; the sub-function index travels
# through a per-task word whose address comes back from calls 8 with g0=1.
# Group 8 is the bootstrap and takes its selector in g0 directly; g0=0
# returns the address of the task's errno.
# Group 7 is the system call layer, group 10 the netOS C library.
#
# b.out prefixes C symbols with an underscore.

	.text
	.align	4

boot_slotptr:				# calls 8 (g0=1) -> &index slot
	mov	1,g0
	calls	8
	ret

boot_errno:				# calls 8 (g0=0) -> &errno
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
	SYS	_netos_malloc, 10, 0x2a
	SYS	_netos_sscanf, 10, 0x4c
	SYS	_sscanf, 10, 0x4c		# netOS C library, used directly
	SYS	_netos_getenv, 10, 0x20
	SYS	_netos_strlen, 10, 0x54
	SYS	_netos_sprintf, 10, 0x49
	SYS	_netos_strcat, 10, 0x4e
	SYS	_netos_strchr, 10, 0x4f
	SYS	_netos_strcpy, 10, 0x51
	SYS	_netos_strcmp, 10, 0x50
	SYS	_netos_strncmp, 10, 0x57
	SYS	_netos_fopen, 10, 0x17
	SYS	_qsort, 10, 0x3a		# netOS C library, used directly
	SYS	_netos_asctime, 10, 0x8b
	SYS	_netos_ctime, 10, 0x8c
	SYS	_netos_gmtime, 10, 0x8d
	SYS	_netos_strrchr, 10, 0x8f
	SYS	_netos_printf, 10, 0x36
	SYS	_netos_fprintf, 10, 0x18
	SYS	_netos_time, 10, 0x85
	SYS	_netos_localtime, 10, 0x8e

# BSD sockets, used directly
	SYS	_accept, 7, 0x00
	SYS	_bind, 7, 0x01
	SYS	_connect, 7, 0x03
	SYS	_getpeername, 7, 0x07
	SYS	_ioctl, 7, 0x09
	SYS	_listen, 7, 0x0b
	SYS	_recvfrom, 7, 0x10
	SYS	_sendto, 7, 0x12
	SYS	_setsockopt, 7, 0x13
	SYS	_socket, 7, 0x16
	SYS	_socketpair, 7, 0x17
	SYS	_getsockname, 7, 0x2f
	SYS	_shutdown, 10, 0x9b
	SYS	_getservbyname, 10, 0x9a
	SYS	_netos_gethostbyname, 10, 0x7c
	SYS	_netos_gethostbyaddr, 10, 0x7d

# process control, used directly
	SYS	_dup, 7, 0x04
	SYS	_dup2, 7, 0x05
	SYS	_kill, 7, 0x0a
	SYS	_wait, 7, 0x18
	SYS	_pipe, 7, 0x1b
	SYS	_execv, 7, 0x1d
	SYS	_vfork, 7, 0x1e
	SYS	_getpid, 7, 0x26
	SYS	_chdir, 7, 0x22

# netOS enters here with g0=argc g1=argv g2=envp
	.globl	_start
_start:
	movt	g0,r4			# stash argc/argv/envp
	call	boot_slotptr
	st	g0,slot
	call	boot_errno
	st	g0,___netos_errno
	call	boot_rt
	st	g0,rt2
	mov	r5,g0			# argv
	mov	r6,g1			# envp
	call	___netos_setargs
	movt	r4,g0			# restore argc/argv/envp for main
	call	_main
	call	_exit			# flushes stdio, then netos_exit
	ret

	.data
	.align	2
	.globl	___netos_errno
___netos_errno:
	.word	0

	.lcomm	slot,4
	.lcomm	rt2,4
	.lcomm	pad,4
