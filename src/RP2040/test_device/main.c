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
#include "hardware/gpio.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTOTYPES
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


//#define ARRAY_SIZE      131072
#define ARRAY_SIZE      32768
//#define GPIO_MASK_ADDR  0x03FFFF00 // ultimately, 128KB + port space
#define GPIO_MASK_ADDR  0x007FFF00 // currently, 16KB + port space

//#define GPIO_ADDR_THRESH  0x00020000 // ultimately, port space threshold
#define GPIO_ADDR_THRESH  0x00004000 // currently, port space threshold

#define GPIO_ADDR_SHIFT 8

#define GPIO_MASK_DATA  0x000000FF


uint8_t  mem_array[ARRAY_SIZE];
uint32_t bus_address = 0;
uint8_t  *memptr = &mem_array[0];
uint32_t outmem = 0;
uint32_t bus_data = 0;



uint8_t  dump_cmd = '?'; // dump 256 bytes of hex





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

     if ((uart_byte == dump_cmd))
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

    if (fx_command == dump_cmd) {
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
}


void __not_in_flash_func(out_str)(char * string)
{
   for (int i = 0; i < strlen(string); i++) {
      putchar_raw( *(string+i) );
   }
   putchar_raw(0x0D);
   putchar_raw(0x0A);
}


void __not_in_flash_func(write_mem)(int addr, uint8_t data)
{
uint8_t junk;

   gpio_put_masked(GPIO_MASK_ADDR, (addr << GPIO_ADDR_SHIFT));
   gpio_put(CE_PIN, 0);
   gpio_put_masked(GPIO_MASK_DATA, data);
   gpio_set_dir_masked(GPIO_MASK_DATA, 0xFF);
   gpio_put(WE_PIN, 0);

   junk = gpio_get(OE_PIN);     // try to waste 48ns
   junk = gpio_get(DATA0_PIN);
   junk = gpio_get(DATA0_PIN+1);
   junk = gpio_get(DATA0_PIN+2);
   junk = gpio_get(OE_PIN);     // try to waste 48ns
   junk = gpio_get(DATA0_PIN);
   junk = gpio_get(DATA0_PIN+1);
   junk = gpio_get(DATA0_PIN+2);
   junk = gpio_get(OE_PIN);     // try to waste 48ns
   junk = gpio_get(DATA0_PIN);
   junk = gpio_get(DATA0_PIN+1);
   junk = gpio_get(DATA0_PIN+2);
   junk = gpio_get(OE_PIN);     // try to waste 48ns
   junk = gpio_get(DATA0_PIN);
   junk = gpio_get(DATA0_PIN+1);
   junk = gpio_get(DATA0_PIN+2);
   junk = gpio_get(OE_PIN);     // try to waste 48ns
   junk = gpio_get(DATA0_PIN);
   junk = gpio_get(DATA0_PIN+1);
   junk = gpio_get(DATA0_PIN+2);
   junk = gpio_get(OE_PIN);     // try to waste 48ns
   junk = gpio_get(DATA0_PIN);

   gpio_put(WE_PIN, 1);
   gpio_set_dir_masked(GPIO_MASK_DATA, 0x00);  // input
   gpio_put(CE_PIN, 1);
}

uint8_t __not_in_flash_func(read_mem)(int addr)
{
uint8_t data_read;
uint8_t junk;
int i;

char buf[32];

//sprintf(buf, "addr %6.6X", addr);
//out_str(buf);

   gpio_put_masked(GPIO_MASK_ADDR, (addr << GPIO_ADDR_SHIFT));
   gpio_put(CE_PIN, 0);

   junk = gpio_get(WE_PIN);     // try to waste 48ns
   junk = gpio_get(DATA0_PIN);
   junk = gpio_get(DATA0_PIN+1);

   gpio_put(OE_PIN, 0);

   junk = gpio_get(WE_PIN);     // try to waste 48ns
   junk = gpio_get(DATA0_PIN);
   junk = gpio_get(DATA0_PIN+1);
   junk = gpio_get(DATA0_PIN+2);
   junk = gpio_get(WE_PIN);     // try to waste 48ns
   junk = gpio_get(DATA0_PIN);
   junk = gpio_get(DATA0_PIN+1);
   junk = gpio_get(DATA0_PIN+2);
   junk = gpio_get(WE_PIN);     // try to waste 48ns
   junk = gpio_get(DATA0_PIN);
   junk = gpio_get(DATA0_PIN+1);
   junk = gpio_get(DATA0_PIN+2);
   junk = gpio_get(WE_PIN);     // try to waste 48ns
   junk = gpio_get(DATA0_PIN);
   junk = gpio_get(DATA0_PIN+1);
   junk = gpio_get(DATA0_PIN+2);
   junk = gpio_get(WE_PIN);     // try to waste 48ns
   junk = gpio_get(DATA0_PIN);
   junk = gpio_get(DATA0_PIN+1);
   junk = gpio_get(DATA0_PIN+2);
   junk = gpio_get(WE_PIN);     // try to waste 48ns
   junk = gpio_get(DATA0_PIN);
   junk = gpio_get(DATA0_PIN+1);
   junk = gpio_get(DATA0_PIN+2);

   gpio_put(OE_PIN, 1);
   data_read = (gpio_get_all() & GPIO_MASK_DATA);
   gpio_put(CE_PIN, 1);

   return(data_read);
}

