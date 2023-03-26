Step 1. 

For this step, I wrote a functional simulator of a subset of ARM.


/*** Files ***/

1. "src/" subdirectory 
- The shell: "shell.h", "shell.c" 
- The simulator skeleton: "sim.c"

2. "inputs/" subdirectory 
- Test inputs for the simulator (ARM assembly code): "*.s"
- ARM assembler/hexdump (assembly code -> machine code -> hexdump): "asm2hex"

3. Reference simulator
- You can check the register dump and memory dump (0x10000000 to 0x10000100).
- You should run the test inputs on your simulator and reference simulator and the results must match.

4. "ref"
- LEGv8_Reference_Data.pdf is a simplified ARM Instruction Set reference data.
- However, for LSL and LSR, LEGv8 is different from ARMv8. See README for details.
- DDI0487B_a_armv8_arm.pdf is the detail ARMv8 Instruction Set reference manual.

5. "aarch64-linux-android-4.9"
- Google Android compiler toolchain for translating assembly code into machine code.

/*** Instructions ***/

Run "make" to compile the simulator.

$ cd src/
$ make

You should now have an executable file named "sim".


2. Use "asm2hex" to convert the test inputs ("*.s") into hexdumps of assembled
machine code ("*.x").

$ cd inputs/
$ ./asm2hex addis.s

You should now have a hexdump of the assembled machine code "addis.x". Repeat
this step for the rest of the test inputs.


3. Run the hexdump in the simulator.

$ src/sim inputs/addis.x

>> ARM Simulator
>> 
>> Read 5 words from program into memory.
>> 
>> ARM-SIM> go
>> 
>> Simulating...

Press Control+C to exit the simulator.
