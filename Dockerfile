FROM --platform=linux/386 i386/debian:bookworm-slim

# 32-bit host on purpose: gas 2.11's b.out header struct is declared with
# `unsigned long`, so on an LP64 host it writes 88-byte headers instead of 44.
RUN apt-get update -qq && apt-get install -y -qq --no-install-recommends \
      build-essential wget ca-certificates gzip bzip2 autotools-dev \
      flex bison texinfo file xxd \
  && rm -rf /var/lib/apt/lists/*

# binutils 2.11.2 is the last release whose gas can emit i960 b.out (obj-bout)
COPY build/patch-binutils.sh build/build-binutils.sh /build/
RUN mkdir -p /w && bash /build/build-binutils.sh && rm -rf /tmp/binutils-* /tmp/b /tmp/b.tgz

COPY mkbout.c /build/
RUN gcc -O2 -o /opt/i960/bin/mkbout /build/mkbout.c

ENV PATH=/opt/i960/bin:$PATH

# gcc 2.95.3 with its i960 back end
COPY build/build-gcc.sh build/float.h /build/
RUN bash /build/build-gcc.sh && rm -rf /tmp/gcc-2.95.3 /tmp/g /tmp/gcc.tgz

# newlib, the C library, with the netOS headers it needs from libnetos
COPY build/build-newlib.sh /build/
COPY libnetos/include /build/libnetos/include
RUN bash /build/build-newlib.sh /build/libnetos/include && rm -rf /tmp/newlib* /tmp/nl

# libnetos, newlib's system layer, and the compiler driver
COPY libnetos /build/libnetos
COPY netos-gcc /opt/i960/bin/netos-gcc
RUN cd /build/libnetos && sh build.sh /opt/i960 && chmod +x /opt/i960/bin/netos-gcc

# the 2.11BSD termcap library, with the common terminals built in
COPY libtermcap /build/libtermcap
RUN cd /build/libtermcap && sh build.sh /opt/i960

# ncurses 5.9, with the common terminals compiled in
COPY build/build-ncurses.sh /build/
RUN bash /build/build-ncurses.sh && rm -rf /tmp/ncurses* /tmp/hnc

WORKDIR /w
