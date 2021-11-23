/*=========================================================================
 
  Pico7219usb

  main.c

  This is the main program for the Pico7218usb firmware. It includes
  the serial input/output routines, command parser, and font
  renderer.

  =========================================================================*/
#include <pico7219/pico7219.h> 
#include <pico/stdlib.h>
#include <stdio.h>
#include <string.h>
#include "prog/buffer.h" 
#include "prog/config.h"
#include "prog/protocol.h"

extern uint8_t font8_table[];

// Number of millisecond ticks since the last scroll operation
int scroll_count = 0;

// Set to TRUE if the display should be scrolled automatically
int scrolling = FALSE;

typedef struct Pico7219 Pico7219; // Shorter than "struct Pico7219..."

// Draw a character to the display. Note that the width of the "virtual
// display" can be much longer than the physical module chain, and
// off-display elements can later be scrolled into view. However, it's
// the job of the application, not the Pico7219 library, to size the virtual
// display sufficiently to fit all the text in.
//
// chr is an offset in the font table, which corresponds to a character 
// code, as the font table extends down to character zero. The CP437
// character set defines printable characters in the 0-31 range, that
// are not ASCII characters and which may, conceivably, be useful.
void draw_character (Pico7219 *pico7219, uint8_t chr, int x_offset, 
       BOOL flush)
  {
  for (int i = 0; i < 8; i++) // row
    {
    // The font elements are one byte wide even though, as it's an 8x5 font,
    //   only the top five bits of each byte are used.
    uint8_t v = font8_table[8 * chr + i];
    for (int j = 0; j < 8; j++) // column
      {
      int sel = 1 << j;
      if (sel & v)
        pico7219_switch_on (pico7219, 7 - i, 7- j + x_offset, FALSE);
      }
    }
  if (flush)
    pico7219_flush (pico7219);
  }

// Draw a string of text on the (virtual) display. This function assumes
// that the library has already been configured to provide a virtual
// chain of LED modules that is long enough to fit all the text onto.
void draw_string (Pico7219 *pico7219, const char *s, BOOL flush)
  {
  int x = 0;
  while (*s)
    {
    //draw_character (pico7219, *s - ' ', x, FALSE); 
    draw_character (pico7219, *s, x, FALSE); 
    s++;
    x += 6;
    }
  if (flush)
    pico7219_flush (pico7219);
  }

// Get the number of horizontal pixels that a string will take. Since each
// font element is five pixels wide, and there is one pixel between each
// character, we just multiply the string length by 6.
int get_string_length_pixels (const char *s)
  {
  return strlen (s) * 6;
  }

// Get the number of 8x8 LED modules that would be needed to accomodate the
// string of text. That's the number of pixels divided by 8 (the module
// width), and then one added to round up. 
int get_string_length_modules (const char *s)
  {
  return get_string_length_pixels(s) / 8 + 1;
  }

//
// size_and_draw_string
//
// Set the virtual module length to match the number of characters
// in the string, then draw the string.
void size_and_draw_string (Pico7219 *pico7219, const char *string)
  {
  int slm = get_string_length_modules (string);
  if (slm < CHAIN_LEN) slm = CHAIN_LEN;
  if (slm > pico7219_get_virtual_chain_length (pico7219)) 
    pico7219_set_virtual_chain_length (pico7219, slm);
  draw_string (pico7219, string, FALSE);
  }


void size_and_turn_on (Pico7219 *pico7219, int x, int y)
  {
  int slm = x / 8 + 1;
  if (slm < CHAIN_LEN) slm = CHAIN_LEN;
  if (slm > pico7219_get_virtual_chain_length (pico7219)) 
    pico7219_set_virtual_chain_length (pico7219, slm);
  pico7219_switch_on (pico7219, y, x, FALSE);
  }

void size_and_turn_off (Pico7219 *pico7219, int x, int y)
  {
  int slm = x / 8 + 1;
  if (slm < CHAIN_LEN) slm = CHAIN_LEN;
  if (slm > pico7219_get_virtual_chain_length (pico7219)) 
    pico7219_set_virtual_chain_length (pico7219, slm);
  pico7219_switch_off (pico7219, y, x, FALSE);
  }

//
// respond_error
//
// Send an error response to the host
//
void respond_error (int error, const char *text)
  {
  printf ("%d ", error);
  switch (error)
    {
    case ERR_NONE: printf ("OK"); break;
    case ERR_TOOSHORT: printf ("too_short"); break;
    case ERR_ARGS: printf ("bad_arguments"); break;
    case ERR_BADCMD: printf ("bad_command"); break;
    case ERR_TOOLONG: printf ("too_long"); break;
    }
  if (text) printf (" %s", text);
  printf ("\n");
  }

//
// respond_ok
//
void respond_ok ()
  {
  respond_error (ERR_NONE, NULL);
  }

