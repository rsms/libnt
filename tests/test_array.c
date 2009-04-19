/**
 This code is released in the Public Domain (no restrictions, no support
 100% free) by Notion.
*/
#include "../src/array.h"

int main (int argc, char const *argv[]) {
  nt_array_t *a, *b;
  char *c = "hej";
  
  a = nt_array_new(3, 0);
  b = nt_array_new(2, 0);
  
  assert(nt_array_length(a) == 0);
  assert(nt_array_length(b) == 0);
  assert(nt_array_available(a) == 3);
  assert(nt_array_available(b) == 2);
  
  nt_array_push(a, b);
  assert(nt_array_available(a) == 2);
  assert(nt_array_length(a) == 1);
  assert(nt_array_get(a, 0) == b);
  
  nt_array_push(a, c);
  assert(nt_array_available(a) == 1);
  assert(nt_array_length(a) == 2);
  assert(nt_array_get(a, 0) == b);
  assert(nt_array_get(a, 1) == c);
  
  nt_array_push(a, b);
  assert(nt_array_available(a) == 0);
  assert(nt_array_length(a) == 3);
  assert(nt_array_get(a, 0) == b);
  assert(nt_array_get(a, 1) == c);
  assert(nt_array_get(a, 2) == b);
  
  nt_array_push(a, b);
  assert(nt_array_available(a) == 0);
  assert(nt_array_length(a) == 4);
  assert(nt_array_get(a, 0) == b);
  assert(nt_array_get(a, 1) == c);
  assert(nt_array_get(a, 2) == b);
  assert(nt_array_get(a, 3) == b);
  
  char *d="D", *e="E", *f="F", *g="G";
  
  nt_array_set(a, 0, a);
  nt_array_set(a, 1, b);
  nt_array_set(a, 2, c);
  nt_array_set(a, 3, d);
  nt_array_push(a, e);
  nt_array_push(a, f);
  nt_array_push(a, g);
  assert(nt_array_length(a) == 7);
  
  assert(nt_array_get(a, 0) == a);
  assert(nt_array_get(a, 1) == b);
  assert(nt_array_get(a, 2) == c);
  assert(nt_array_get(a, 3) == d);
  assert(nt_array_get(a, 4) == e);
  assert(nt_array_get(a, 5) == f);
  assert(nt_array_get(a, 6) == g);
  assert(nt_array_length(a) == 7);
  
  nt_array_del(a, 2, 2);
  assert(nt_array_length(a) == 5);
  
  assert(nt_array_get(a, 0) == a);
  assert(nt_array_get(a, 1) == b);
  assert(nt_array_get(a, 2) == e);
  assert(nt_array_get(a, 3) == f);
  assert(nt_array_get(a, 4) == g);
  // accessing these for testing purposes, which are normally invalid:
  assert(nt_array_get(a, 5) == NULL);
  assert(nt_array_get(a, 6) == NULL);
  
  assert(nt_array_indexof(a, a) == 0);
  assert(nt_array_indexof(a, b) == 1);
  assert(nt_array_indexof(a, c) == -1);
  assert(nt_array_indexof(a, d) == -1);
  assert(nt_array_indexof(a, e) == 2);
  assert(nt_array_indexof(a, f) == 3);
  assert(nt_array_indexof(a, g) == 4);
  
  return 0;
}
