# ARM-Simulator

This C program is an instruction-level simulator for a subset of the ARMv8 instruction set. This instruction-level simulator will model the behavior of each instruction, and
will allow the user to run ARM programs and see their outputs. It features 5-stage pipelining, branch predictor, and Level-1 instruction and data caching.  


## Step 1:

The simulator will process an input file that contains an ARM program. Each line of the input file corresponds
to a single ARM instruction written as a hexadecimal string. For example, `0x91000440` is the hexadecimal
representation of addi X0, X2, 1. The simulator will execute any given input program one instruction at
a time. After each instruction, the simulator will modify the ARM architectural state which includes values
stored in both registers and memory.

The simulator is partitioned into two main sections: (1) the shell and (2) the simulation routine. In the `src/` directory, `shell.c` and `shell.h` implemented the shell. There is a third file `sim.c` that implements the simulation
routine.

## Step 2:

I pipelined ARM simulator for a limited subset of the ARMv8
instruction set (the same set that I implemented in Step 1). This pipelined simulator not only models
the functional behavior of each instruction, but also enables pipelined instruction processing.
I implemented the 5 pipeline stages, including Instruction Fetch, Instruction
Decode, Execute, Memory Access, and Register Writeback. The simulator is able to handle all possible
branch cases, stall cases, and bypass cases. 

## Step 3:
I expanded on my pipelined ARM simulator from Step 2 by developing a branch
predictor. The behavior of the branch predictor is fully specified below.

The branch predictor consists of (i) a gshare predictor and (ii) a branch target buffer. The gshare predictor uses an 8-bit global branch history register (GHR). The most recent branch
is stored in the least-significant-bit of the GHR and a value of ‘1’ denotes a taken branch. The predictor
XORs the GHR with bits `[9:2]` of the PC and uses this 8 bit value to index into a 256-entry pattern history
table (PHT). Each entry of the PHT is a 2-bit saturating counter: a taken branch increments whereas a
not-taken branch decrements; the four values of the counter correspond to strongly not-taken (`00`), weakly
not-taken (`01`), weakly taken (`10`), strongly taken (`11`).

The branch target buffer (BTB) contains 1024 entries and is indexed by bits
`[11:2]` of the PC. Each entry of the BTB contains (i) an address tag, which is the full 64 bits of the fetch
stage PC; (ii) a valid bit (1 means the entry is valid, 0 means the entry is not valid); (iii) a bit indicating
whether this branch is conditional (1 means the branch is conditional, 0 means the branch is unconditional);
and (iv) the target of the branch, which is 64 bits, with the low two bits always equal to 2'b00. Note, in
actual hardware implementation only bits 63:12 of the PCs are used as tags. However, we use the full 64
bits just to keep the code clean.

At every fetch cycle, the predictor indexes into both the BTB and the PHT. If the predictor
misses in the BTB (i.e., `address tag != PC` or `valid bit == 0`), then the next PC is predicted as PC+4. If
the predictor hits in the BTB, then the next PC is predicted as the target supplied by the BTB entry when
either of the following two conditions are met: (i) the BTB entry indicates that the branch is unconditional,
or (ii) the gshare predictor indicates that the branch should be taken. Otherwise, the next PC is predicted
as PC+4.

The branch predictor structures are always updated in the execute stage, where both the branch
target and the branch condition are resolved. The update consists of: (i) updating the PHT, which is indexed
using the current value of the GHR and PC, (ii) updating the GHR (note that, the update of GHR must
happen after the update to PHT to make sure we update the right entry), and (iii) updating the BTB.

Unconditional branches do not update the PHT or the GHR, but only the BTB (setting the unconditional
bit in the corresponding entry). When it is needed to add a new PC/branch target to BTB, but the BTB entry
was already taken by another PC with the same lower address bits, it always replaces the entry with
the new PC.

I used BTB to predict indirect branches as well as direct branches. However the accuracy of prediction may be low for indirect branches. Indirect Branches are hard to predict because of the undetermined branch destination.

## Step 4: 

In this step, I extended the 5-stage pipelined ARM machine that I have implemented in previous
steps with level 1 (L1) instruction and data caches. The microarchitecture of the caches is fully specified below:

Instruction Cache: The instruction cache is accessed every cycle in the fetch stage (unless the pipeline is stalled).
Organization. It is a four-way set-associative cache that is 8 KB in size with 32 byte blocks (this implies
that the cache has 64 sets). When accessing the cache, the set index is calculated using bits `[10:5]` of the
PC.
Miss Timing: When the fetch stage misses in the instruction cache, the block must be retrieved from main
memory. An access to main memory takes 50 cycles. On the 50th cycle, the new block is inserted into the
cache. In total, an instruction cache miss stalls the pipeline for 50 cycles. In the 51st cycle, the data is
returned to the processor, so the Instruction Fetch stage is completed at the 51st cycle.

Replacement: When a new block is retrieved from main memory, it is inserted into the appropriate set
within the instruction cache. If any block within the set are empty, the new block is simply inserted into an
empty block. However, if none of the blocks in the set are empty, the new block replaces the least-recently-used
block. For both cases, the new block becomes the most-recently-used block.

Control-Flow: While the fetch stage is stalled due to a miss in the instruction cache, a control-flow
instruction further down the pipeline may redirect the PC. As a result, the pending miss may turn out to be
unnecessary: it is retrieving the wrong block from main memory. In this case, the pending miss is canceled:
the block that is eventually returned by main memory is not inserted into the cache. (A test input of this
case is provided in `inputs/cancel req.s.`) Finally, note that a redirection that accesses the same block as a
pending miss does not cancel the pending miss.

Data Cache: The data cache is accessed whenever a load or store instruction is in the memory stage.
Organization. It is an eight-way set-associative cache that is 64 KB in size with 32 byte blocks (this
implies that the cache has 256 sets). When accessing the cache, the set index is calculated using bits `[12:5]`
of the data address that is being loaded/stored. The L1 data cache is a write-through, allocate-on-write
cache.
Miss Timing & Replacement: Miss timing and replacement of the data cache are identical to the
instruction cache.
Handling Load/Store: Both load and store misses stall the pipeline for 50 cycles. They both retrieve a
new block from main memory and insert it into the cache. The Memory stage of the load/store instruction will take 51 cycles when miss, and 1 cycle when hit. For write miss, memory should be updated in the 51st
cycle.
