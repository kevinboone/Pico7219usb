#!/usr/bin/perl -w
# 
# A simple Perl utility to draw an hour:minute:second clock on a 32x8
# LED matrix, using Pico7219usb firmware. This program demonstrates how
# to use the single-LED setting facilities of Pico7219usb to draw 
# very small, LED-style numbers -- the standard font won't allow eight
# symbols (HH:MM:SS) to fit into a 32-LED width.
#
# Note that the Pico7219usb protocol is fully synchronous -- the Pico
# firmware expects a client to consume its response before sending the
# next command. This means that updating a large number of individual
# LEDs can be quite slow. We have to take some care to avoid updating
# parts of the display that have not changed from one second to the 
# next, but there's still a lot of work to do, between the Pico and the
# display module, because LED data has to be shifted from one end of
# the LED array to the other. As a result, we don't actually need any
# time delay in this code -- it takes about half a second to update
# the display for each new second.
#
# Copyright (c)2021 Kevin Boone, GPL v3.0.

use strict;

# The serial device to which the Pico is connected

my $device = "/dev/ttyACM0";

# Bitmap table for the ten digits and the colon character. Each digit is
#  5x3 pixels, so each array entry is a row, and only the lowest three
#  bits is actually used.

my @digit_0 = (0x07, 0x05, 0x05, 0x05, 0x07); 
my @digit_1 = (0x01, 0x01, 0x01, 0x01, 0x01); 
my @digit_2 = (0x07, 0x01, 0x07, 0x04, 0x07); 
my @digit_3 = (0x07, 0x01, 0x07, 0x01, 0x07); 
my @digit_4 = (0x05, 0x05, 0x07, 0x01, 0x01); 
my @digit_5 = (0x07, 0x04, 0x07, 0x01, 0x07); 
my @digit_6 = (0x07, 0x04, 0x07, 0x05, 0x07); 
my @digit_7 = (0x07, 0x01, 0x01, 0x01, 0x01); 
my @digit_8 = (0x07, 0x05, 0x07, 0x05, 0x07); 
my @digit_9 = (0x07, 0x05, 0x07, 0x01, 0x07); 
my @colon   = (0x00, 0x02, 0x00, 0x02, 0x00); 

# send_and_receive (cmd)
# Send a message to the Pico7219usb, and wait for the response. Ideally,
#  we should check the response for error.

sub send_and_receive ($)
  {
  print OUT $_[0];
  print OUT "\n";
  my $response = <IN>;
  # TODO -- check error response
  }

# rst() 
# Send a reset command.

sub rst()
  {
  send_and_receive ("R");
  }

# dim() 
# Set intensity zero (or whatever you like)

sub dim()
  {
  send_and_receive ("I0");
  }

# flush() 
# Flush the individual LED changes to the hardware 

sub flush()
  {
  send_and_receive ("F");
  }

# on(row,col)
# Turn on the LED at row,col

sub on($$)
  {
  my $row = $_[0];
  my $col = $_[1];
  send_and_receive ("A$row,$col");
  }

# off(row,col)
# Turn off the LED at row,col

sub off($$)
  {
  my $row = $_[0];
  my $col = $_[1];
  send_and_receive ("B$row,$col");
  }

# draw_sym(offset, list) 
# Draw the symbol represented by the five-number list, at the
#   column offset given by the first item in the list. The list is
#   one of the entries in the digit bitmap table.

sub draw_sym (@)
  {
  my $col = $_[0];
  for (my $r = 1; $r < 6; $r++)
    {
    my $row = $_[$r];
    for (my $c = 0; $c < 3; $c++)
      {
      my $o = $row & 0x01;
      # Depending on whether the bit in the bitmap is a 0 or a 1,
      #   turn on or off the LED at the corresponding position.
      if ($o)
	{
	on (6-$r, $col+3-$c);
	}
      else
        {
	off (6-$r, $col+3-$c);
	}
      $row = $row >> 1; 
      }
    }
  }

# draw_colon (offset)
# Draw the colon symbol at the specified column offset

sub draw_colon ($)
  {
  my $col = $_[0];
  draw_sym ($col, @colon); 
  }

# draw_digit (offset, digit)
# Draw the specified digit (0..9) at the specified column offset.

sub draw_digit ($$)
  {
  my $col = $_[0];
  my $digit = $_[1];
  if ($digit==1) { draw_sym ($col, @digit_1); }
  elsif ($digit==2) { draw_sym ($col, @digit_2); }
  elsif ($digit==3) { draw_sym ($col, @digit_3); }
  elsif ($digit==4) { draw_sym ($col, @digit_4); }
  elsif ($digit==5) { draw_sym ($col, @digit_5); }
  elsif ($digit==6) { draw_sym ($col, @digit_6); }
  elsif ($digit==7) { draw_sym ($col, @digit_7); }
  elsif ($digit==8) { draw_sym ($col, @digit_8); }
  elsif ($digit==9) { draw_sym ($col, @digit_9); }
  else { draw_sym ($col, @digit_0); }
  }

# Use stty to initialise the serial port in a way that
#   is appropriate for this utility. It would be more elegant to use the 
#   Perl serial library, but not every installation has that.
system ("stty -F /dev/ttyACM0 speed 115200 >& /dev/null");
system ("stty -F /dev/ttyACM0 crtscts -ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoke -echok");

open OUT, ">$device" or die;
open IN, "<$device" or die;

rst();
dim();

# Store the hour and minute of each iteration, so we don't update hours
#   and minutes that have not changed.
my $last_min = -1;
my $last_hour = -1;

# Draw the colons, just once for the life of the program.
draw_colon (18);
draw_colon (7);

# Loop forever, updating the time in each iteration.
while (1)
  {
  my $sec; my $min; my $hour; my $dummy;
  ($sec,$min,$hour,$dummy,$dummy,$dummy,$dummy,$dummy,$dummy) = localtime();

  my $s1 = int ($sec / 10);
  my $s2 = $sec - ($s1 * 10);

  draw_digit (26, $s2);
  draw_digit (22, $s1);

  if ($min != $last_min)
    {
    my $m1 = int ($min / 10);
    my $m2 = $min - ($m1 * 10);
    draw_digit (15, $m2);
    draw_digit (11, $m1);
    }

  if ($hour != $last_hour)
    {
    my $h1 = int ($hour / 10);
    my $h2 = $hour - ($h1 * 10);
    draw_digit (4, $h2);
    draw_digit (0, $h1);
    }

  flush();

  $last_hour = $hour;
  $last_min = $min;
  }

# We don't get here.
close IN;
close OUT;

