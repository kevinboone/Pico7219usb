/*=========================================================================
 
  Pico7219usb

  buffer.c

  Implements a fixed-size buffer of characters. See buffer.h for
  details.

  (c)2021 Kevin Boone, GPL v3.0

  =========================================================================*/

#include <stdlib.h>
#include <string.h>
#include "prog/buffer.h"

// 
// buffer_new
//
Buffer *buffer_new (int length)
  {
  Buffer *self = malloc (sizeof (Buffer));
  if (self)
    {
    self->c_str = malloc (length * sizeof (char) + 1);
    if (self->c_str)
      {
      self->length = length;
      self->pos = 0;
      self->c_str[self->pos] = 0;
      }
    else
      {
      free (self);
      self = NULL;
      }
    }
  return self;
  }

// 
// buffer_destroy
//
void buffer_destroy (Buffer *self)
  {
  if (self)
    {
    if (self->c_str) free (self->c_str);
    free (self);
    }
  }

// 
// buffer_apppend
//
void buffer_append (Buffer *self, char c)
  {
  if (self->pos < self->length - 1)
    {
    self->c_str [self->pos] = c;
    self->c_str [self->pos + 1] = 0;
    self->pos++;
    }
  }

// 
// buffer_reset
//
void buffer_reset (Buffer *self)
  {
  self->pos = 0;
  self->c_str [self->pos] = 0;
  }

// 
// buffer_backspace
//
void buffer_backspace (Buffer *self)
  {
  if (self->pos > 0)
    {
    self->pos--;
    self->c_str[self->pos] = 1;
    }
  }

void buffer_set (Buffer *self, const char *s)
  {
  int l = strlen (s);
  if (l > self->length) l = self->length;
  strncpy (self->c_str, s, l);
  self->c_str[l] = 0; // Ensure terminated, because strncpy does not
  self->pos = l;
  }


