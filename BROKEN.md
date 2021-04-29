* Start with completely reset MCU (no boot flash bit)
* OpenOCD launches successfully with `sam4.cpu: hardware has 6 breakpoints, 4 watchpoints`
* VSCode: run "Set bootflash" task
```
warning: No executable has been specified and target does not support
determining executable automatically.  Try using the "file" command.
0x0080050a in ?? ()
[Inferior 1 (Remote target) detached]
```
* Make clean and make tasks are run successfully
* Make program:
```
arm-none-eabi-gdb -batch -x ../../mmculib/../mat91lib/sam4s/scripts/program.gdb ledflash1.bin
0x008000da in ?? ()
target halted due to debug-request, current mode: Thread 
xPSR: 0x01000000 pc: 0xfffffffe msp: 0xfffffffc
Loading section .vectors, size 0x1c lma 0x400000
Loading section .text, size 0x2e1c lma 0x400020
Loading section .rodata, size 0x4c lma 0x402e3c
Loading section .eh_frame, size 0x4 lma 0x402e88
Loading section .ARM.exidx, size 0x8 lma 0x402e8c
Loading section .data, size 0x94c lma 0x402e94
Start address 0x400020, load size 14300
Transfer rate: 24 KB/sec, 2383 bytes/write.
[Inferior 1 (Remote target) detached]
```

* Open OCD begins spamming:
```
Info : dropped 'gdb' connection (error -400)
Error: jtag status contains invalid mode value - communication failure
Polling target sam4.cpu failed, trying to reexamine
Examination failed, GDB will be halted. Polling again in 100ms
Info : Previous state query failed, trying to reconnect
Error: jtag status contains invalid mode value - communication failure
Polling target sam4.cpu failed, trying to reexamine
Examination failed, GDB will be halted. Polling again in 300ms
```

* When tryng to run the program, it fails to erase the flash, presumably because it cant find the target as shown in the above snippet

```
Info : accepting 'gdb' connection on tcp/3333
Halting target due to gdb attach

Error: Target not halted
Error: failed erasing sectors 0 to 1
Error: flash_erase returned -304
```