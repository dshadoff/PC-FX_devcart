/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/uart.h"

// These "extern inline" definitions are strong hints to
// actually inline these functions (which would otherwise
// be left up to the compiler to decide)
//
extern inline uint32_t gpio_get_all(void) __attribute__((always_inline));
extern inline void gpio_put_masked(uint32_t mask, uint32_t value) __attribute__((always_inline));
extern inline void gpio_set_dir_masked(uint32_t mask, uint32_t value) __attribute__((always_inline));

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+


#define UART_ID uart0
#define BAUD_RATE 115200

// Use USB UART
//
//  We are using pins 0 and 1, but see the GPIO function select table in the
//  datasheet for information on which other pins can be used.
// #define UART_TX_PIN 0
// #define UART_RX_PIN 1


//
// USB COM port commmand definitions:
//
#define  CMD_DUMP      '?'    // dump 256 bytes of hex
#define  CMD_DUMPALL   '!'    // dump 16384+16 bytes of hex
#define  CMD_GETFIFO   '/'    // read FIFO

#define  CMD_BOOT             'B'    // Boot program (B is followed by 4 bytes size, then program)
#define  CMD_BOOT_BLOCK_CONT  '1'    // Boot program load a block (add prev length to start; confirm
                                     // with '1' to indicate continuation)
                                     // '1' is followed by 4 bytes size, then program
#define  CMD_BOOT_BLOCK_FINAL '2'    // Boot program load final block (add prev length to start;
                                     // confirm with '2' to indicate completion)
                                     // '2' is followed by 4 bytes size, then program
#define  CMD_WRITE128K        'W'    // write all 128KB of memory (128KB of data follows)
#define  CMD_READ128K         'R'    // read all 128KB of memory (128KB of data is returned)


//
// hardware-level definitions:
//
#define DATA0_PIN      0      // use GPIO 0-7  for data
#define ADDR0_PIN      8      // use GPIO 8-25 for address
#define CE_PIN        26      // use GPIO 26 for Chip Enable
#define OE_PIN        27      // use GPIO 27 for Output Enable
#define WE_PIN        28      // use GPIO 28 for Write Enable



// #define GPIO_TESTSIZE   // use GPIO_FULLSIZE when all GPIOs are available and connected
//#define GPIO_FULLSIZE
//
//#ifdef GPIO_FULLSIZE

#define ARRAY_SIZE        131072
#define GPIO_MASK_ADDR    0x03FFFF00 // ultimately, 128KB + port space
#define GPIO_ADDR_THRESH  0x00020000 // ultimately, port space threshold
#define GPIO_PORT_DATA    0x00020002
#define GPIO_PORT_CONTROL 0x00020001

//#else                   // else, these are the TESTSIZE variables
//
//#define ARRAY_SIZE        32768
//#define GPIO_MASK_ADDR    0x007FFF00 // currently, 16KB + port space
//#define GPIO_ADDR_THRESH  0x00004000 // currently, port space threshold
//#define GPIO_PORT_DATA    0x00004002
//#define GPIO_PORT_CONTROL 0x00004001
//
//#endif

#define GPIO_ADDR_SHIFT   8              // since A0 = GPIO8, this is the shift amount

#define GPIO_MASK_DATA    0x000000FF     // bottom GPIOs (GPIO0-7) used for data

#define VALUE_INV_ADDR    0xFF           // 'floating' lines. Should be 0xFF, but 0xAA during test.


// PC-FX Code Boot particulars:
//
#define CODE_BMP_OFFSET      0x0400
#define FX_EXECUTION_ADDR    0x8000

#define FX_CONFIRMATION      0x0027      // address to confirm that data write is complete (and/or final)
#define FX_BOOT_OFFSET       0x0030      // Offset from start of cartridge memory (data to transfer to RAM)
#define FX_BOOT_LOADADDR     0x0034      // PCFX main memory location to transfer the memory into
#define FX_BOOT_NUMBYTES     0x0038      // Number of bytes to copy to target memory location
#define FX_BOOT_EXEC         0x003C      // After load completes, this holds the execution addr

#define FX_WRITE_DONE        '1'         // confirm memory write complete; more to come
#define FX_WRITE_FINISHED    '2'         // confirm memory write complete; no more to come


// PC-FX control port flag bits:
#define  FIFO_CTRL_DO_NOT_WRITE  0x80    // bit 7 = HIGH when queue full (i.e. do not write)
#define  FIFO_CTRL_DO_NOT_READ   0x40    // bit 6 = HIGH when queue empty (i.e. do not read)

