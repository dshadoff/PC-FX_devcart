
.macro  push reg1
        add     -4, sp
        st.w    \reg1, 0x0[sp]
.endm

.macro  pop reg1
        
        ld.w    0x0[sp], \reg1
        add     4, sp
.endm

.macro  movw data, reg1
        movhi   hi(\data),r0,\reg1
        movea   lo(\data),\reg1,\reg1
        
        /* could be made smarter to omit if not required */
.endm

.macro  call target
        push r31
        jal \target
        pop r31
.endm

.macro  ret
        jmp [r31]
.endm

.macro  jump target
        movw    \target, r30
        jmp     [r30]
.endm
