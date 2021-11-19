# Pico7219usb

Version 0.1a, Novemeber 2021

_Work in progress_

## What is this?

`Pico7219usb` is firmware for the Raspberry Pi Pico, that enables it to 
interface an LED matrix to a host computer, using a USB connection. This
provides the host with an auxiliary display, that can display some useful
information in a highly visible way. The data can be sent using
simple command-line operations or scripts -- no other software is
needed.

The firmware is designed to work with chains of 8x8 LED matrices that are 
controlled by MAX7219 devices. These are available in 8x32 and
larger formats from a number of suppliers; so far as I can tell, they
are near-identical, perhaps even made in the same factory. An 8x32
unit can be had for as little as 5 UK pounds. Various colours are
available. An 8x32 matrix actually conists of four 8x8 units, each
with its own MAX7219 controller. 

The MAX7219 device uses a clocked serial interface, that works
well with the Pico's built-in SPI hardware. Note, however, that
these display modules are designed to run with 5-volt logic, as
well as a 5-volt supply. In general, the Pico's ~3V GPIO voltages
are sufficient to signal the MAX7219, but I wouldn't rely on this
in a commercial application -- not when a 3V-5V level shifter
only costs pennies. 

This software makes use of my low-level library for controlling 
MAX7219 device chains, which I've described in detail
[on my website](http://kevinboone.me/pico7219.html).

When the firmware is installed, the Pico emulates a USB serial
device. The host computer can send commands to write text,
scroll the display, etc., over the USB device. On a Linux machine,
the Pico device usually appears as `/dev/ttyACM0`.

The full (very simple) protocol is defined in the file `prog/protocol.h`.

## Building the firmware

To build the firmware, you'll need the official Raspberry Pi Pico
software development kit, and all its dependencies. Then the build
procedure on Linux is

    $ mkdir build_pico
    $ cd build_pico
    $ PICO_SDK_PATH=/path/to/pico/sdk cmake .. 
    $ make

This should provide `pico2719usb.uf2`, which can be copied to the
Pico device when it's in bootloader mode.

## Building the hardware

Only five connections are needed between the Pico and the display 
module. The file `pinout.txt` provide pin connections that match
the server. To use different pins, edit `prog/config.h`.

The connections should be as short as possible -- soldering
the Pico directly onto the back of the display module is probably 
best. With short connections, it might be possible to increase the
SPI baud rate, and get smoother scrolling.

However, you'll probably need to arrange the components so that the
Pico's BOOTSEL button is visible, so you can force the unit into
bootloader mode to upload new firmware. 

## Testing

It's possible to test the display using a terminal emulator (e.g., minicom,
terraterm). Please note, though, that the firmware isn't really designed to
work that way -- no characters are echoed back to the terminal, and there is
(obviously) no line editing.  The command should be terminated using (at least)
a line-feed character (ctrl-J), although it won't hurt if carriage return is
sent as well. It won't respond to CR on its own. The response sent by the Pico
will be terminated in a single LF, not CR-LF. Although not very
terminal-friendly, the protocol is designed to minimize the amount of data that
has to be exchanged. 

On Linux systems, it should be possible simply to send text strings directly to
the serial device. However, for reliable operation the device needs to be
configured in a particular way. Please see the file `test.sh` for how to do
this. To be honest, I don't know how to do the same thing on a Windows
system.

## Text control

A 4x32 display will only handle 5 or so characters so, in practice,
it's necessary to scroll the text systematically across the
display. The scrolling can be carried out under the control of the
host computer, or automatically by the firmware. 

A text string sent to the display can, therefore, be much long than
the display will accommodate. If it's _too_ long, though, it will
take too long to read. At present, the firmware will reject 
strings that are longer than 128 characters for this reason -- and
because they create a memory burden for the Pico. 

The 8x32 display can be seen as a scrolling window onto a longer
"virtual" display. Any text that is scrolled off the left-hand
edge of the display is moved to the right hand edge of the
virtual display. This means that as the text scrolls, it will wrap
around and come back into view. Be aware, though, that the 
software won't automatically apply any padding. If the text
being displayed is an exact fit in the physical display, and
the display scrolls, it will look very odd. It's worth putting
one or two spaces on the end of each text string, to avoid this
happening. Or to use logic to work out whether the display needs
to scroll at all. After all, there's no point scrolling text that
already fits on the display. In general, a text string of 
five characters or fewer will not need to be scrolled.

## Character set

The built-in font of `Pico2719usb` supports the 8-bit CP327 
character set. This character set has 256 characters, including
the standard ASCII set and some non-English characters.  

For a full listing, see this 
[Wikipedia page](https://en.wikipedia.org/wiki/Code_page_437#Character_set).

There is no support for any kind of multi-byte
character set. Linux systems typically use the UTF-8 character set,
and it's all too easy to send characters that make no sense to
the display.

Linux users can use the `iconv` utility to convert from the platform
default encoding to CP437. For example:

  $ TEXT=`echo α=Ω | iconv -t CP437`

Of course, there's no getting away from the fact that CP437 only
has 256 characters, and some common symbols (like the 
Euro currency symbol) are missing.
 
I doubt that the Pico ROM is large enough to accommodate a complete
Unicode font, even if there was such a thing that would render
properly on a display only 8 pixels tall.

## Controlling individual LEDs

The protocol provides ways to turn on and off individual LEDs in the
array. These operations require an explicit "flush" command to 
update the display -- until then, only the internal state is
changed. This process allows for smoother updates, if used
carefully.

Scrolling operations work with individually-set LEDs as well
as text; but be aware that the "virtual" display size is not
changed by writing individual LEDs (yet). 

## Tweaks

As it stands, `Pico2719usb` supports up to eight 8x8 modules. 
These settings can be tweaked in `pico7210/include/pico7219.h`. 
All the other relevant tweaks, including the pin connections,
are in `prog/config.h`.

## Limitations

Pixel-by-pixel scrolling requires a _lot_ of binary shift and rotate
arithmetic -- the display modules don't have any built-in support
for this. In addition, pixel data has to be shifted from one
8x8 module to the next, along the chain, to get to the point where
it will be displayed. What that means is that, to change one pixel
at the "far" end of the module, we have to write every pixel along
the way.

All this means that scrolling is not all that smooth. There is
a scroll delay setting that can be tweaked but, in practice, there
isn't much point reducing it below 100 msec, because the limiting
factor isn't the delay.
 
Although the brightness of the LEDs can be controlled, there is
no "dark" level of brightness. Even the zero brightness level 
has some illumination. This is a limit of the display modules --
the only way to make the display complete dark is to turn
off all the LEDs completely.

It should go without saying that an 8x5 font is not particulary
elegant. Some letters are hard to distinguish when the resolution
is this low ('H' and 'M', for example). 

It is probably possible to send commands to the firmware that will
make it crash -- the error checking is not that robust. 

It should be possible to power the Pico and display module from
a computer's USB port. Be aware, however, that if many LEDs are
turned on, and brightness is turned up, the current draw could
challenge a USB port. 