#define FIFO_SIZE        1024
#define FIFO_SIZE_MASK   0x3FF



// Memory array variables:
//
volatile uint8_t  mem_array[ARRAY_SIZE];


// FIFO port variables:
//
volatile uint8_t  fifo_from_pcfx[FIFO_SIZE];
volatile uint8_t  fifo_to_pcfx[FIFO_SIZE];

volatile int      from_pcfx_tail = 0; // outbound data to PC
volatile int      from_pcfx_head = 0;

volatile int      to_pcfx_tail = 0;   // tail points at location to read next data from
volatile int      to_pcfx_head = 0;   // head points at location to write next data to

volatile uint8_t  control  = FIFO_CTRL_DO_NOT_READ; // flags to tell PCFX whether not to read/write
volatile uint8_t  dataport = VALUE_INV_ADDR;        // pre-fetched data for fifo_to_pcfx[to_pcfx_tail]

// Maybe making these into 'int' type makes them faster ?
//
//volatile int  control  = FIFO_CTRL_DO_NOT_READ; // flags to tell PCFX whether not to read/write
//volatile int  dataport = VALUE_INV_ADDR;        // pre-fetched data for fifo_to_pcfx[to_pcfx_tail]

// debug stuff commented out
//
//volatile int      writes_to_port = 0;
//volatile int      writes_not_to_port = 0;

//volatile uint32_t write_addr[128];
//volatile int write_addr_index = 0;


