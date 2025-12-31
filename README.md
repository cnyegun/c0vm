# What is C0VM? 

C0VM is a virtual machine, influenced by Java Virtual Machine (JVM) and the LLVM. 

C0VM is much simpler than the JVM, following the design of C0, a safe subset of C.
There is no bytecode verification; in this way the machine bears a closer resemblance to the LLVM.
It is fully functional design and should be able to execute arbitrary C0 code.

# 1. Structure of the C0VM

Compiled code to be executed by the C0 virtual machine is represented in a byte code format (.bc0).

This file contains numerical and string constants as well as byte code for the functions defined in the C0 source. 

## The Type **c0_value**

C0 has types: **int**, **bool**, **char**, **string**, **t[]**, and **t\***. 
In C0VM, we will represent _primitive types_ as 32-bit signed integers (abbr. "w32") and the _reference types_ as void pointers (abbr. "*") 

| C0 type | COVM type | C type   |
|--------:|:---------:|:---------|
| int     | w32       | int32\_t  |
| bool    | w32       | int32\_t  |
| char    | w32       | int32\_t  |
| t[]     | *         | void*    |
| t*      | *         | void*    |


