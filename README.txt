15-122 Principles of Imperative Computation
The C0VM

==========================================================

Useful information:
   c0vm-ref.txt           - Bytecode implementation reference

Files you won't modify:
   lib/contracts.h     - Contracts for C
   lib/xalloc.h        - NULL-checking allocation
   lib/stack.h         - Stacks (with O(1) stack_size!)
   lib/c0v_stack.h     - C0 Value Stacks (with O(1) stack_size!)
   lib/c0vm_abort.h    - Functions for your C0VM to report runtime errors
   lib/c0vm_c0ffi.h    - Interface to the library functions
   lib/c0vm.h          - Header file, contains C0VM types and functions
   lib/read_program.h  - Reading in C0 bytecode from a file
   lib/*.o             - machine-specific object files
   c0vm_main.c         - Main function - loads bytecode, handles return

Files you will modify:
   c0vm.c              - Executes C0 bytecode

==========================================================

Compiling a C0 program to create a .bc0 file (produces tests/iadd.bc0):
   % cc0 -b tests/iadd.c0

Compiling and running your C0VM implementation (with -DDEBUG)
(Must compile iadd.c0 to iadd.bc0 first)
   % make
   % ./c0vmd tests/iadd.bc0

Compiling and running your C0VM implementation (without -DDEBUG)
(Must compile iadd.c0 to iadd.bc0 first)
   % make
   % ./c0vm tests/iadd.bc0

==========================================================

Creating a tarball to submit with autolab.autolab.cmu.edu web interface:
   % tar -czvf handin.tgz c0vm.c
