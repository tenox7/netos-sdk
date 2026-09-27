# Prebuilt images

netOS i960 b.out images, ready to drop into `netOS/bin/i960/` on a boot server.
Add a matching line to `netOS/config/apps.cfg`, `rehash` on the Station, then
run the name:

    app.demo.path:	///%netOSdir%/netOS/bin/%cpu%/demo

| image | what it does |
|---|---|
| `hello` | hand-written assembly, 308 bytes, no C library at all |
| `demo` | argv, malloc, the printf format specifiers, time, sleep |
| `heaptest` | allocates 64 x 4 KB from the kernel heap; sscanf and getenv |
| `systest` | calls the kernel's own C library directly through `netos.h` |
| `floattest` | soft float and `long long` arithmetic |
| `floatfmt` | `%f` and `%e` formatting |
| `aclock` | [aclock](https://github.com/tenox7/aclock), built from unmodified source |
| `nettest` | TCP/UDP echo server on port 7777, or a client given `host port [text]` |
| `rshd` | BSD remote-shell daemon on port 514; run `rsh <station> <cmd>` |
| `telnetd` | telnet server on port 23 running the netOS shell; one session at a time |

Rebuild any of them with `make examples`.
