# Multi-threaded DNS Resolver

## Introduction
In this assignment, you will build a multi-threaded application that resolves domain names (for example, `google.com`) to IP addresses. This mirrors one part of the work that takes place when a web browser locates a remote server, but here the work must be coordinated across multiple threads.

Your implementation should use a producer-consumer style design:
- Requester thread(s) read hostnames from the input files, lookup the IPv4 address, and place them into a shared, thread-safe data structure.
- Resolver thread(s) remove hostnames and IPs from that shared structure, and write the results to the log file.

The goal of the assignment is not only to perform DNS lookups correctly, but to do so safely in the presence of concurrency.

## File overview
- [`../common/hostnames/`](../common/hostnames/): text files containing hostnames that can be used as input
- `array.c`: implement your thread-safe shared array or queue here (from PA 3!)
- `array.h`: header file for your shared array or queue
- `Makefile`: GNU Makefile used to compile your code
  - `make`: builds an executable named `multi-lookup`
  - `make clean`: removes compiled binaries and object files
- `multi-lookup.c`: main program and thread coordination logic
- `multi-lookup.h`: header file for your resolver implementation

## Implementation
Your program should process filenames passed on the command line, along with a path to a log file:

```sh
./multi-lookup <log_file> [ <names1> <names2> ... ]
```

Each input file contains one hostname per line. Your program should:

1. Create the threads needed for your design.
2. Read each hostname from each valid input file.
3. Pass hostnames through a shared thread-safe data structure.
4. Resolve each hostname to an IPv4 address.
5. Write the hostname and resolved IP address to the log file.

Use the following output format:

```text
google.com, 142.250.72.14
```

If an IP address cannot be found for a given hostname, write `NOT_RESOLVED` in place of the address:

```text
glsdkjf.com, NOT_RESOLVED
```

Your program only needs to resolve one IPv4 address per hostname.

After all work has completed, and before the program exits, print the total time taken:

```text
./multi-lookup: total time is 2.810863 seconds
```

## Threading requirements
Your solution must explicitly address the multi-threaded nature of the program.

- Input files may be processed by multiple requester threads.
- DNS lookups may be performed by multiple resolver threads.
- The shared array or queue must be safe for concurrent access.
- Access to the log file must be synchronized so output is not corrupted or interleaved.
- Threads must terminate cleanly once all input files have been processed and all queued hostnames have been resolved.

You may choose the exact threading structure, but your design should clearly separate shared-state management from lookup logic. Use `pthread` synchronization primitives where appropriate.

## Error handling
Ensure your program accounts for the following:

- Missing arguments: terminate with a usage synopsis written to `stdout`
- Existing writable log file: overwrite its contents
- Missing log file: create it
- Missing or unreadable input file: print `Invalid file <filename>` to `stderr` and continue to the next file

All function calls should be checked for errors. If an error is fatal, print an informative error message and exit with an appropriate status code.

## Submission requirements
- A `zip` file called `submission.zip` containing all of the code needed to compile and run you project. The provided `template.zip` is an example of what all should be included. A list is also provided below:
  - `array.c`
  - `array.h`
  - `multi-lookup.c`
  - `multi-lookup.h`
  - `Makefile`
- A 1 page report describing what you learned, difficulties you encountered, how you addressed synchronization issues, and any comments about the assignment
