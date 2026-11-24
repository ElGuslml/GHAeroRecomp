# The XDK's atomic operations without a process-wide lock

**Symptom.** None on screen: CPU. Every atomic increment, exchange or compare-exchange the game makes went through
one lock shared by all its threads.

**Cause.** The XDK's inline Interlocked operations disable the core's interrupts around their reserved load and store
(`mfmsr rX`, `mtmsrd r13`, `lwarx` ... `stwcx.`, `mtmsrd rX`). By default the code generator turns that into the
SDK's process-wide recursive mutex, so the job threads, the main thread and the render thread wait on each other at
every one. On the console the pair only stopped interrupts on that one core, never on the others: the operations
were never excluded from each other, only each made atomic.

**Fix.** `skip_msr = true` in `ghaerosmith_functions.toml` (as in GH Warriors of Rock and GH5): each site is the host's
atomic operation alone. Every site was checked by address (GH5's `tools/perf_port/msr_sites_mftb.py`): 68 sites in 51
functions, each a reserved load and store with register arithmetic, compares and branches between. The only sites
of another shape are the XDK's trace functions (`sub_824B4320`-`sub_824B50F8`), which nothing in the game calls.

Measured on 2026-11-24 (NativeXRenderer, 160 FPS, 1280x720, Dream Police on Easy, 20 s of the song, two runs each):
the process used 1.66 and 1.53 cores before, 1.40 and 1.31 after (about -16 %).

**Turn it off:** it is a code generation option: `skip_msr = false` and a new build.
