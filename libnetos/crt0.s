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
# SYS stubs return what the kernel returns.  SYSE stubs are POSIX calls: on
# -1 they translate the kernel's errno into newlib's.
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

boot_rt:				# calls 13 / index 12, as the shipped apps do
	ld	slot,r4
	lda	12,r5
	st	r5,(r4)
	calls	13
	ret

	.globl	___main
___main:				# gcc emits this for main(); libgcc would supply it
	ret

	.macro	SYS name, grp, idx
	.globl	\name
\name:
	ld	slot,r4
	lda	\idx,r5
	st	r5,(r4)
	calls	\grp
	ret
	.endm

	.macro	SYSE name, grp, idx
	.globl	\name
\name:
	ld	slot,r4
	lda	\idx,r5
	st	r5,(r4)
	calls	\grp
	addo	1,g0,r4
	cmpobne	0,r4,1f
	call	___netos_seterr
1:	ret
	.endm

# group 7, wrapped in C
	SYSE	_netos_open, 7, 0x0d
	SYSE	_netos_close, 7, 0x02
	SYSE	_netos_read, 7, 0x0e
	SYSE	_netos_write, 7, 0x19
	SYSE	_netos_stat, 7, 0x23
	SYSE	_netos_lstat, 7, 0x24
	SYSE	_netos_fstat, 7, 0x25
	SYSE	_netos_fcntl, 7, 0x21
	SYSE	_netos_ioctl, 7, 0x09
	SYSE	_netos_dup2, 7, 0x05
	SYSE	_netos_execve, 7, 0x1d
	SYSE	_netos_wait, 7, 0x18
	SYS	_netos_exit, 7, 0x06
	SYS	_netos_vexit, 7, 0x1c
	SYS	_netos_getsockname, 7, 0x2f

# group 7, POSIX as is
	SYSE	_accept, 7, 0x00
	SYSE	_bind, 7, 0x01
	SYSE	_connect, 7, 0x03
	SYSE	_dup, 7, 0x04
	SYSE	_getpeername, 7, 0x07
	SYSE	_kill, 7, 0x0a
	SYSE	_listen, 7, 0x0b
	SYSE	_readv, 7, 0x0f
	SYSE	_recvfrom, 7, 0x10
	SYSE	_sendto, 7, 0x12
	SYSE	_setsockopt, 7, 0x13
	SYS	_sigblock, 7, 0x14
	SYS	_sigsetmask, 7, 0x15
	SYSE	_socket, 7, 0x16
	SYSE	_socketpair, 7, 0x17
	SYSE	_writev, 7, 0x1a
	SYSE	_pipe, 7, 0x1b
	SYSE	_chdir, 7, 0x22
	SYS	_getpid, 7, 0x26
	SYSE	_unlink, 7, 0x28
	SYSE	_lseek, 7, 0x29
	SYSE	_gettimeofday, 7, 0x2a
	SYSE	_select, 7, 0x2c
	SYSE	_getsockname, 7, 0x2f
	SYSE	_chmod, 7, 0x30
	SYSE	_fchmod, 7, 0x31
	SYSE	_ftruncate, 7, 0x32
	SYSE	_netos_mkdir, 7, 0x34
	SYSE	_netos_utimes, 7, 0x37
	SYSE	_getsockopt, 7, 0x38
	SYSE	_chown, 7, 0x3c
	SYSE	_link, 7, 0x41
	SYSE	_mkfifo, 7, 0x42
	SYSE	_mknod, 7, 0x43
	SYSE	__rename, 7, 0x46
	SYSE	_rmdir, 7, 0x47
	SYSE	_symlink, 7, 0x4a
	SYS	_umask, 7, 0x4d

# vfork: the child runs in our memory until it execs or exits, so note which
# side of it we are on; malloc and _exit differ in the child.  The parent
# gets back the flag it had, so a vforked child may vfork again.
	.globl	_vfork
_vfork:
	ld	___netos_vforked,r6
	ld	slot,r4
	lda	0x1e,r5
	st	r5,(r4)
	calls	7
	mov	r6,r4
	cmpobne	0,g0,1f
	mov	1,r4
1:	st	r4,___netos_vforked
	addo	1,g0,r4
	cmpobne	0,r4,2f
	call	___netos_seterr
2:	ret

# group 10, the kernel's C library
	SYS	_netos_malloc, 10, 0x2a
	SYS	_netos_syslog, 10, 0x5e
	SYS	_netos_free, 10, 0x1c
	SYS	_netos_opendir, 10, 0x66
	SYS	_netos_readdir, 10, 0x67
	SYS	_netos_closedir, 10, 0x6b
	SYS	_netos_gethostbyname, 10, 0x7c
	SYS	_netos_gethostbyaddr, 10, 0x7d
	SYS	_getservbyname, 10, 0x9a
	SYSE	_shutdown, 10, 0x9b
	SYS	_netos_bcopy, 10, 0x07
	SYS	_netos_fopen, 10, 0x17
	SYS	_netos_fprintf, 10, 0x18
	SYS	_netos_getenv, 10, 0x20
	SYS	_netos_printf, 10, 0x36
	SYS	_netos_qsort, 10, 0x3a
	SYS	_netos_sprintf, 10, 0x49
	SYS	_netos_sscanf, 10, 0x4c
	SYS	_netos_strcat, 10, 0x4e
	SYS	_netos_strchr, 10, 0x4f
	SYS	_netos_strcmp, 10, 0x50
	SYS	_netos_strcpy, 10, 0x51
	SYS	_netos_strlen, 10, 0x54
	SYS	_netos_strncmp, 10, 0x57
	SYS	_netos_fputc, 10, 0x76
	SYS	_netos_time, 10, 0x85
	SYS	_netos_asctime, 10, 0x8b
	SYS	_netos_ctime, 10, 0x8c
	SYS	_netos_gmtime, 10, 0x8d
	SYS	_netos_localtime, 10, 0x8e
	SYS	_netos_strrchr, 10, 0x8f

# netOS enters here with g0=argc g1=argv g2=envp
	.globl	_start
_start:
	movt	g0,r4			# stash argc/argv/envp
	call	boot_slotptr
	st	g0,slot
	call	boot_errno
	st	g0,___netos_errno
	call	boot_rt
	st	r5,___netos_argv
	st	r6,_environ
	call	___netos_init
	movt	r4,g0			# restore argc/argv/envp for main
	call	_main
	call	_exit			# atexit, stdio flush, then _exit
	ret

	.data
	.align	2
	.globl	___netos_errno
___netos_errno:
	.word	0
	.globl	_environ
_environ:
	.word	0
	.globl	___netos_argv
___netos_argv:
	.word	0
	.globl	___netos_vforked
___netos_vforked:
	.word	0

	.lcomm	slot,4
