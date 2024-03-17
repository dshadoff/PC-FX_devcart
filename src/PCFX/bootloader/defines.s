
/*============================================================= */
.equiv VEC_IRQ_VBLA,  0x7fcc
/* ============================================================= */
.equiv JOY_UP,	  (1<< 8)
.equiv JOY_RIGHT, (1<< 9)
.equiv JOY_DOWN,  (1<<10)
.equiv JOY_LEFT,  (1<<11)
.equiv JOY_B1,    (1<< 0)

/* IO ADRESSES */

.equiv HuC6261_reg, 0x300
.equiv HuC6261_dat, 0x304

.equiv SUPA_reg, 0x400
.equiv SUPA_dat, 0x404

.equiv SUPB_reg, 0x500
.equiv SUPB_dat, 0x504

.equiv KING_reg, 0x600
.equiv KING_dat, 0x604
.equiv KING_dat2, 0x606

/* register OFFSETS */

.equiv HuC6261_SCREENMODE, 0x00
.equiv HuC6261_PAL_NR, 0x01
.equiv HuC6261_PAL_DATA, 0x02
.equiv HuC6261_PAL_READ, 0x03 
.equiv HuC6261_PAL_7UP_OFF, 0x4
.equiv HuC6261_PAL_KING_01, 0x5 
.equiv HuC6261_PAL_KING_23, 0x6 
.equiv HuC6261_PAL_RAINBOW, 0x7 
.equiv HuC6261_PRIO_0, 0x8
.equiv HuC6261_PRIO_1, 0x9 
.equiv HuC6261_COLKEY_Y, 0xa
.equiv HuC6261_COLKEY_U, 0xb
.equiv HuC6261_COLKEY_V, 0xc
.equiv HuC6261_COLCELL, 0xd
.equiv HuC6261_CTRL_CELL, 0xe
.equiv HuC6261_CELL_SPRBANK, 0x0f 
.equiv HuC6261_CELL_1A, 0x10 
.equiv HuC6261_CELL_1B, 0x11
.equiv HuC6261_CELL_2A, 0x12
.equiv HuC6261_CELL_2B, 0x13
.equiv HuC6261_CELL_3A, 0x14
.equiv HuC6261_CELL_3B, 0x15

.equiv KING_KRAM_ADR_read, 0x0c
.equiv KING_KRAM_ADR_write, 0x0d
.equiv KING_KRAM_rw, 0x0e
.equiv KING_KRAM_page, 0x0f

.equiv KING_BG_MODE, 0x10
.equiv KING_BG_PRIO, 0x12
.equiv KING_MICRO_ADR, 0x13
.equiv KING_MICRO_DATA, 0x14
.equiv KING_MICRO_CTRL, 0x15

.equiv KING_BG_SCROLL, 0x16

.equiv KING_BG0_BAT,    0x20
.equiv KING_BG0_CG,     0x21
.equiv KING_BG0_BATsub, 0x22
.equiv KING_BG0_CGsub,  0x23

.equiv KING_BG1_BAT,    0x24
.equiv KING_BG1_CG,     0x25
#26,27 GAP
.equiv KING_BG2_BAT,    0x28
.equiv KING_BG2_CG,     0x29

.equiv KING_BG3_BAT,    0x2a
.equiv KING_BG3_CG,     0x2b

.equiv KING_BG0_size,   0x2c
.equiv KING_BG1_size,   0x2d
.equiv KING_BG2_size,   0x2e
.equiv KING_BG3_size,   0x2f

.equiv KING_BG0_X,   0x30
.equiv KING_BG0_Y,   0x31

.equiv KING_BG1_X,   0x32
.equiv KING_BG1_Y,   0x33

.equiv KING_BG2_X,   0x34
.equiv KING_BG2_Y,   0x35

.equiv KING_BG3_X,   0x36
.equiv KING_BG3_Y,   0x37

.equiv KING_BG_aff_A, 0x38
.equiv KING_BG_aff_B, 0x39
.equiv KING_BG_aff_C, 0x3a
.equiv KING_BG_aff_D, 0x3b
.equiv KING_BG_aff_centerX, 0x3c
.equiv KING_BG_aff_centerY, 0x3d

.equiv SUP_VRAM_ADR_W, 0x00
.equiv SUP_VRAM_ADR_R, 0x01
.equiv SUP_VRAM_RW, 0x02
.equiv SUP_CTRL, 0x05
.equiv SUP_RASTER, 0x06
.equiv SUP_BG_X, 0x7
.equiv SUP_BG_Y, 0x8
.equiv SUP_MEMWIDTH, 0x09
.equiv SUP_HSYNC, 0x0a
.equiv SUP_HDISP, 0x0b
.equiv SUP_VSYNC, 0x0c
.equiv SUP_VDISP, 0x0d
.equiv SUP_VDISPEND, 0x0e
.equiv SUP_DMA_CTRL, 0x0f
.equiv SUP_DMA_SRC, 0x10
.equiv SUP_DMA_DST, 0x11
.equiv SUP_DMA_LEN, 0x12
.equiv SUP_SAT_ADR, 0x13



/* BIT FLAGS */

.equiv HuC6261_line262, (0x01 << 0)
.equiv HuC6261_intsync, ( 0x0 << 2) /* int or ext? */
.equiv HuC6261_320px,   ( 0x1 << 3)
.equiv HuC6261_256px,   ( 0x0 << 3)
.equiv HuC6261_bg16,    ( 0x0 << 6)
.equiv HuC6261_bg256,   ( 0x1 << 6)
.equiv HuC6261_spr16,   ( 0x0 << 7)
.equiv HuC6261_spr256,  ( 0x1 << 7)



.equiv HuC6261_7upBG_on,  ( 0x1 << 8)
.equiv HuC6261_7upSPR_on, ( 0x1 << 9)
.equiv HuC6261_KingBG0_on,( 0x1 << 10)
.equiv HuC6261_KingBG1_on,( 0x1 << 11)
.equiv HuC6261_KingBG2_on,( 0x1 << 12)
.equiv HuC6261_KingBG3_on,( 0x1 << 13)
.equiv HuC6261_Rainbow_on,( 0x1 << 14)

.equiv KING_mode_4,     0b0001
.equiv KING_mode_16,    0b0010
.equiv KING_mode_256,   0b0011
.equiv KING_mode_64k,   0b0100
.equiv KING_mode_16m,   0b0101
.equiv KING_mode_4block,  0b1001
.equiv KING_mode_16block, 0b1010
.equiv KING_mode_256block,0b1011

.equiv KING_prio_hidden,     0b000
.equiv KING_prio_last,       0b001
.equiv KING_prio_abovelast,  0b010
.equiv KING_prio_underfirst, 0b011
.equiv KING_prio_first,      0b100

.equiv KING_size_8,     0b0011
.equiv KING_size_16,    0b0100
.equiv KING_size_32,    0b0101
.equiv KING_size_64,    0b0110
.equiv KING_size_128,   0b0111
.equiv KING_size_256,   0b1000
.equiv KING_size_512,   0b1001
.equiv KING_size_1024,  0b1010 /*only bg0*/

.equiv KING_b_height,    0
.equiv KING_b_width,     4
.equiv KING_b_subheight, 8
.equiv KING_b_subwidth, 12
.equiv KING_b_inc, 18 /*for read/write*/

.equiv KING_bg0_rotation,    (1<<12)

.equiv SUP_SPR_on, (1<<6)
.equiv SUP_BG_on, (1<<7)
