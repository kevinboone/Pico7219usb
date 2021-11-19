/*=========================================================================
 
  Pico7219

  buffer.h 

  Implements a fixed-size buffer of characters. Note that characters
  are represented as C chars, which are usually signed. However, the
  display module character set is 8-bit unsigned. I've decided to 
  work with (signed) char throughout the program, in spite of this,
  to avoid a load of ugly casts when compiling with a high warning
  level.

  (c)2021 Kevin Boone, GPL v3.0

  =========================================================================*/
#pragma once

typedef struct _Buffer
  {
  int length;
  int pos;
  char *c_str;
  } Buffer;

// 
// buffer_new
//
// Creates a new buffer of given length (not including terminating zero).
// Returns zero if out of memory
//
Buffer *buffer_new (int length);

// 
// buffer_destroy
//
// Cleans up an existing buffer
// 
void    buffer_destroy (Buffer *self);

// 
// buffer_apppend
//
// Append the character to the buffer. If the buffer is full, new characters
// are silently dropped, to protect against buffer overruns
//
void buffer_append (Buffer *self, char c);
 
// 
// buffer_reset
//
// Reset the buffer position to the start. Does not free any memory.
// 
void buffer_reset (Buffer *self);

// 
// buffer_backspace
//
// Move the input position back one space, if possible 
// 
void buffer_backspace (Buffer *self);

//
//  buffer_set
//  
//  Replaces the buffer with the new string, up to the maximum length.
//  If too long, the input string is silently truncated
//
void buffer_set (Buffer *self, const char *s);

