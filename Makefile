IMAGE   ?= netos-i960
EXAMPLES = demo heaptest systest floattest floatfmt nettest rshd telnetd
PORTS    = vi busybox
RUN      = docker run --rm --platform linux/386 -v "$(CURDIR):/w" -w /w $(IMAGE)

.PHONY: image examples ports clean

image:
	docker build --platform linux/386 -t $(IMAGE) .

examples: image
	$(RUN) sh -c 'cd examples && for f in $(EXAMPLES); do netos-gcc $$f.c -o $$f; done \
	              && netos-gcc aclock-vt100.c -o aclock \
	              && netos-gcc curses.c -o curses -lncurses \
	              && i960-intel-nindy-as -o /tmp/h.o hello.s \
	              && i960-intel-nindy-ld -r -o /tmp/h.rel /tmp/h.o \
	              && mkbout /tmp/h.rel hello _start'

ports: image
	$(RUN) sh -c 'for p in $(PORTS); do sh ports/$$p/build.sh || exit 1; done'

clean:
	rm -f examples/demo examples/heaptest examples/systest examples/floattest \
	      examples/floatfmt examples/nettest examples/rshd examples/telnetd examples/aclock examples/curses examples/hello \
	      examples/*.o examples/*.rel ports/vi/exvi ports/busybox/busybox ports/busybox/apps.cfg