// default format for a (formatted and empty) 128KB memory card
//
uint8_t formatted_bmp_00[] = {
        0x24, 0x8A, 0xDF, 0x50,  'P',  'C',  'F',  'X',   'C',  'a',  'r',  'd', 0x00, 0x01, 0x01, 0x00,
        0x01, 0xFC, 0x00, 0x00, 0x04, 0xF9, 0x0C, 0x00,  0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t formatted_bmp_80[] = {
        0xF9, 0xFF, 0xFF
};


// Next line is only used for booting from Cart:
uint8_t boot_bmp_20[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   'P',  'C',  'F',  'X',  'B',  'o',  'o',  't'

        // Next 16 bytes (at offset 0x0030) are 4-byte words of:
        //    source offset    (0x0400)
        //    RAM Destination  (0x8000)
        //    Transfer Size    (variable)
        //    Transfer Address (0x8000)
};




void __not_in_flash_func(to_pcfx_add_byte)(uint8_t indata)
{
int size;

   size = ((to_pcfx_head + FIFO_SIZE - to_pcfx_tail) & FIFO_SIZE_MASK);

   if (size < FIFO_SIZE_MASK) {
      fifo_to_pcfx[to_pcfx_head] = indata;
      to_pcfx_head = ((to_pcfx_head + 1) & FIFO_SIZE_MASK);
   }

   dataport = fifo_to_pcfx[to_pcfx_tail];
   control &= ~FIFO_CTRL_DO_NOT_READ;
}

uint8_t __not_in_flash_func(to_pcfx_consume_byte)(void)
{
uint8_t data;

   if (to_pcfx_tail == to_pcfx_head) {    // empty FIFO
      data = VALUE_INV_ADDR;
      control |= FIFO_CTRL_DO_NOT_READ;
   }
   else {
      data = fifo_to_pcfx[to_pcfx_tail];
      to_pcfx_tail = ((to_pcfx_tail + 1) & FIFO_SIZE_MASK);

      if (to_pcfx_tail == to_pcfx_head)   // did we drain the FIFO completely ?
      {                                   // pre-setup next values
         dataport = VALUE_INV_ADDR;
         control |= FIFO_CTRL_DO_NOT_READ;
      } else {
         dataport = fifo_to_pcfx[to_pcfx_tail];
         control &= ~FIFO_CTRL_DO_NOT_READ;
      }
   }
   return(data);
}


void __not_in_flash_func(from_pcfx_add_byte)(uint8_t indata)
{
int size;

   size = ((from_pcfx_head + 1024 - from_pcfx_tail) & FIFO_SIZE_MASK);

   if (size < FIFO_SIZE_MASK) {
      fifo_from_pcfx[from_pcfx_head] = indata;
      from_pcfx_head = ((from_pcfx_head + 1) & FIFO_SIZE_MASK);
   }

   if ((size + 1) == FIFO_SIZE_MASK)
      control |= FIFO_CTRL_DO_NOT_WRITE;
   else
      control &= ~FIFO_CTRL_DO_NOT_WRITE;
}

uint8_t __not_in_flash_func(from_pcfx_consume_byte)(void)
{
uint8_t data;

   if (from_pcfx_tail == from_pcfx_head) {
      data = VALUE_INV_ADDR;
   }
   else {
      data = fifo_from_pcfx[from_pcfx_tail];
      from_pcfx_tail = ((from_pcfx_tail + 1) & FIFO_SIZE_MASK);
   }
   control &= ~FIFO_CTRL_DO_NOT_WRITE;
   return(data);
}

void __not_in_flash_func(out_str)(char * string)
{
   for (int i = 0; i < strlen(string); i++) {
      putchar_raw( *(string+i) );
   }
   putchar_raw(0x0D);
   putchar_raw(0x0A);
}

//
// Deal with bus access to Pseudo-RAM
//
static void __not_in_flash_func(core1_entry)(void)
{
//char buf[32];
uint8_t indata;
uint32_t bus_address;

    while (1)
    {
       while (!gpio_get(CE_PIN))
       {
          if (!gpio_get(OE_PIN))   // bus reads from memory
          {
             bus_address = ((gpio_get_all() & GPIO_MASK_ADDR) >> GPIO_ADDR_SHIFT);

             if (bus_address < GPIO_ADDR_THRESH) {
                gpio_put_masked(GPIO_MASK_DATA, mem_array[bus_address]);
             }
             else switch(bus_address)
             {
                case GPIO_PORT_DATA:
                   gpio_put_masked(GPIO_MASK_DATA, dataport);
                   break;

                case GPIO_PORT_CONTROL:
                   gpio_put_masked(GPIO_MASK_DATA, control);
                   break;

                default:
                   gpio_put_masked(GPIO_MASK_DATA, VALUE_INV_ADDR);
                   break;
             }
             gpio_set_dir_masked(GPIO_MASK_DATA, 0xFF);
             while (!gpio_get(OE_PIN) );
             gpio_set_dir_masked(GPIO_MASK_DATA, 0x00);

             if (bus_address == GPIO_PORT_DATA) {
                indata = to_pcfx_consume_byte();  // throwaway
             }
          }

          if (!gpio_get(WE_PIN))   // bus write to memory
          {
             bus_address = ((gpio_get_all() & GPIO_MASK_ADDR) >> GPIO_ADDR_SHIFT);

             // Read again, as data is not guaranteed to be correct until
             // slightly after the /WE pin goes low.
             //
             indata = (gpio_get_all() & GPIO_MASK_DATA);

// debug logging
//             write_addr[write_addr_index] = bus_address;
//             write_addr_index = write_addr_index + 1;

             if (bus_address < GPIO_ADDR_THRESH) {
                mem_array[bus_address] = indata;
             }
             else switch(bus_address) {
                case GPIO_PORT_DATA:
                   from_pcfx_add_byte(indata);
//                   writes_to_port++;
                   break;

                default:
//                   writes_not_to_port++;
                   break;
             }
             while (!gpio_get(WE_PIN) );
          }
       }
//sprintf(buf, "addr %6.6X", bus_address);
//out_str(buf);

    }
}



/// UART
//
uint8_t __not_in_flash_func(uart_get_char)(void)
{
int inchar;

   inchar = getchar_timeout_us(0);

   while (inchar == PICO_ERROR_TIMEOUT)
      inchar = getchar_timeout_us(0);

   return((uint8_t)inchar);
}


uint8_t __not_in_flash_func(uart_get_cmd)(void)
{
uint8_t  uart_byte = 0;
bool     word_match = false;

   while (!word_match)
   {
     uart_byte = uart_get_char();

     if ((uart_byte == CMD_DUMP) ||
         (uart_byte == CMD_DUMPALL) ||
         (uart_byte == CMD_GETFIFO) ||
         (uart_byte == CMD_WRITE128K) ||
         (uart_byte == CMD_READ128K) ||
         (uart_byte == CMD_BOOT) ||
         (uart_byte == CMD_BOOT_BLOCK_CONT) ||
         (uart_byte == CMD_BOOT_BLOCK_FINAL))
     {
        word_match = true;
     }
   }
   return(uart_byte);
}

uint32_t __not_in_flash_func(uart_get_word)(void)
{
uint32_t uart_word = 0;

   uart_word = uart_get_char();
   uart_word |= (uart_get_char() << 8);
   uart_word |= (uart_get_char() << 16);
   uart_word |= (uart_get_char() << 24);

   return(uart_word);
}

uint32_t __not_in_flash_func(get_word)(uint32_t offset)
{
uint32_t value;
   value  = mem_array[offset];
   value |= (mem_array[offset+1] << 8);
   value |= (mem_array[offset+2] << 16);
   value |= (mem_array[offset+3] << 24);
   return(value);
}

void __not_in_flash_func(put_word)(uint32_t offset, uint32_t value)
{
   mem_array[offset]   = (value & 0xff);
   mem_array[offset+1] = ((value>>8) & 0xff);
   mem_array[offset+2] = ((value>>16) & 0xff);
   mem_array[offset+3] = ((value>>24) & 0xff);
}

void __not_in_flash_func(get_cmd_from_uart)(void)
{
uint8_t fx_command;
uint8_t uart_byte;
char buf[256];
uint32_t i, j, k;
uint32_t data_length;
uint32_t new_load_addr;
uint32_t queue_depth;

    fx_command = uart_get_cmd();

    if (fx_command == CMD_DUMP) {
       for (i = 0; i < 16; i++) {
          sprintf(buf, "%6.6X:", (i*16));
          for (k = 0; k < strlen(buf); k++) {
             putchar_raw(buf[k]);
          }
          for (j = 0; j < 16; j++) {
             sprintf(buf, " %2.2X", mem_array[(i*16) + j]);
             for (k = 0; k < strlen(buf); k++) {
                putchar_raw(buf[k]);
             }
          }
          putchar_raw(0x0D);
          putchar_raw(0x0A);
       }
       putchar_raw(0x0D);
       putchar_raw(0x0A);
       
    }

    else if (fx_command == CMD_DUMPALL) {
       for (i = 0; i < 1025; i++) {
          sprintf(buf, "%6.6X:", (i*16));
          for (k = 0; k < strlen(buf); k++) {
             putchar_raw(buf[k]);
          }
          for (j = 0; j < 16; j++) {
             sprintf(buf, " %2.2X", mem_array[(i*16) + j]);
             for (k = 0; k < strlen(buf); k++) {
                putchar_raw(buf[k]);
             }
          }
          putchar_raw(0x0D);
          putchar_raw(0x0A);
       }
       putchar_raw(0x0D);
       putchar_raw(0x0A);
    }
       
    else if (fx_command == CMD_GETFIFO) {

       queue_depth = ((from_pcfx_head + FIFO_SIZE - from_pcfx_tail) & FIFO_SIZE_MASK);
       putchar_raw(queue_depth & 0xFF);
       putchar_raw((queue_depth>>8) & 0xFF);

       while (queue_depth > 0) {
          putchar_raw(from_pcfx_consume_byte());
          queue_depth--;
       }
    }

    else if (fx_command == CMD_WRITE128K) {
       for (i = 0; i < 131072; i++) {
          mem_array[i] = uart_get_char();
       }
    }

    else if (fx_command == CMD_READ128K) {
       for (i = 0; i < 131072; i++) {
          putchar_raw(mem_array[i]);
       }
    }

    else if (fx_command == CMD_BOOT) {
       data_length = uart_get_word();

       for (i = 0; i < data_length; i++) {
          mem_array[CODE_BMP_OFFSET + i] = uart_get_char();
       }
       for (i = 0; i < 16; i++) {
          mem_array[0x0020 + i] = boot_bmp_20[i];
       }
       put_word(FX_BOOT_OFFSET, CODE_BMP_OFFSET);
       put_word(FX_BOOT_LOADADDR, FX_EXECUTION_ADDR);
       put_word(FX_BOOT_NUMBYTES, data_length);
       put_word(FX_BOOT_EXEC, FX_EXECUTION_ADDR);
       mem_array[FX_CONFIRMATION] = FX_WRITE_DONE;     // message that transfer is complete (but more is coming)
    }

    else if (fx_command == CMD_BOOT_BLOCK_CONT) {
       data_length = uart_get_word();

       for (i = 0; i < data_length; i++) {
          mem_array[CODE_BMP_OFFSET + i] = uart_get_char();
       }
       for (i = 0; i < 16; i++) {
          mem_array[0x0020 + i] = boot_bmp_20[i];
       }
       new_load_addr = get_word(FX_BOOT_LOADADDR) + get_word(FX_BOOT_NUMBYTES); // previous FX target addr + length
       put_word(FX_BOOT_OFFSET, CODE_BMP_OFFSET);
       put_word(FX_BOOT_LOADADDR, new_load_addr);
       put_word(FX_BOOT_NUMBYTES, data_length);
       put_word(FX_BOOT_EXEC, FX_EXECUTION_ADDR);
       mem_array[FX_CONFIRMATION] = FX_WRITE_DONE;     // message that transfer is complete (but more is coming)
    }

    else if (fx_command == CMD_BOOT_BLOCK_FINAL) {
       data_length = uart_get_word();

       for (i = 0; i < data_length; i++) {
          mem_array[CODE_BMP_OFFSET + i] = uart_get_char();
       }
       for (i = 0; i < 16; i++) {
          mem_array[0x0020 + i] = boot_bmp_20[i];
       }
       new_load_addr = get_word(FX_BOOT_LOADADDR) + get_word(FX_BOOT_NUMBYTES); // previous FX target addr + length
       put_word(FX_BOOT_OFFSET, CODE_BMP_OFFSET);
       put_word(FX_BOOT_LOADADDR, new_load_addr);
       put_word(FX_BOOT_NUMBYTES, data_length);
       put_word(FX_BOOT_EXEC, FX_EXECUTION_ADDR);
       mem_array[FX_CONFIRMATION] = FX_WRITE_FINISHED;     // message that transfer is complete (and no more will come)
    }
}


void to_pcfx_string(char * string)
{
int index = 0;

   while (*(string+index) != 0x00) {
      to_pcfx_add_byte(*(string+index));
      index++;
   }
}

void fake_from_pcfx_string(char * string)
{
int index = 0;

   while (*(string+index) != 0x00) {
      from_pcfx_add_byte(*(string+index));
      index++;
   }
}

int main() {

int a;
uint32_t outword;
int i;
bool gotclock;
char buf[100];

    stdio_init_all();

    for (i = 0; i < ARRAY_SIZE; i++) {
       mem_array[i] = 0;
    }
//    for (i = 0; i < 32; i++) {
//       mem_array[i] = formatted_bmp_00[i];
//    }
//    for (i = 0; i < 3; i++) {
//       mem_array[0x80 + i] = formatted_bmp_80[i];
//    }


//// UART
    // Set up our UART with the required speed.
    uart_init(UART_ID, BAUD_RATE);
    uart_set_translate_crlf(UART_ID, false);

    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
//    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
//    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
////

//////////////////////////////////////

    gpio_init(CE_PIN);
    gpio_pull_up(CE_PIN);
    gpio_put(CE_PIN, 1);
    gpio_set_dir(CE_PIN, GPIO_IN);

    gpio_init(OE_PIN);
    gpio_pull_up(OE_PIN);
    gpio_put(OE_PIN, 1);
    gpio_set_dir(OE_PIN, GPIO_IN);

    gpio_init(WE_PIN);
    gpio_pull_up(WE_PIN);
    gpio_put(WE_PIN, 1);
    gpio_set_dir(WE_PIN, GPIO_IN);

// Data GPIOs - initially input
//
    for (int gpio = 0; gpio < 8; gpio++) {
       gpio_init(DATA0_PIN + gpio);
       gpio_pull_up(DATA0_PIN + gpio);
       gpio_put(DATA0_PIN + gpio, 1);
       gpio_set_dir(DATA0_PIN + gpio, GPIO_IN);
    }

// Addr GPIOs
//
    for (int gpio = 0; gpio < 18; gpio++) {
       gpio_init(ADDR0_PIN + gpio);
       gpio_pull_up(ADDR0_PIN + gpio);
       gpio_put(ADDR0_PIN + gpio, 1);
       gpio_set_dir(ADDR0_PIN + gpio, GPIO_IN);
    }


    sleep_ms(500);

// Test code
//    fake_from_pcfx_string("Hello");
//    to_pcfx_string("Boo");
//    to_pcfx_add_byte('B');

    gotclock = set_sys_clock_khz(240000, 0);
//    if (gotclock)
//       sprintf(buf, "Got 240MHz clock");
//    else
//       sprintf(buf, "Clock stayed at 125MHz");
//    out_str(buf);
    
    multicore_launch_core1(core1_entry);

// wait at startup
//
    sleep_ms(500);

    while(1) {
      get_cmd_from_uart();
    }

    return 0;
}
