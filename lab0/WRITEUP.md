# Lab 0

## fork_loop

### Output
0Yo!1Yo!Yo!2Yo!Yo!Yo!3Yo!Yo!Yo!Yo!4Yo!Yo!Yo!Yo!Yo!5Yo!Yo!Yo!Yo!Yo!5

### What is happening?
Essentially each new child is inheriting the parents buffer when forked so when printf() is called, even though relative to the child, the only thing in the buffer is "i", the inherited buffer gets printed as well. It's worth noting that fork returns 0 for the child so any child will never enter the while loop.

### Output (GDB)
0[Detaching after fork from child process 5493]
Yo!1[Detaching after fork from child process 5494]
Yo!Yo!2[Detaching after fork from child process 5495]
Yo!Yo!Yo!3[Detaching after fork from child process 5496]
Yo!Yo!Yo!Yo!4[Detaching after fork from child process 5497]
Yo!Yo!Yo!Yo!Yo!5Yo!Yo!Yo!Yo!Yo!5[Inferior 1 (process 5488) exited normally]

### What is happening now?
Essentially the same thing as before but now its easier to see

## who_runs_first

### What is observed using write system call?
    32 childparent
    968 parentchild

### What is observed using printf?
    First 15 line sample.

    <>
    245 child 54
    373 child nt 382
    382 child parent 0
    382 parent 1
    373 parent 10
    283 parent 100
    282 parent 101
    281 parent 102
    280 parent 103
    279 parent 104
    278 parent 105
    277 parent 106
    276 parent 107
    275 parent 108
    274 parent 109

    <>

### What is observed using printf followed by fflush?
    First 30 line sample

    <>
      1 child parent 11
      1 child parent 15
      1 child parent 182
      1 child parent 184
      1 child parent 197
      1 child parent 204
      1 child parent 209
      1 child parent 25
      1 child parent 263
      1 child parent 29
      1 child parent 417
      1 child parent 504
      1 child parent 535
      1 child parent 545
      1 child parent 549
      1 child parent 553
      1 child parent 562
      1 child parent 564
      1 child parent 568
      1 child parent 569
      1 child parent 583
      1 child parent 88
      1 child parent 89
      1 parent child 0
      1 parent child 1
      1 parent child 10
      1 parent child 100
      1 parent child 101
      1 parent child 102
      1 parent child 103

      <>

### Explain the differences in outputs
    The differences are caused by process scheduling and stdio buffering. "write" shows only which process runs first, while "printf" introduces buffering that can delay and mix outputs, and "fflush" forces immediate printing so the order reflects scheduling more clearly.

### Sleep system call
    Adding a short sleep (0.1s) before each `write()` makes the execution more interleaved and increases the chances that the child runs before the parent, so the output order becomes more mixed. With a longer sleep (1s), the processes alternate more predictably, often producing a more consistent and less chaotic ordering because each process yields the CPU for a noticeable time.
