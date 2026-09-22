IMAGE   ?= netos-i960
EXAMPLES = demo heaptest systest floattest floatfmt
RUN      = docker run --rm --platform linux/386 -v "$(CURDIR):/w" -w /w $(IMAGE)

.PHONY: image examples test clean

image:
	docker build --platform linux/386 -t $(IMAGE) .

examples: image
	$(RUN) sh -c 'cd examples && for f in $(EXAMPLES); do netos-gcc $$f.c -o $$f; done \
	              && netos-gcc aclock-vt100.c -o aclock \
	              && i960-intel-nindy-as -o /tmp/h.o hello.s \
	              && i960-intel-nindy-ld -r -o /tmp/h.rel /tmp/h.o \
	              && mkbout /tmp/h.rel hello _start'

# build libnetos against host syscalls and run it natively
test:
	docker run --rm -v "$(CURDIR):/s" -w /s/libnetos debian:bookworm-slim sh -c \
	  'apt-get update -qq >/dev/null && apt-get install -y -qq --no-install-recommends gcc libc6-dev >/dev/null; \
	   sh test/run.sh'

clean:
	rm -f examples/demo examples/heaptest examples/systest examples/floattest \
	      examples/floatfmt examples/aclock examples/hello examples/*.o examples/*.rel
