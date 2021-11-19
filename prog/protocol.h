/*=========================================================================
 
Pico7219usb

protocol.h 

This file contains definitions related to the communications protocol that
Pico7219usb supports. Once programmed, the Pico emulates a USB serial device.
When connected to a Linux systems, this device typically appears as
/dev/ttyUSB0 or /dev/ttyACM0. It is often necessary to exclude this device from
the control of the modem manager, or just stop the modem manager completely.
Otherwise, when the Pico device is plugged in, the modem manager will send it a
bunch of AT commands, as if it were a real modem.    

All commands sent to the Pico device start with a single letter, followed by
some data, followed by a line feed (character 10). Any additional CR (13)
character is ignored.  Nothing is echoed back to the sender, so all the sender
needs to look for is a response after it has sent its command.  

Responses are terminated by a LF, without CR.

The response consists of a numeric code, then a single text token which is
string version of the numeric code, then perhaps some additional data,
depending on the error. A success code is "0 OK <LF>"

Commands fall into three groups:

1. Commands to control the display in general
2. Commands to turn on or turn off individual LEDs
3. Commands to write text

In group (1) are commands to reset the display ('R'), control the brightness
('I'), and control scrolling behaviour ('S', 'G', and 'H').  Group (2) conists
only of commands 'A' and 'B' to turn individual LEDs on and off.  Group (3)
consists of character output ('C') and string output ('D') commands.

Note that most commands do not affect the physical display immediately,
but only when the flush ('F') command is sent. This is to allow more
complex displays to be built up without repeated output to the hardware
(which is comparatively slow).

Note that the built-in font implements the CP437 8-bit character
set. There is no support of any kind for any multi-byte character
set. CP437 has a few non-English letters and some math symbols,
in addition to ASCII. 

The syntax of the individual commands, along with error codes, are
listed below.

Copyright (c)2021 Kevin Boone, GPL v3.0

  =========================================================================*/
#pragma once

// ON -- Acol,row
// Turn on an LED
// Row, col are zero-indexed, from the _bottom left_ corner.  It isn't an error
// to set values that are outside the range of the display, but they are
// silently ignored.  Values that are outside the range of the physical
// display, but in the range of the virtual display, could later be scrolled
// into view, but there is presently no way to extend the size of the virtual
// display using a command -- this is done automatically when displaying text.
// I guess it would be possible to display a line of spaces to extend the
// virtual display. The change is not written to th display hardware until the
// flush command is issued.
#define CMD_ON       'A'

// OFF -- Bcol,row
// Turn off an LED. Same considerations as ON apply.
#define CMD_OFF      'B'

// CHAR -- Cc 
// Adds a character to the display buffer. The display is not updated
//  until the flush command is issued. In practice, it will usually
//  be more efficient to use the STRING command to send a whole
//  text string. 
#define CMD_CHAR     'C'

// STRING -- Dstring
// Replaces the line buffer with 'string'. Changes are effected immediately.
// There is a limit on the string length and, of course, a string longer than a
// few characters will not fit on the display anyway. However, a longer string
// can be sent, and then scrolled into view, either by enabling automatic
// scrolling, or by sending the SCROLL command. Using long strings uses a lot
// of memory, so a limit of 128 characters is enforced. This can be changed in
// config.h, but be careful of memory usage. If the display is currently
// scrolling, this command will not change that -- the new text will also
// scroll. This command will not force scrolling to start, even if the text
// doesn't fit on the display. 
#define CMD_STRING   'D'

// FLUSH -- F
// Flush changes to the physical hardware.
#define CMD_FLUSH    'F'

// SCROLL_ON -- G
// Enable scrolling. The physical display can be viewed as a window onto
// a larger "virtual" display. Scrolling moves this window to the right,
// so text appears to move to the left. The scrolling speed is about
// ten pixels per second -- it's difficult to get it much faster than
// this because of the amount of bit-bashing needed.
#define CMD_SCROLLON 'G'

// SCROLL_OFF -- H
// Disable scrolling. Note that the scrolling position stays where it is, which
// might not leave the beginning of the current text in the display.
#define CMD_SCROLLOFF 'H'

// BRIGHTNESS -- In
// 'n' is 0-15
// Note that there is no "off" brightness -- even zero-brightness is visible.
// Values outside the 0-15 range are silently clipped. The default brightness
// is "1". Note that increasing the brightness increases the current drawn by
// the display. Turning on all LEDs at full brightnes could result in a current
// draw of about 0.5A -- that's at the limit of what a USB port will normally
// supply. 
#define CMD_BRIGHTNESS 'I'

// RESET -- R
// Clear pixels, clear the line buffer, stop scrolling, set brightness to 1
#define CMD_RESET    'R'

// SCROLL -- S 
// Scroll one pixel left. If scrolling is already active, this won't be
// visible. Implicitly flushes updates to the hardware.
#define CMD_SCROLL   'S'

// 
// Error codes
//
#define ERR_NONE     0

// Command input too short (e.g., no command character)
#define ERR_TOOSHORT 1

// Too many or too few arguments
#define ERR_ARGS     2

// Unknown command character
#define ERR_BADCMD   3

// Command input too long (e.g., too many characters in string)
#define ERR_TOOLONG  4


