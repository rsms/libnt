/**
 This code is released in the Public Domain (no restrictions, no support
 100% free) by Notion.
*/
#include "../src/buffer.h"

int main (int argc, char const *argv[]) {
  nt_buffer_t *b;
  
  b = nt_buffer_new(0, 0);
  assert(nt_buffer_occupied(b) == 0);
  nt_buffer_appends(b, "hello hello\n");
  assert(nt_buffer_occupied(b) == 12);
  nt_buffer_appends(b, "mos korv\n");
  assert(nt_buffer_occupied(b) == 21);
  nt_buffer_appendc(b, '\0');
  assert(nt_buffer_occupied(b) == 22);
  
  // xxx todo assertions
  
  return 0;
}