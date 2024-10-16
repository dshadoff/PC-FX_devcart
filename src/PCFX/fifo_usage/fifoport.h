/*
 *  fifoport.h
 *
 *  Access to routines for the FIFO port
 *
 *  Copyright (C) 2024 David Shadoff
 */

// FIFO port control bits:
//
#define FIFO_CTRL_DO_NOT_WRITE  0x80
#define FIFO_CTRL_DO_NOT_READ   0x40


extern u8 read_fifo_ctrl(void);
extern u8 read_fifo_data(void);
extern void write_fifo_data(u8 databyte);
extern u8 read_fxbmpmem_offset(u32 address);

extern u8 get_devcart_ver_major(void);
extern u8 get_devcart_ver_minor(void);

