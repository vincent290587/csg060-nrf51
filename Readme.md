
# CSG060 bike hack

## UART in qemu

Enumerate serials in order to send UART on telnet as server:

`-serial stdio -chardev socket,server=on,id=s0,host=127.0.0.1,port=1235 -serial chardev:s0`

## OPENOCD

```cmd

C:\Tools\OpenOCD-20231002-0.12.0\bin\openocd.exe -f board/nordic_nrf51_dap.cfg

telnet 127.0.0.1 4444

rtt setup 0x20002B00 512 "SEGGER RTT"

rtt start

rtt channels

rtt server start 9090 0

telnet 127.0.0.1 9090

```
