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
// be left up to the compiler to decde)
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


#define DATA0_PIN      0      // use GPIO 0-7  for data
#define ADDR0_PIN      8      // use GPIO 8-25 for address
#define CE_PIN        26      // use GPIO 26 for Chip Enable
#define OE_PIN        27      // use GPIO 27 for Output Enable
#define WE_PIN        28      // use GPIO 28 for Write Enable



#define GPIO_TESTSIZE   // use GPIO_FULLSIZE when all GPIOs are available and connected

#ifdef GPIO_FULLSIZE

#define ARRAY_SIZE        131072
#define GPIO_MASK_ADDR    0x03FFFF00 // ultimately, 128KB + port space
#define GPIO_ADDR_THRESH  0x00020000 // ultimately, port space threshold
#define GPIO_PORT_DATA    0x00020000
#define GPIO_PORT_CONTROL 0x00020001

#else                   // else, these are the TESTSIZE variables

#define ARRAY_SIZE        32768
#define GPIO_MASK_ADDR    0x007FFF00 // currently, 16KB + port space
#define GPIO_ADDR_THRESH  0x00004000 // currently, port space threshold
#define GPIO_PORT_DATA    0x00004000
#define GPIO_PORT_CONTROL 0x00004001

#endif

#define GPIO_ADDR_SHIFT   8              // since A0 = GPIO8, this is the shift amount

#define GPIO_MASK_DATA    0x000000FF     // bottom GPIOs (GPIO0-7) used for data

#define VALUE_INV_ADDR    0xAA           // 'floating' lines. Should be 0xFF, but 0xAA during test.


// PC-FX based bits:
#define  FIFO_CTRL_DO_NOT_WRITE  0x80    // bit 7 = HIGH when queue full (i.e. do not write)
#define  FIFO_CTRL_DO_NOT_READ   0x40    // bit 6 = HIGH when queue empty (i.e. do not read)

// Memory array variables:
//
volatile uint8_t  mem_array[ARRAY_SIZE];


// FIFO port variables:
//
#define FIFO_SIZE        1024
#define FIFO_SIZE_MASK   0x3FF

volatile uint8_t  fifo_from_pcfx[FIFO_SIZE];
volatile uint8_t  fifo_to_pcfx[FIFO_SIZE];

volatile int      from_pcfx_tail = 0; // outbound data to PC
volatile int      from_pcfx_head = 0;

volatile int      to_pcfx_tail = 0;   // tail points at location to read next data from
volatile int      to_pcfx_head = 0;   // head points at location to write next data to

volatile uint8_t  control  = FIFO_CTRL_DO_NOT_READ; // flags to tell PCFX whether not to read/write
volatile uint8_t  dataport = VALUE_INV_ADDR;        // pre-fetched data for fifo_to_pcfx[to_pcfx_tail]

// debug stuff commented out
//
//volatile int      writes_to_port = 0;
//volatile int      writes_not_to_port = 0;

//volatile uint32_t write_addr[128];
//volatile int write_addr_index = 0;


// default format for a (formatted and empty) 128KB memory card
//
uint8_t formatted_bmp_00[] = {
0x24, 0x8A, 0xDF, 0x50, 0x43, 0x46, 0x58, 0x43,  0x61, 0x72, 0x64, 0x80, 0x00, 0x01, 0x01, 0x00,
0x01, 0xFC, 0x00, 0x00, 0x04, 0xF9, 0x0C, 0x00,  0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t formatted_bmp_80[] = {
0xF9, 0xFF, 0xFF
};


uint8_t  cmd_dump    = '?';   // dump 256 bytes of hex
uint8_t  cmd_dumpall = '!';   // dump 16384+16 bytes of hex
uint8_t  cmd_getfifo = '/';   // read FIFO




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
         dataport = 0xAA;
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

             indata = (gpio_get_all() & GPIO_MASK_DATA);

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

     if ((uart_byte == cmd_dump) ||
         (uart_byte == cmd_dumpall) ||
         (uart_byte == cmd_getfifo))
     {
        word_match = true;
     }
   }
   return(uart_byte);
}


void __not_in_flash_func(get_cmd_from_uart)(void)
{
uint8_t fx_command;
char buf[256];
int i, j, k;

    fx_command = uart_get_cmd();

    if (fx_command == cmd_dump) {
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

    else if (fx_command == cmd_dumpall) {
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
       
    else if (fx_command == cmd_getfifo) {
       sprintf(buf, "tail = %d, head = %d", from_pcfx_tail, from_pcfx_head);
       out_str(buf);

//       sprintf(buf, "writes to port = %d", writes_to_port);
//       out_str(buf);
//       sprintf(buf, "writes not to port = %d", writes_not_to_port);
//       out_str(buf);

//       sprintf(buf, "write_addr_index = %d", write_addr_index);
//       out_str(buf);
//
//       if (write_addr_index > 0) {
//          for (i = 0; i < write_addr_index; i++) {
//             sprintf(buf, "write_addr[%d] = %8.8X", i, write_addr[i]);
//             out_str(buf);
//          }
//       }

       while (from_pcfx_tail != from_pcfx_head) {
          putchar_raw(from_pcfx_consume_byte());
       }
       putchar_raw(0x0D);
       putchar_raw(0x0A);
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
    for (i = 0; i < 32; i++) {
       mem_array[i] = formatted_bmp_00[i];
    }
    for (i = 0; i < 3; i++) {
       mem_array[0x80 + i] = formatted_bmp_80[i];
    }


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

//    fake_from_pcfx_string("Hello");
//    to_pcfx_string("Boo");
    to_pcfx_add_byte('B');

    gotclock = set_sys_clock_khz(240000, 0);
    if (gotclock)
       sprintf(buf, "Got 240MHz clock");
    else
       sprintf(buf, "Clock stayed at 125MHz");
    out_str(buf);
    
    multicore_launch_core1(core1_entry);

// wait at startup
//
    sleep_ms(500);

    while(1) {
      get_cmd_from_uart();
    }

    return 0;
}
