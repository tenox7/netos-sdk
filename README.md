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
| 7/0x02 | close | elimination within `_dd` |
| 7/0x06 | exit | `mov 0,g0` before a non-returning call |
| 7/0x0d | open | third argument is `0666`; args are `/dev/audio` etc |
| 7/0x0e | read | elimination within `_dd` |
| 7/0x19 | write | return compared against the requested count |
| 7/0x23 | stat | `(path, statbuf)`, paired with 0x24 in `_ls` |
| 7/0x24 | lstat | same shape as 0x23, adjacent branch |
| 7/0x29 | lseek | offset computed as `a*b`, whence 0 |
| 7/0x2c | select | 5 args ending in a timeval; xscreensaver's delay idiom |
| 7/0x45 | readlink | `(path, stackbuf, 256)` in `_ls` |
| 7/0x47 | rmdir | only extra call in `_rmdir` |
| 9/0xfb | XOpenDisplay | `waitforserver` retries it with a display name |
| 10/0x07 | bcopy(src,dst,n) | the localtime shim in `dclock` |
| 10/0x17 | fopen | `(path, "r")`, result null-checked |
| 10/0x18 | fprintf | stderr plus a format string; 1453 call sites |
| 10/0x1c | free | frees localtime's result; most-called entry, 1545 sites |
| 10/0x20 | getenv | args DISPLAY, GS_LIB, POSIXLY_CORRECT, TABSIZE |
| 10/0x2a | strlen | 1 arg, arbitrary strings, result always used |
| 10/0x36 | printf | the `"%s\n"` call site in `_rmdir` |
| 10/0x3a | qsort | `(base, count, 0x50 element size, comparison fn)` in `_ls` |
| 10/0x49 | sprintf | `(stack buffer, format, arg)` in `_ls` |
| 10/0x4c | sscanf | formats use `%[^"]`, which is scanf-only syntax |
| 10/0x4e | strcpy | `(dst, "XXXXXX")`, `(dst, "fonts.dir")` |
| 10/0x50 | strcmp | 2 args of option names, result tested against 0 |
| 10/0x57 | strncmp | 3 args, XPM keywords, `"nfs://"`, `"%!PS-Adobe-"` |
| 10/0x67 | malloc | 1 arg, result null-checked at every site |
| 10/0x76 | fputc | `fputc('\n', stream)` — first argument is the literal 0xa |
| 10/0x85 | time | `time(0)` immediately before a struct tm appears |
| 10/0x8b | asctime | localtime's wrapper shape, 26-byte copy |
| 10/0x8c | ctime | same wrapper, 26-byte copy |
| 10/0x8d | gmtime | same wrapper, 44-byte struct tm copy |
| 10/0x8e | localtime | returns a **malloc'd** struct tm the caller must free |
| 10/0x8f | strchr | second argument is the literal `'/'` |

Probable but unverified: `10/0x2d` strncpy, `10/0x4d` strcasecmp. `stat`/`lstat`
are certainly that pair, but which index is which is a guess — they appear in
adjacent branches of the same test. `struct tm` is the BSD 44-byte layout
(nine ints, `tm_gmtoff`, `tm_zone`).

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
* No `opendir`/`readdir`, no `signal`, no sockets.

## License

Apache 2.0. `examples/aclock-vt100.c` is Antoni Sawicki's, included unmodified
from the aclock distribution to demonstrate that upstream source builds as-is.
