#include <assert.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

#include "lib/xalloc.h"
#include "lib/stack.h"
#include "lib/contracts.h"
#include "lib/c0v_stack.h"
#include "lib/c0vm.h"
#include "lib/c0vm_c0ffi.h"
#include "lib/c0vm_abort.h"

/* call stack frames */
typedef struct frame_info frame;
struct frame_info {
  c0v_stack_t S;   /* Operand stack of C0 values */
  ubyte *P;        /* Function body */
  size_t pc;       /* Program counter */
  c0_value *V;     /* The local variables */
};

int execute(struct bc0_file *bc0) {
  REQUIRES(bc0 != NULL);

  /* Variables */
  c0v_stack_t S = c0v_stack_new(); 							/* Operand stack of C0 values */
  ubyte *P = bc0->function_pool[0].code;	     	/* Array of bytes that make up the current function */
  size_t pc = 0;							     							/* Current location within the current byte array P */
	/* Local variables (you won't need this till Task 2) */
  c0_value *V = xcalloc((size_t) bc0->function_pool[0].num_vars, sizeof *V);   
  (void) V;      // silences compilation errors about V being currently unused

  /* The call stack, a generic stack that should contain pointers to frames */
  /* You won't need this until you implement functions. */
  // gstack_t callStack;
  // (void) callStack; // silences compilation errors about callStack being currently unused

  while (true) {

#ifdef DEBUG
    /* You can add extra debugging information here */
    fprintf(stderr, "Opcode %x -- Stack size: %zu -- PC: %zu\n",
            P[pc], c0v_stack_size(S), pc);
#endif

    switch (P[pc]) {

    /* Additional stack operation: */

    case POP: {
      pc++;
      c0v_pop(S);
      break;
    }

    case DUP: {
      pc++;
      c0_value v = c0v_pop(S);
      c0v_push(S,v);
      c0v_push(S,v);
      break;
    }

    case SWAP: {
			pc++;
			c0_value v1 = c0v_pop(S);
			c0_value v2 = c0v_pop(S);
			c0v_push(S, v1);
			c0v_push(S, v2);
			break;
		}


    /* Returning from a function.
     * This currently has a memory leak! You will need to make a slight
     * change for the initial tasks to avoid leaking memory.  You will
     * need to revise it further when you write INVOKESTATIC. */

    case RETURN: {
      int retval = val2int(c0v_pop(S));
      assert(c0v_stack_empty(S));
// Another way to print only in DEBUG mode
IF_DEBUG(fprintf(stderr, "Returning %d from execute()\n", retval));
      // Free everything before returning from the execute function!
			c0v_stack_free(S);
			free(V);
      return retval;
    }

    /* Arithmetic and Logical operations */

    case IADD: {
			pc++;
			int32_t y = val2int(c0v_pop(S));
			int32_t x = val2int(c0v_pop(S));
			c0v_push(S, int2val(x + y));
			break;
		}

    case ISUB: {
			pc++;
			int32_t y = val2int(c0v_pop(S));
			int32_t x = val2int(c0v_pop(S));
			c0v_push(S, int2val(x - y));
			break;
		}

    case IMUL: {
			pc++;
			int32_t y = val2int(c0v_pop(S));
			int32_t x = val2int(c0v_pop(S));
			c0v_push(S, int2val(x * y));
			break;
		}

    case IDIV: {
			pc++;
			int32_t y = val2int(c0v_pop(S));
			if (y == 0) {
				c0_arith_error("Division by zero");
			}
			int32_t x = val2int(c0v_pop(S));
			if (x == INT32_MIN && y == -1) {
				c0_arith_error("INT32_MIN / -1");
			}
			c0v_push(S, int2val(x / y));
			break;
		}

    case IREM: {
			pc++;
			int32_t y = val2int(c0v_pop(S));
			if (y == 0) {
				c0_arith_error("Division by zero");
			}
			int32_t x = val2int(c0v_pop(S));
			if (x == INT32_MIN && y == -1) {
				c0_arith_error("INT32_MIN / -1");
			}
			c0v_push(S, int2val(x % y));
			break;
		}

    case IAND: {
			pc++;
			int32_t y = val2int(c0v_pop(S));
			int32_t x = val2int(c0v_pop(S));
			c0v_push(S, int2val(x & y));
			break;
		}

    case IOR: {
			pc++;
			int32_t y = val2int(c0v_pop(S));
			int32_t x = val2int(c0v_pop(S));
			c0v_push(S, int2val(x | y));
			break;
		}

    case IXOR: {
			pc++;
			int32_t y = val2int(c0v_pop(S));
			int32_t x = val2int(c0v_pop(S));
			c0v_push(S, int2val(x ^ y));
			break;
		}

    case ISHR: {
			pc++;
			int32_t y = val2int(c0v_pop(S));
			int32_t x = val2int(c0v_pop(S));
			if (y < 0 || y >= 32) {
				c0_arith_error("Invalid shift range");
				// I don't free here because exit() will terminate 
				// and the system will reclaim
				exit(EXIT_FAILURE);
			}
			c0v_push(S, int2val(x >> y));
			break;
		}

    case ISHL: {
			pc++;
			int32_t y = val2int(c0v_pop(S));
			int32_t x = val2int(c0v_pop(S));
			if (y < 0 || y >= 32) {
				c0_arith_error("Invalid shift range");
				// I don't free here because exit() will terminate 
				// and the system will reclaim
				exit(EXIT_FAILURE);
			}
			c0v_push(S, int2val(x << y));
			break;
		}

    /* Pushing constants */

    case BIPUSH:

    case ILDC:

    case ALDC:

    case ACONST_NULL:


    /* Operations on local variables */

    case VLOAD:

    case VSTORE:


    /* Assertions and errors */

    case ATHROW:

    case ASSERT:


    /* Control flow operations */

    case NOP:

    case IF_CMPEQ:

    case IF_CMPNE:

    case IF_ICMPLT:

    case IF_ICMPGE:

    case IF_ICMPGT:

    case IF_ICMPLE:

    case GOTO:


    /* Function call operations: */

    case INVOKESTATIC:

    case INVOKENATIVE:


    /* Memory allocation and access operations: */

    case NEW:

    case IMLOAD:

    case IMSTORE:

    case AMLOAD:

    case AMSTORE:

    case CMLOAD:

    case CMSTORE:

    case AADDF:


    /* Array operations: */

    case NEWARRAY:

    case ARRAYLENGTH:

    case AADDS:


    /* BONUS -- C1 operations */

    case CHECKTAG:

    case HASTAG:

    case ADDTAG:

    case ADDROF_STATIC:

    case ADDROF_NATIVE:

    case INVOKEDYNAMIC:

    default:
      fprintf(stderr, "invalid opcode: 0x%02x\n", P[pc]);
      abort();
    }
  }

  /* cannot get here from infinite loop */
  assert(false);
}
