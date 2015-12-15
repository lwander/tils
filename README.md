# HTTP server written in C

The goal is to create a static HTTP library, with a couple of constraints.

1. Minimize memory allocations.

2. Minimize memory copies.

3. Non-blocking IO whenever possible.

4. Run 1 thread per CPU core.

5. Minimal connection overhead.
