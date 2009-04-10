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
  assert(nt_array_get(a, 0) == b);
  
  nt_array_push(a, c);
  assert(nt_array_available(a) == 1);
  assert(nt_array_get(a, 0) == b);
  assert(nt_array_get(a, 1) == c);
  
  nt_array_push(a, b);
  assert(nt_array_available(a) == 0);
  assert(nt_array_get(a, 0) == b);
  assert(nt_array_get(a, 1) == c);
  assert(nt_array_get(a, 2) == b);
  
  nt_array_push(a, b);
  assert(nt_array_available(a) == 0);
  assert(nt_array_get(a, 0) == b);
  assert(nt_array_get(a, 1) == c);
  assert(nt_array_get(a, 2) == b);
  assert(nt_array_get(a, 3) == b);
  
  fprintf(stderr, "\nA => %p\n", a);
  fprintf(stderr, "B => %p\n", b);
  fprintf(stderr, "C => %p\n", c);
  #define C(p) ((void *)p==a?'A':((void *)p==(void *)b?'B':((void *)p==(void *)c?'C':'-'))), p
  #define P(i,p) fprintf(stderr, "%d => %c  %p\n", i, C(p));
  
  putc('\n', stderr);
  P(0,nt_array_get(a, 0));
  P(1, nt_array_get(a, 1));
  P(2, nt_array_get(a, 2));
  P(3, nt_array_get(a, 3));
  
  #define Z sizeof(void*)
  size_t i = 1;
  nt_array_t *self = a;
  putc('\n', stderr);
  void **pp = ((void **)((self)->start + (Z * (i))));
  P(0, *pp);
  
  assert(nt_array_get(a, 1) == c);
  //memcpy(pp, (void **)&b, Z);
  nt_array_set(self, 1, b);
  assert(nt_array_get(a, 1) == b);
  
  putc('\n', stderr);
  P(0,nt_array_get(a, 0));
  P(1, nt_array_get(a, 1));
  P(2, nt_array_get(a, 2));
  P(3, nt_array_get(a, 3));
  
  //((void *)((a)->start + (Z * i))) = (void **)&((void *)b);
  //assert(nt_array_get(a, 1) == b);
  
  return 0;
}