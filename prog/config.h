/*=========================================================================
 
  Pico7219usb

  config.h 

  This file defines fundamental configuration parameters for the 
  software, including limits and pin assignments.

  =========================================================================*/

#pragma once

//
// Pin assignments
//
// ***** IMPORTANT *****
// The numbers for MOSI (aka SPI TX), SCK and CS in the Pico C API
// are GPIO numbers, _not_ physical pin numbers. This can be very
// confusing. See pinout.txt for physical pin numbers and general
// wiring information.
//

// MOSI pin, also called "TX" in the Pico pinout. Connects to the
//   "DIN" pin on the MAX7219.
#define MOSI 19

// SCK pin. Connects to the "CLOCK" or "CLK"  pin on the MAX7219.
#define SCK 18

// Chip-select pin. The same name is used on the Pico and the MAX7219.
#define CS 17

// Number of physical 8x8 modules in the display chain
#define CHAIN_LEN 4

// SPI channel can be 0 or 1, and depends on the pins you've wired
//   to be MOSI, etc. 
#define SPI_CHAN 0

// Maximum length of an input command. Must be greated than MAX_LINE.
#define MAX_INPUT 256 

// Maximum length of a text line on the display. This could be made
//  much larger, but long lines taken ages to read unless the scroll
//  speed is very fast. 
#define MAX_LINE 128 

// Time in milliseconds to delay between scrolls, when auto-scrolling.
// In practice, it's hard to get very fast scrolling because of the
// amount of data that has to be transferred, and the amount of binary
// math required.  
#define SCROLL_TIME 100 



