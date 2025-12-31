/**************************************************************************/
/*              COPYRIGHT Carnegie Mellon University 2023                 */
/* Do not post this file or any derivative on a public site or repository */
/**************************************************************************/

#include "c0vm.h"

#ifndef H_READ_PROGRAM
#define H_READ_PROGRAM

struct bc0_file* read_program(string filename);
void free_program(struct bc0_file *program);

#endif // H_READ_PROGRAM