uint8_t __not_in_flash_func(read_mem_part1)(int addr)
{
uint8_t data_read;
uint8_t junk;
int i;

   gpio_put_masked(GPIO_MASK_ADDR, (addr << GPIO_ADDR_SHIFT));
   gpio_put(CE_PIN, 0);

   junk = gpio_get(WE_PIN);     // try to waste 48ns
   junk = gpio_get(DATA0_PIN);
   junk = gpio_get(DATA0_PIN+1);

   gpio_put(OE_PIN, 0);

   junk = gpio_get(WE_PIN);     // try to waste 48ns
   junk = gpio_get(DATA0_PIN);
   junk = gpio_get(DATA0_PIN+1);
   junk = gpio_get(DATA0_PIN+2);
   junk = gpio_get(WE_PIN);     // try to waste 48ns
   junk = gpio_get(DATA0_PIN);
   junk = gpio_get(DATA0_PIN+1);
   junk = gpio_get(DATA0_PIN+2);
   junk = gpio_get(WE_PIN);     // try to waste 48ns
   junk = gpio_get(DATA0_PIN);
   junk = gpio_get(DATA0_PIN+1);
   junk = gpio_get(DATA0_PIN+2);
   junk = gpio_get(WE_PIN);     // try to waste 48ns
   junk = gpio_get(DATA0_PIN);
   junk = gpio_get(DATA0_PIN+1);
   junk = gpio_get(DATA0_PIN+2);
   junk = gpio_get(WE_PIN);     // try to waste 48ns
   junk = gpio_get(DATA0_PIN);
   junk = gpio_get(DATA0_PIN+1);
   junk = gpio_get(DATA0_PIN+2);
   junk = gpio_get(WE_PIN);     // try to waste 48ns
   junk = gpio_get(DATA0_PIN);
   junk = gpio_get(DATA0_PIN+1);
   junk = gpio_get(DATA0_PIN+2);

   gpio_put(OE_PIN, 1);
   data_read = (gpio_get_all() & GPIO_MASK_DATA);

   return(data_read);
}

uint8_t __not_in_flash_func(read_mem_part2)(int addr)
{
uint8_t data_read;
uint8_t junk;
int i;

   gpio_put_masked(GPIO_MASK_ADDR, (addr << GPIO_ADDR_SHIFT));

   junk = gpio_get(WE_PIN);     // try to waste 48ns
   junk = gpio_get(DATA0_PIN);
   junk = gpio_get(DATA0_PIN+1);

   gpio_put(OE_PIN, 0);

   junk = gpio_get(WE_PIN);     // try to waste 48ns
   junk = gpio_get(DATA0_PIN);
   junk = gpio_get(DATA0_PIN+1);
   junk = gpio_get(DATA0_PIN+2);
   junk = gpio_get(WE_PIN);     // try to waste 48ns
   junk = gpio_get(DATA0_PIN);
   junk = gpio_get(DATA0_PIN+1);
   junk = gpio_get(DATA0_PIN+2);
   junk = gpio_get(WE_PIN);     // try to waste 48ns
   junk = gpio_get(DATA0_PIN);
   junk = gpio_get(DATA0_PIN+1);
   junk = gpio_get(DATA0_PIN+2);
   junk = gpio_get(WE_PIN);     // try to waste 48ns
   junk = gpio_get(DATA0_PIN);
   junk = gpio_get(DATA0_PIN+1);
   junk = gpio_get(DATA0_PIN+2);
   junk = gpio_get(WE_PIN);     // try to waste 48ns
   junk = gpio_get(DATA0_PIN);
   junk = gpio_get(DATA0_PIN+1);
   junk = gpio_get(DATA0_PIN+2);
   junk = gpio_get(WE_PIN);     // try to waste 48ns
   junk = gpio_get(DATA0_PIN);
   junk = gpio_get(DATA0_PIN+1);
   junk = gpio_get(DATA0_PIN+2);

   gpio_put(OE_PIN, 1);
   data_read = (gpio_get_all() & GPIO_MASK_DATA);
   gpio_put(CE_PIN, 1);

   return(data_read);
}