// 
// process_input_buffer
//
// This is the command parser, and the place where most of the
// heavy lifting happens.
//
void process_input_buffer (Pico7219 *pico7219, Buffer *in_buffer, 
        Buffer *line_buffer)
  {
  if (in_buffer->pos > 0)
    {
    char cmd = in_buffer->c_str[0];
    switch (cmd)
      {
      int x, y;
      case CMD_ON:
        if (sscanf (in_buffer->c_str + 1, "%d,%d", &x, &y) == 2)
          {
	  size_and_turn_on (pico7219, y, x);
          respond_ok();
          }
        else 
          {
          respond_error (ERR_ARGS, in_buffer->c_str + 1);
          }
        break;

      case CMD_OFF:
        if (sscanf (in_buffer->c_str + 1, "%d,%d", &x, &y) == 2)
          {
	  size_and_turn_off (pico7219, y, x);
          respond_ok();
          }
        else 
          {
          respond_error (ERR_ARGS, in_buffer->c_str + 1);
          }
        break;

      case CMD_FLUSH:
        pico7219_flush (pico7219);
        respond_ok();
        break;

      case CMD_CHAR:
        if (in_buffer->length >= 2)
          {
          if (in_buffer->length < MAX_LINE)
            {
            buffer_append (line_buffer, in_buffer->c_str[1]);
            size_and_draw_string (pico7219, line_buffer->c_str);
            respond_ok();
            }
          else
            respond_error (ERR_TOOLONG, in_buffer->c_str + 1);
          }
        else
          respond_error (ERR_TOOSHORT, NULL);
        break;

      case CMD_STRING:
        if (in_buffer->length >= 2)
          {
          if (strlen (in_buffer->c_str + 1) < MAX_LINE)
            {
            buffer_set (line_buffer, in_buffer->c_str + 1);
            size_and_draw_string (pico7219, line_buffer->c_str);
            pico7219_flush (pico7219);
            respond_ok();
            }
          else
            respond_error (ERR_TOOLONG, in_buffer->c_str + 1);
          }
        else
          respond_error (ERR_TOOSHORT, NULL);
        break;

      case CMD_RESET:
        buffer_reset (line_buffer);
        pico7219_switch_off_all (pico7219, TRUE);
        pico7219_set_intensity (pico7219, 1);
        pico7219_set_virtual_chain_length (pico7219, CHAIN_LEN);
        scroll_count = SCROLL_TIME;
        scrolling = FALSE;
        respond_ok();
        break;

      case CMD_SCROLL:
        pico7219_scroll (pico7219, TRUE);
        respond_ok();
        break;

      case CMD_SCROLLON:
        scrolling = TRUE;
        scroll_count = SCROLL_TIME;
        respond_ok();
        break;

      case CMD_SCROLLOFF:
        scrolling = FALSE;
        scroll_count = SCROLL_TIME;
        respond_ok();
        break;

      case CMD_BRIGHTNESS:
        if (sscanf (in_buffer->c_str + 1, "%d", &x) == 1)
          {
          if (x < 0) x = 0;
          if (x > 15) x = 15;
          pico7219_set_intensity (pico7219, x);
          respond_ok();
          }
        else 
          {
          respond_error (ERR_ARGS, in_buffer->c_str + 1);
          }
        break;

      default:
        respond_error (ERR_BADCMD, in_buffer->c_str + 1);
      }
    }
  else
    {
    // Don't process empty line
    respond_error (ERR_TOOSHORT, NULL);
    }
  buffer_reset (in_buffer);
  }

//
// Start here
//
int main()
  {
  stdio_init_all();

  // Create the Pico7219 object, specifying the connected pins and
  //  baud rate. The last parameter indicates whether the column order
  //  should be reversed -- this depends on how the LED matrix is wired
  //  to the MAX7219, and isn't easy to determine except by trying.
  // With short wiring, the baud rate could be at least twice what is
  //  set here, but this probably isn't the limiting factor in 
  //  throughput.
  Pico7219 *pico7219 = pico7219_create (SPI_CHAN, 2000 * 1000,
    MOSI, SCK, CS, CHAIN_LEN, FALSE);

  // The module should power on blank, but let's be sure.
  pico7219_switch_off_all (pico7219, FALSE);

  // The input buffer, for commands from the host
  Buffer *in_buffer = buffer_new (MAX_INPUT);

  // The line buffer, for text to be displayed
  Buffer *line_buffer = buffer_new (MAX_LINE);

  int c;
  do
     {
     // Use getchar_timeout_us() so we can scroll the display -- if
     //  necessary -- while waiting for data from the host
     while ((c = getchar_timeout_us (1000)) == PICO_ERROR_TIMEOUT)
       {
       // Handle scrolling
       if (scrolling)
         {
         scroll_count--;
         if (scroll_count <= 0)
           {
           scroll_count = SCROLL_TIME;
           pico7219_scroll (pico7219, TRUE);
           }
         }
       }
     switch (c)
       {
       case 13: break; // Ignore CR (for testing with terminal emulator)

       case 10: // Line feed -- command is complete
         process_input_buffer (pico7219, in_buffer, line_buffer);  
         break;

       default:
         // Note that input that won't fit in the buffer is dropped.
         buffer_append (in_buffer, c);
       }
     } while (TRUE);

  // For completeness, but we never get here...

  buffer_destroy (in_buffer);
  buffer_destroy (line_buffer);
  pico7219_destroy (pico7219, FALSE);
  }

