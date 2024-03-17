
.macro movw   data, reg1
       movhi  hi(\data),r0,\reg1
       movea  lo(\data),\reg1,\reg1
.endm

       .global _read_fifo_ctrl
       .global _read_fifo_data
       .global _write_fifo_data
       .global _read_fxbmpmem_offset


.equiv r_tmp,   r8
.equiv r_base1, r10

/*********************************/
/* Remember calling conventions: */
/* ----------------------------- */
/* r6  is entry parameter #1     */
/* r10 is return value           */
/* lp  is return address         */
/*********************************/


/***********************************/
/* extern u8 read_fifo_ctrl(void); */
/***********************************/
_read_fifo_ctrl:
       mov   lp, r18
       movw  0xe8040000, r_base1
       ld.b  2[r_base1], r10
       mov   r18, lp
       jmp   [lp]


/***********************************/
/* extern u8 read_fifo_data(void); */
/***********************************/
_read_fifo_data:
       mov   lp, r18
       movw  0xe8040004, r_base1
       ld.b  0[r_base1], r10
       mov   r18, lp
       jmp   [lp]


/*********************************************/
/* extern void write_fifo_data(u8 databyte); */
/*********************************************/
_write_fifo_data:
       mov   lp, r18
       movw  0xe8040004, r_base1
       st.b  r6,0[r_base1]
       mov   r18, lp
       jmp   [lp]


/************************************************/
/* extern u8 read_fxbmpmem_offset(u32 address); */
/************************************************/
_read_fxbmpmem_offset:
       mov   lp, r18
       shl   1, r6
       movw  0xe8000000, r_base1
       add   r6, r_base1
       ld.b  0[r_base1], r10
       mov   r18, lp
       jmp   [lp]
