#!/bin/sh
# Loopback smoke test of the socket layer: nettest server and clients on the host.
set -e
NORUN=1 sh test/run.sh ../examples/nettest.c
/tmp/lt/run 7777 > /tmp/lt/srv.log &
sleep 1
/tmp/lt/run 127.0.0.1 7777 hello | grep -q '^hello'
/tmp/lt/run localhost 7777 quit > /dev/null
/tmp/lt/run -u 7778 > /tmp/lt/udp.log &
sleep 1
/tmp/lt/run -u 127.0.0.1 7778 ping | grep -q ': ping$'
/tmp/lt/run -u 127.0.0.1 7778 quit > /dev/null || true
wait
grep -q 'echo 7 bytes' /tmp/lt/srv.log
echo "nettest loopback ok"
