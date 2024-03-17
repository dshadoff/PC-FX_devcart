/*
 *  This is an example of what needs to be added to your
 *  'C' program in order to take advantage of the FX devcart
 *  during program execution
 *
 *   Copyright (C) 2024 David Shadoff
 */


/* 
 * Make sure that this is included, because access to the
 * backup memory needs to be enabled in order to use the
 * devcart and its FIFO output
 */

#include <eris/bkupmem.h>


/* 
 * You will likely want to have a string output function,
 * something like the implementation below:
 */

void fifo_write(uint8_t out_data)
{
   while ((read_fifo_ctrl() & FIFO_CTRL_DO_NOT_WRITE) != 0);
   write_fifo_data(out_data);
}

void out_string_to_fifo(char* str)
{
   int i;

   for (i = 0; i < strlen8(str); i++) {
      fifo_write((uint8_t) str[i]);
   }
}

/* 
 * At some point in your program, you will use these function
 * like this...
 */

   out_string_to_fifo("Test string\n");



/* 
 * Also, in order to ensure that reads/writes to the FIFO port
 * actually work, access to the external memory will need to be
 * enabled, such as by doing this:
 */

void init(void)
{
	eris_bkupmem_set_access(1,1);
}

