
# CSG060 bike hack

## UART in qemu

Enumerate serials in order to send UART on telnet as server:

`-serial stdio -chardev socket,server=on,id=s0,host=127.0.0.1,port=1235 -serial chardev:s0`
