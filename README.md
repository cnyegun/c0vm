# What is C0VM? 

C0VM is a virtual machine, influenced by Java Virtual Machine (JVM) and the LLVM. 

C0VM is much simpler than the JVM, following the design of C0, a safe subset of C.
There is no bytecode verification; in this way the machine bears a closer resemblance to the LLVM.
It is fully functional design and should be able to execute arbitrary C0 code.

# 1. Structure of the C0VM

Compiled code to be executed by the C0 virtual machine is represented in a byte code format (.bc0).

This file contains numerical and string constants as well as byte code for the functions defined in the C0 source. 

## The Type `c0_value`

C0 has types: **int**, **bool**, **char**, **string**, **t[]**, and **t\***. 
In C0VM, we will represent _primitive types_ as 32-bit signed integers (abbr. "w32") and the _reference types_ as void pointers (abbr. "*") 

| C0 type | COVM type | C type   |
|--------:|:---------:|:---------|
| int     | w32       | int32\_t  |
| bool    | w32       | int32\_t  |
| char    | w32       | int32\_t  |
| t[]     | *         | void*    |
| t*      | *         | void*    |

## 1.2 Runtime Data

The C0VM defines several types of data that are used during the execution of a program. 
The first three are the operand stack, bytecode and the program counter).

### 1.2.1 The Operand Stack (S)

The C0VM is a _stack machine_, similar in design to the JVM. Arithmetic ops and other instructions pop their operands from an operand stack and push their result back to the operand stack.
The C0VM is defined in terms of operand stack S, a stack of only C0 values.

### 1.2.2 Bytecode (P)

In the C0VM, instructions for the current C0 function are stored in an array of bytes. Functions are stored in the array `bc0->function_pool`. The `main` function is always stored at the index 0. 

### 1.2.3 The Program Counter (pc)

The program counter `pc` holds the address of the program instruction currently being executed. It always starts at 0 when you begin executing a function. Unless a non-local transfer of control occurs (`goto, conditional branch, function call or return`), the program counter is incremented by the number of bytes in the current instruction before the next instruction before the next instruction is fetched and interpreted.

### 1.2.4 Local Variables (V)

Locals variable are stored in a `c0_value` array. Every struct `function_info` has a field `num_vars` which specifies how big this array needs to be in order to store all the locals of that function. The four components S, P, pc, and V are everything that we need to know in order to represent the current state of any function.

### 1.2.5 The Call Stack

The C0VM has a call stack consisting of _frames_, each one containing local variables, a local operand stack, and a return address. The call stack grows when a function is called and shrink when a function returns, deallocating the frame during the return. 
Every frame consists of the four components that represent the current state of some function call that got interrupted in order to call another function. It contains the operand stack S for computing expression values, a pointer P to the function's byte code, a return address pc, which is the address of the next instruction in the interpreted function, and an array of locals V. At any point of execution there is a _current frame_ as well as a _calling frame_. The latter becomes the current frame when a function returns to its caller.

### 1.2.6 Constant Pools

Numerical constants requiring more than 8 bits and all strings constants will be allocated in constant pools. 
There is _integer pool_ and _string pool_. They never change during program execution.

### 1.2.7 Function Pools

Functions are kept in _function pool_ and _native pool_ (for library fns). 

### 1.2.8 The Heap

C0VM allocates strings, arrays and cells directly using calls to `xmalloc` and `xcalloc`.