void __not_in_flash_func(out_hex)(uint8_t hexval)
{
int nybble;
int outchar;

   nybble = ((hexval >> 4) & 0x0F);
   if (nybble < 10)
     outchar = '0' + nybble;
   else
     outchar = 'A' + nybble - 10;
   putchar_raw(outchar);

   nybble = (hexval & 0x0F);
   if (nybble < 10)
     outchar = '0' + nybble;
   else
     outchar = 'A' + nybble - 10;
   putchar_raw(outchar);

   putchar_raw(0x0D);
   putchar_raw(0x0A);
}

void write_to_queue(uint8_t * string)
{
int i;

   for (i = 0; i < strlen(string); i++) {
      write_mem(0x4000, *(string+i));
      sleep_us(1);
   }
}

void __not_in_flash_func(core1_entry)(void)
{
uint8_t data, data2;
uint8_t junk;

   out_str("Read 0x00");
   data = read_mem(0x00);
   out_hex(data);

   sleep_us(15);

   out_str("Read 0x01");
   data = read_mem(0x01);
   out_hex(data);

   sleep_us(15);

   out_str("Read 0x02");
   data = read_mem(0x02);
   out_hex(data);

   sleep_us(15);

   out_str("Read 0x03 and 0x04");
   data = read_mem_part1(0x03);

   junk = gpio_get(WE_PIN);     // try to waste 48ns
   junk = gpio_get(DATA0_PIN);
   junk = gpio_get(DATA0_PIN+1);
   junk = gpio_get(DATA0_PIN+2);

   data2 = read_mem_part2(0x04);
   out_hex(data);
   out_hex(data2);

   sleep_us(15);

   out_str("Write 0x12 with 0x13");
   write_mem(0x12, 0x13);

   sleep_us(15);

   out_str("Write 0x13 with 0xC6");
   write_mem(0x13, 0xC6);

   sleep_us(15);

   out_str("Read 0x4001");
   data = read_mem(0x4001);
   out_hex(data);

   sleep_us(15);

   out_str("Read 0x4000");
   data = read_mem(0x4000);
   out_hex(data);

   sleep_us(15);

   out_str("Read 0x4010");
   data = read_mem(0x4010);
   out_hex(data);

   sleep_us(15);

   out_str("Read 0x4001");
   data = read_mem(0x4001);
   out_hex(data);

   sleep_us(15);

   out_str("Read 0x4000");
   data = read_mem(0x4000);
   out_hex(data);

   sleep_us(15);

   out_str("Put 'Hi There' in FIFO queues");
   write_to_queue("Hi There");

   sleep_us(15);

   putchar_raw(0x0D);
   putchar_raw(0x0A);
}



int main() {

int a;
uint32_t outword;
int i;

    gpio_init(CE_PIN);
    gpio_pull_up(CE_PIN);
    gpio_put(CE_PIN, 1);
    gpio_set_dir(CE_PIN, GPIO_OUT);

    stdio_init_all();

    for (i = 0; i < ARRAY_SIZE; i++) {
       mem_array[i] = 0;
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

    // All four communications state machines run on one PIO processor

    gpio_init(CE_PIN);
    gpio_pull_up(CE_PIN);
    gpio_put(CE_PIN, 1);
    gpio_set_dir(CE_PIN, GPIO_OUT);

    gpio_init(OE_PIN);
    gpio_pull_up(OE_PIN);
    gpio_put(OE_PIN, 1);
    gpio_set_dir(OE_PIN, GPIO_OUT);

    gpio_init(WE_PIN);
    gpio_pull_up(WE_PIN);
    gpio_put(WE_PIN, 1);
    gpio_set_dir(WE_PIN, GPIO_OUT);

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
       gpio_set_dir(ADDR0_PIN + gpio, GPIO_OUT);
    }

    sleep_ms(7000);

    multicore_launch_core1(core1_entry);



    while(1) {
//      get_cmd_from_uart();
    }

    return 0;
}
