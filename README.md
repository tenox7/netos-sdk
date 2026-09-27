# netOS SDK

A C cross-development kit for **Neoware netOS** on the Intel i960 — the thin
client OS shipped by Human Designed Systems, later HDS Network Systems, later
Neoware, on the @workStation / ViewStation / Neoware Station network computers.

netOS never had a public SDK. This one was reconstructed by reverse engineering
the shipped binaries: the executable format, the system call convention, and
enough of the C library inside the kernel to be useful. Ordinary C compiles and
runs on real hardware.

    ./nsdk netos-gcc hello.c -o hello

## Quick start

Requires Docker. The image builds a 1998-era cross toolchain from source, which
takes about fifteen minutes the first time.

    make image
    ./nsdk netos-gcc examples/demo.c -o demo

Copy the result into your netOS tree as `netOS/bin/i960/demo`, add a line to
`netOS/config/apps.cfg`:

    app.demo.path:	///%netOSdir%/netOS/bin/%cpu%/demo

then `rehash` and `demo` at the Station's console. `prebuilt/` has images you
can try without building the toolchain.

## What works

Ordinary C, including varargs, floating point and `long long`. `examples/`
builds unmodified upstream source — `aclock-vt100.c` is
[aclock](https://github.com/tenox7/aclock) straight from its own repository,
compiled with no netOS-specific changes at all.

libnetos provides stdio (the printf family, `FILE`, `fopen`/`fread`/`fwrite`),
`string.h`, `malloc`, `strtol`/`atoi`, `getenv`, the time functions and
`sleep`/`usleep`. Most of it is plain computation done locally; file I/O, time,
memory and `select` go through netOS, and `qsort`/`sscanf` are called straight
out of the kernel. `netos.h` exposes the kernel's own C library directly.

BSD sockets work through `sys/socket.h`, `netinet/in.h`, `arpa/inet.h`,
`netdb.h` and `errno.h`, mapped straight onto the kernel's socket calls.
`examples/nettest.c` is a TCP/UDP echo server, or a client given a host.

Process control is `vfork`, `execv`/`execl`, `dup2`, `pipe`, `wait` and
`kill` (`unistd.h`, `signal.h`, `sys/wait.h`). netOS has no `fork`, only
`vfork`, so a child may only rearrange fds and `execv`/`_exit`.
`examples/rshd.c` is a BSD remote-shell daemon built on all of it, and
`examples/telnetd.c` a telnet server running the netOS shell over a socketpair.

Floating point goes through libgcc soft float — the i960 core has no FPU, and
the i960 gcc configuration never built `fp-bit.c`, so the SDK builds it. `%f`
and `%e` work, `%g` maps to `%f`, and the integer part must fit in a long.
This costs roughly 11 KB per image because printf always pulls the float code
in; call `netos_printf` instead if size matters more.

## Layout

    Dockerfile      builds the whole toolchain
    nsdk            run a toolchain command against the current directory
    netos-gcc       compiler driver: C in, loadable b.out image out
    mkbout.c        post-processor that makes ld's output loadable
    build/          binutils and gcc build + patches
    libnetos/       the C library, headers and a host-side test harness
    examples/       sample programs
    tools/          the reverse-engineering scripts
    prebuilt/       ready-to-run images

## Toolchain

binutils 2.11.2 and gcc 2.95.3 targeting `i960-intel-nindy` — the last releases
whose assembler emits i960 b.out and whose compiler has an i960 back end. Three
things are non-obvious:

* The container is **32-bit on purpose**. gas 2.11's b.out header struct is
  declared with `unsigned long`, so on a 64-bit host it writes 88-byte headers
  instead of 44 and nothing can read them.
* `patch-binutils.sh` fixes three places where 2001-era C no longer compiles
  under gcc 12: `cpu-i960.c` lowercases its own `const` argument in place, and
  `tc-i960.h` declares an array of a still-incomplete struct.
* libgcc needs `libgcc1.null` for the machine-specific half, and
  `LIBGCC2_CFLAGS` must be *appended to* rather than replaced or you lose
  `-DIN_GCC`.

## Executable format

b.out, magic `0x010d`, 44-byte header, text at VMA 0, relocations retained and
no symbol table: the loader places the image anywhere and enters at `e_entry`.
`mkbout` sets the entry point from a symbol and truncates the symbol table after
`ld -r`.

Two rules the loader enforces, both of which produce "Invalid executable format"
if broken:

* Relocations must be `abs32` or `pcrel24` only. gcc assembles calls to `callx`
  with the relaxable *bsr* bit set. `ld -relax` would clear it, but b.out
  relaxation is broken in binutils 2.11, so `mkbout` clears it instead.
* Build with `-fno-common`. `ld -r` leaves tentative definitions as COMMON with
  symbolic relocations, which dangle once the symbol table is stripped.

`mkbout` refuses to emit an image with either problem.

## System calls

Services are i960 `calls` traps. The group is the system-procedure number; the
sub-function index is written through a per-task word whose address comes back
from `calls 8` with `g0=1`. Group 8 is the bootstrap and takes its selector in
`g0` directly.

    ld slot,r4 ; lda IDX,r5 ; st r5,(r4) ; calls GROUP

Group 7 is the system call layer and group 10 a full 4.4BSD C library with at
least 184 entries. Groups 9, 11 and 12 are Xlib, Xt and the widget set — 514
entries between them, entirely unexplored. The PowerPC 8xx build of netOS uses
the same group/index pairs, encoded as `sc` with `(group<<16)|index` in `r0`.

| group/index | call | how it was identified |
|---|---|---|
| 7/0x00 | accept | perror `accept` in rfloppyd and rexec |
| 7/0x01 | bind | perror `bind` in rfloppyd |
| 7/0x02 | close | elimination within `_dd` |
| 7/0x03 | connect | perror `tn3270: connect`; rexec retries it on errno 5 |
| 7/0x06 | exit | `mov 0,g0` before a non-returning call |
| 7/0x07 | getpeername | perror `getpeername` in vidconf |
| 7/0x09 | ioctl | FIONBIO and audio ioctl messages; 259 sites |
| 7/0x0b | listen | `listen(s, 5)`, perror `listen` in rfloppyd |
| 7/0x04 | dup | one fd arg in netscape |
| 7/0x05 | dup2 | `dup2(fd, 2)` in netscape |
| 7/0x0a | kill | `kill(pid, 15)` in netscape's shutdown path |
| 7/0x0d | open | third argument is `0666`; args are `/dev/audio` etc |
| 7/0x0e | read | elimination within `_dd` |
| 7/0x0f | readv | Xlib `_XReadPad`: `(fd, iov, 2)` |
| 7/0x10 | recvfrom | 6 args in every UDP program: XDMCP, ICA browser |
| 7/0x12 | sendto | perror `sendto` in simulcast |
| 7/0x13 | setsockopt | `(s, 0xffff, 4, &on, 4)`, perror `setsockopt` |
| 7/0x16 | socket | `(2, 1, 0)`, perror `socket` |
| 7/0x17 | socketpair | `(1, 1, 0, sv)` in multiNC |
| 7/0x18 | wait | one status-ptr arg, returns the pid |
| 7/0x19 | write | return compared against the requested count |
| 7/0x1a | writev | Xlib `_XSend` |
| 7/0x1b | pipe | `pipe(&fds[2])` in tar, netscape, gview |
| 7/0x1d | execv | `execv(path, argv)` then "olvwm: exec" error |
| 7/0x1e | vfork | the shipped apps' error string says "Vfork" |
| 7/0x22 | chdir | `_du` "cannot change to directory %s" |
| 7/0x23 | stat | `(path, statbuf)`, paired with 0x24 in `_ls` |
| 7/0x24 | lstat | same shape as 0x23, adjacent branch |
| 7/0x29 | lseek | offset computed as `a*b`, whence 0 |
| 7/0x26 | getpid | no args, result stored as a pid |
| 7/0x2c | select | 5 args ending in a timeval; xscreensaver's delay idiom |
| 7/0x2f | getsockname | perror `getsockname` in rexec |
| 7/0x45 | readlink | `(path, stackbuf, 256)` in `_ls` |
| 7/0x47 | rmdir | only extra call in `_rmdir` |
| 9/0xfb | XOpenDisplay | `waitforserver` retries it with a display name |
| 10/0x07 | bcopy(src,dst,n) | the localtime shim in `dclock` |
| 10/0x09 | bzero | `FD_ZERO`; `(&sin, 16)` before bind |
| 10/0x17 | fopen | `(path, "r")`, result null-checked |
| 10/0x18 | fprintf | stderr plus a format string; 1453 call sites |
| 10/0x1c | free | frees localtime's result; most-called entry, 1545 sites |
| 10/0x20 | getenv | args DISPLAY, GS_LIB, POSIXLY_CORRECT, TABSIZE |
| 10/0x24 | inet_addr | tn3270 tests the result for -1 before "unknown host" |
| 10/0x26 | inet_ntoa | xhost's fallback when gethostbyaddr fails |
| 10/0x2a | malloc | GNU `xmalloc` in `_ls` retries it with 1 on NULL |
| 10/0x36 | printf | the `"%s\n"` call site in `_rmdir` |
| 10/0x3a | qsort | `(base, count, 0x50 element size, comparison fn)` in `_ls` |
| 10/0x49 | sprintf | `(stack buffer, format, arg)` in `_ls` |
| 10/0x4c | sscanf | formats use `%[^"]`, which is scanf-only syntax |
| 10/0x4d | strcasecmp | rexec's `.netrc` parser, `strcasecmp(host, tokval)` |
| 10/0x4e | strcat | `(dst, "XXXXXX")`, `(dst, "fonts.dir")` |
| 10/0x4f | strchr | rexec's `.netrc` parser, `index(hostname, '.')` |
| 10/0x50 | strcmp | 2 args of option names, result tested against 0 |
| 10/0x51 | strcpy | rexec: `strcpy(malloc(strlen(tok) + 1), tok)` |
| 10/0x54 | strlen | the same line; rexec's `write(s, num, strlen(num) + 1)` |
| 10/0x57 | strncmp | 3 args, XPM keywords, `"nfs://"`, `"%!PS-Adobe-"` |
| 10/0x67 | readdir | every site is a `while (readdir(d))` loop between 0x66 and 0x6b |
| 10/0x76 | fputc | `fputc('\n', stream)` — first argument is the literal 0xa |
| 10/0x7c | gethostbyname | returns a **malloc'd** hostent; apps copy it and free it |
| 10/0x7d | gethostbyaddr | `(addr, 4, AF_INET)` after getpeername in vidconf |
| 10/0x85 | time | `time(0)` immediately before a struct tm appears |
| 10/0x8b | asctime | localtime's wrapper shape, 26-byte copy |
| 10/0x8c | ctime | same wrapper, 26-byte copy |
| 10/0x8d | gmtime | same wrapper, 44-byte struct tm copy |
| 10/0x8e | localtime | returns a **malloc'd** struct tm the caller must free |
| 10/0x8f | strrchr | basename idiom: `p = X(name, '/'); p = p ? p + 1 : name` |
| 10/0x9a | getservbyname | `("ntrigue-rf", "tcp")` in rfloppyd, then reads `s_port` |
| 10/0x9b | shutdown | `shutdown(s, 1)` at stdin EOF in rexec |

The first block of groups 7 and 10 is in alphabetical order (7 runs `accept`
to `writev`; 10 has `malloc`, `memchr` .. `memset`, `strcasecmp` .. `strlen`),
so each identified entry pins down its neighbours. `errno` lives at the address
`calls 8` returns for `g0=0`, with netOS's own numbering from the kernel's name
table at 0x302cbf50. Sockets are 4.3BSD: 16-bit `sa_family` and no `sa_len`,
BSD `SOL_SOCKET`/`SO_*` values, ioctls numbered `('f'<<8)|n`.

Also identified, not wrapped yet: 7/0x14 sigblock, 7/0x15 sigsetmask,
7/0x21 fcntl, 7/0x25 fstat, 7/0x28 unlink, 10/0x2f memset, 10/0x34 perror,
10/0x48 sleep, 10/0x55 strncasecmp, 10/0x5e syslog, 10/0x66 opendir,
10/0x6b closedir; 13/0x05 returns the `FILE` for fd 0, 1 or 2.

Probable but unverified: `10/0x2d` memcpy. `stat`/`lstat` are certainly that
pair, but which index is which is a guess — they appear in adjacent branches of
the same test. `struct tm` is the BSD 44-byte layout (nine ints, `tm_gmtoff`,
`tm_zone`).

## Reverse-engineering tools

These operate on a netOS distribution, which is not included here.

    tools/stubmap.sh <binary>                   group/index pairs a binary calls
    tools/callsites.sh <binary> <group/index>   how a stub is actually called
    tools/fingerprint.sh <dir>                  profile every stub in every binary
    tools/scan-syscalls.sh <binaries>           the same for PowerPC images

`fingerprint.sh` records, for each call site, the argument count, whether the
result is used or null-checked, and any string argument; `aggregate.awk` merges
the records. Running it over all 87 i960 binaries yields about 24,500 call
sites, which is how most of the group-10 entries above were named — a format
string identifies the printf family at once, `POSIXLY_CORRECT` names `getenv`,
and a result that is null-checked at every single site names `malloc`.
`tools/syscalls.txt` is the equivalent dump for the PowerPC binaries.

The kernel is stripped — `nm` reports no symbols — and the group-10 dispatcher
is not a plain jump table. All 540 indexed tables in the image were measured;
the largest relevant one, 183 entries at `0x301937fc`, is an unrelated switch
whose default case covers every index we know. The dispatch is probably one of
49 register-based `(gN)[gM*4]` sites, none yet tied to `calls 10`. Finding it
would yield the remaining ~150 library entries in one go.

## Status and limitations

Built and run on real hardware. Not exhaustively tested.

* i960 only. PowerPC 8xx netOS would need a `powerpc-eabi` toolchain, but the
  system call map above transfers unchanged.
* No X11. Group 9 is Xlib and completely unmapped apart from `XOpenDisplay`.
* `%f` handles values whose integer part fits in a long; larger prints `huge`.
* No `opendir`/`readdir`, no `signal` handlers (only `kill`), no `getsockopt`
  or `gethostname`. Only `vfork`, so no post-`fork` code before `execv`.

## License

Apache 2.0. `examples/aclock-vt100.c` is Antoni Sawicki's, included unmodified
from the aclock distribution to demonstrate that upstream source builds as-is.
