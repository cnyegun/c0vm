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
  c0_value *V;     /* The local variables */
  size_t pc;       /* Program counter */
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
  gstack_t callStack = stack_new();

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
			// Pop the last value from the stack
			c0_value retval = c0v_pop(S);

      // Free everything before returning from the execute function!
			c0v_stack_free(S);
			free(V);

			// Resume the last frame if exist
			if (!stack_empty(callStack)) {
				frame *prev_frame = (frame *) pop(callStack);
				S = prev_frame->S;
				P = prev_frame->P;
				pc = prev_frame->pc + 3;
				V = prev_frame->V;
				free(prev_frame);
				c0v_push(S, retval);
				break;
			}
			stack_free(callStack, free);
			return val2int(retval);
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
				c0_arith_error("INT32_MIN % -1");
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
			}
			c0v_push(S, int2val(x << y));
			break;
		}

    /* Pushing constants */

    case BIPUSH: {
			int32_t b = (int32_t) (byte) P[pc + 1];
			pc += 2;
			c0v_push(S, int2val(b));
			break;
		}

    case ILDC: {
			// Read int32_t v from int_pool and push to the op stack
			uint16_t c1 = (uint16_t) P[pc + 1];
			uint16_t c2 = (uint16_t) P[pc + 2];
			int32_t x = bc0->int_pool[(c1<<8)|c2];
			c0v_push(S, int2val(x));
			pc += 3;
			break;
		}

    case ALDC: {
			// Read address a from &string_pool and push it to the op stack
			uint16_t c1 = (uint16_t) P[pc + 1];
			uint16_t c2 = (uint16_t) P[pc + 2];
			char *a = &(bc0->string_pool[(c1<<8)|c2]);
			c0v_push(S, ptr2val(a));
			pc += 3;
			break;
		}

    case ACONST_NULL: {
			pc++;
			c0v_push(S, ptr2val(NULL));
			break;
		}


    /* Operations on local variables */

    case VLOAD: {
			ubyte i = P[pc + 1];
			pc += 2;
			c0v_push(S, V[i]);
			break;
		}

    case VSTORE: {
			ubyte i = P[pc + 1];
			pc += 2;
			c0_value v = c0v_pop(S);
			V[i] = v;
			break;
		}

    /* Assertions and errors */

    case ATHROW: {
			pc++;
			char *a = (char*) val2ptr(c0v_pop(S));
			c0_user_error(a);
			break;
		}

    case ASSERT: {
			pc++;
			char *a = (char*) val2ptr(c0v_pop(S));
			int32_t x = (int32_t) val2int(c0v_pop(S));
			if (x == 0)
				c0_assertion_failure(a);
			break;
		}

    /* Control flow operations */

    case NOP: {
			pc++;
			break;
		}

    case IF_CMPEQ: {
			// Pop two value from the stack and compare, if true => modify pc
			uint16_t o1 = P[pc + 1];
			uint16_t o2 = P[pc + 2];
			int16_t offset = (int16_t) (o1 << 8 | o2);

			c0_value v2 = c0v_pop(S);
			c0_value v1 = c0v_pop(S);

			if (val_equal(v1, v2)) pc += offset;
			else pc += 3;
			break;
		}

    case IF_CMPNE: {
			// Pop two value from the stack and compare, if false => modify pc
			uint16_t o1 = P[pc + 1];
			uint16_t o2 = P[pc + 2];
			int16_t offset = (int16_t) (o1 << 8 | o2);

			c0_value v2 = c0v_pop(S);
			c0_value v1 = c0v_pop(S);

			if (!val_equal(v1, v2)) pc += offset;
			else pc += 3;
			break;
		}


    case IF_ICMPLT: {
			uint16_t o1 = P[pc + 1];
			uint16_t o2 = P[pc + 2];
			int16_t offset = (int16_t) (o1 << 8 | o2);

			int32_t y = val2int(c0v_pop(S));
			int32_t x = val2int(c0v_pop(S));

			if (x < y) pc += offset;
			else pc += 3;
			break;
		}


    case IF_ICMPGE: {
			uint16_t o1 = P[pc + 1];
			uint16_t o2 = P[pc + 2];
			int16_t offset = (int16_t) (o1 << 8 | o2);

			int32_t y = val2int(c0v_pop(S));
			int32_t x = val2int(c0v_pop(S));

			if (x >= y) pc += offset;
			else pc += 3;
			break;
		}


    case IF_ICMPGT: {
			uint16_t o1 = P[pc + 1];
			uint16_t o2 = P[pc + 2];
			int16_t offset = (int16_t) (o1 << 8 | o2);

			int32_t y = val2int(c0v_pop(S));
			int32_t x = val2int(c0v_pop(S));

			if (x > y) pc += offset;
			else pc += 3;
			break;
		}


    case IF_ICMPLE: {
			uint16_t o1 = P[pc + 1];
			uint16_t o2 = P[pc + 2];
			int16_t offset = (int16_t) (o1 << 8 | o2);

			int32_t y = val2int(c0v_pop(S));
			int32_t x = val2int(c0v_pop(S));

			if (x <= y) pc += offset;
			else pc += 3;
			break;
		}


    case GOTO: {
			uint16_t o1 = P[pc + 1];
			uint16_t o2 = P[pc + 2];
			int16_t offset = (int16_t) (o1 << 8 | o2);

			pc += offset;
			break;
		}


    /* Function call operations: */

    case INVOKESTATIC: {
			uint16_t o1 = P[pc + 1];
			uint16_t o2 = P[pc + 2];
			uint16_t fn_idx = (o1 << 8) | o2;
			struct function_info fn = bc0->function_pool[fn_idx];

			// Create a new frame of current execution environment
			frame *f = xmalloc(sizeof(frame));
			f->S = S;
			f->P = P;
			f->pc = pc;
			f->V = V;
			push(callStack, f);

			// Set pc to the beginning of the function
			V = xcalloc(fn.num_vars, sizeof *V);

			for (int i = fn.num_args - 1; i >= 0; i--) {
				V[i] = c0v_pop(S);
			}

			S = c0v_stack_new();
			P = fn.code;
			pc = 0;
			break;
		}

    case INVOKENATIVE: {
			uint16_t o1 = P[pc + 1];
			uint16_t o2 = P[pc + 2];
			uint16_t fn_idx = (o1 << 8) | o2;
			struct native_info native = bc0->native_pool[fn_idx];

			c0_value *args = xcalloc(native.num_args, sizeof *args);
			native_fn *fn = native_function_table[native.function_table_index];

			for (int i = native.num_args - 1; i >= 0; i--) {
				args[i] = c0v_pop(S);
			}

			c0_value v = (*fn) (args);
			c0v_push(S, v);
			pc += 3;
			free(args);
			break;
		}



    /* Memory allocation and access operations: */

    case NEW: {
			ubyte s = P[pc + 1];
			pc += 2;
			void *p = xmalloc(s);
			c0v_push(S, ptr2val(p));
			break;
		}

    case IMLOAD: {
			// Pop an address from the stack
			// Read 4 bytes value from that memory address
			// Pop that result back to the stack.
			uint32_t *a = val2ptr(c0v_pop(S));
			if (a == NULL) c0_memory_error("NULL dereference");
			uint32_t x = *a;
			c0v_push(S, int2val(x));
			pc++;
			break;
		}

    case IMSTORE: {
			// Pop an int from the stack
			// Pop another address
			// Store the int to the address
			uint32_t x = val2int(c0v_pop(S));
			uint32_t *a = val2ptr(c0v_pop(S));
			if (a == NULL) c0_memory_error("NULL dereference");
			*a = x;
			pc++;
			break;
		}

    case AMLOAD: {
			// Pop an address from stack
			// Read the address from that
			// Pop result back to the stack
			uint32_t **a = val2ptr(c0v_pop(S));
			if (a == NULL) c0_memory_error("NULL deference");
			uint32_t *b = *a;
			c0v_push(S, ptr2val(b));
			pc++;
			break;
		}

    case AMSTORE: {
			// Pop an address b from the stack
			// Pop an pointer a from the stack
			uint32_t *b = val2ptr(c0v_pop(S));
			uint32_t **a = val2ptr(c0v_pop(S));
			if (a == NULL) c0_memory_error("NULL deference");
			*a = b;
			pc++;
			break;
		}

    case CMLOAD: {
			// Pop an address from the stack
			uint32_t *a = val2ptr(c0v_pop(S));
			if (a == NULL) c0_memory_error("NULL deference");
			uint32_t x = (uint32_t) (*a);
			c0v_push(S, int2val(x));
			pc++;
			break;
		}

    case CMSTORE: {
			// Pop an int from the stack
			uint32_t x = val2int(c0v_pop(S));	
			uint32_t *a = val2ptr(c0v_pop(S));
			if (a == NULL) c0_memory_error("NULL deference");
			*a = x & 0x7f;
			pc++;
			break;
		}

    case AADDF: {
			ubyte f = P[pc + 1];
			void *a = val2ptr(c0v_pop(S));
			if (a == NULL) c0_memory_error("NULL deference");
			void *p = (void *) ((ubyte *)a + f);
			c0v_push(S, ptr2val(p));
			pc += 2;
			break;
		}

    /* Array operations: */

    case NEWARRAY: {
			int32_t n = val2int(c0v_pop(S));
			if (n < 0) c0_memory_error("array size cannot be negative");
			size_t s = P[pc + 1];
			// Alloc the array struct
			c0_array *arr = (c0_array *) xmalloc(sizeof *arr);
			arr->count = n;
			arr->elt_size = s;
			arr->elems = xcalloc(n, s);
			c0v_push(S, ptr2val(arr));
			pc += 2;
			break;
		}

    case ARRAYLENGTH: {
			c0_array *arr = (c0_array *) val2ptr(c0v_pop(S));
			if (arr == NULL) c0_memory_error("NULL ptr reference");
			uint32_t n = arr->count;
			c0v_push(S, int2val(n));
			pc++;
			break;
		}

    case AADDS: {
			int32_t index = val2int(c0v_pop(S));
			if (index < 0) c0_memory_error("invalid index");
			c0_array *a = (c0_array *) val2ptr(c0v_pop(S));
			if (a == NULL) c0_memory_error("NULL dereference");
			uint8_t *base = (uint8_t *) a->elems;
			void *p = base + (size_t)a->elt_size * (size_t)index;
			c0v_push(S, ptr2val(p));
			pc++;
			break;
		}


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
