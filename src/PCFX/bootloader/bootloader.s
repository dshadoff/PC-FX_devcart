#(c) 2024 David Shadoff, Martin Wendt

.include "macros.s"
.include "defines.s"

#===============================
.equiv r_register, r6    
.equiv r_continue,  r7
.equiv r_ptr,      r10
.equiv r_tmp,      r14
.equiv r_source,   r15
.equiv r_target,   r16
.equiv r_count,    r17
.equiv r_tmp_loop, r20
.equiv r_tmp_adr,  r21
.equiv r_tmp_data, r22


.equiv bmp_status, 0xE800004E
.equiv bmp_offset, 0xE8000060
.equiv bmp_target, 0xE8000068
.equiv bmp_count,  0xE8000070
.equiv bmp_exec,   0xE8000078

.equiv fifo_ctrl,  0xE8040002
.equiv fifo_data,  0xE8040004
#===============================



.globl _start
.org = 0x8000
_start = 0x8000
.equiv relocate_to, 0x1FF000
Start:

    #disable, clear, enable cache
    #hint from Elmer: CD-DMA during boot may have invalidated it!
    ldsr    r0,chcw
    movea   0x8001,r0,r1
    ldsr    r1,chcw
    mov     2,r1
    ldsr    r1,chcw
    
relocate_and_launch_full_client:
    movw client_code, r10
    movw relocate_to, r11
    movea (client_code_end - client_code),r0, r12

1:
    ld.w 0[r10],r13
    st.w r13,0[r11]
    add 4, r10
    add 4, r11
    add -4, r12
    bp 1b
    
    movw relocate_to, r14
    jmp [r14]
#====================================



.align 2
client_code:
# unlock BRAM
    mov 3, r_tmp           # unlock backup RAM and external backup RAM
    out.h r_tmp, 0xc80[r0] # port for access control

readyloop:
    movw    bmp_status, r_tmp_adr
    ld.b    0[r_tmp_adr], r_continue
    cmp     r_continue, r0
    bz      readyloop

    movw    bmp_offset, r_ptr
    call    readbramword
    shl     1, r_tmp

    movw    0xe8000000, r_source
    add     r_tmp, r_source

    movw    bmp_target, r_ptr
    call    readbramword
    mov     r_tmp, r_target

    movw    bmp_count, r_ptr
    call    readbramword
    mov     r_tmp, r_count

copyloop:
    ld.b    0[r_source], r_tmp
    st.b    r_tmp, 0[r_target]
    add     1, r_target
    add     -1, r_count
    be      endloop

    ld.b    2[r_source], r_tmp
    st.b    r_tmp, 0[r_target]
    add     4, r_source
    add     1, r_target
    add     -1, r_count
    bne     copyloop

endloop:
    movw    bmp_status, r_tmp   # reset status to "awaiting write completion"
    st.b    r0, 0[r_tmp]

    movw    '2', r_tmp
    cmp     r_tmp, r_continue
    be      exit

    movw    fifo_data, r_tmp    # send "request next block" to PC
    st.b    r_continue, 0[r_tmp]
    br      readyloop

exit:
    movw    bmp_exec, r_ptr
    call    readbramword
    jmp     [r_tmp]



#
# readbramword
# Read a word from BRAM (not straightforwad since BRAM is every second byte
#
# inputs:  r_ptr = address (i.e. 0xE8000060)
# outputs: r_tmp = reconstructed word of data at that address
#

.align 2
readbramword:
    add     -4, sp         # establish stack frame for building one word

    ld.b 0[r_ptr], r_tmp
    st.b r_tmp, 0[sp]
    ld.b 2[r_ptr], r_tmp
    st.b r_tmp, 1[sp]
    ld.b 4[r_ptr], r_tmp
    st.b r_tmp, 2[sp]
    ld.b 6[r_ptr], r_tmp
    st.b r_tmp, 3[sp]

    ld.w 0[sp], r_tmp
    add     -4, sp

    ret
	
.hword 0x55aa
.hword 0x77bb
.hword 0x55aa
.hword 0x77bb

client_code_end:
#===================================
.align 2
.endofprogram:
#==================================================
		
