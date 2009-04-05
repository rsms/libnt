/**
 This code is released in the Public Domain (no restrictions, no support
 100% free) by Notion.
*/
#include "../src/obj.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

typedef struct myobj {
  NT_OBJ_HEAD
  int myint;
} myobj;

void myobj_destructor(nt_obj *obj) {
  printf("myobj_destructor called with obj @ %p\n", obj);
  //free(obj);
}

int main (int argc, char const *argv[]) {
  myobj *obj;
  obj = malloc(sizeof(myobj));
  
  // initialize an object
  nt_obj_init((nt_obj *)obj, &myobj_destructor); // new reference
  assert(nt_obj_get_refcount((nt_obj *)obj) == 1);
  
  // get an extra reference
  nt_getref(obj);
  assert(nt_obj_get_refcount((nt_obj *)obj) == 2);
  printf("obj refcount is %d\n", nt_obj_get_refcount((nt_obj *)obj));
  
  // swap
  nt_obj *old = nt_obj_swap((nt_obj * volatile *)&obj, NULL);
  assert(obj == NULL); // obj was replaced and is not NULL
  assert(nt_obj_get_refcount(old) == 1); // obj was released by nt_obj_swap
  old = nt_obj_swap((nt_obj * volatile *)&obj, old);
  assert(obj != NULL);
  assert(old == NULL);
  assert(nt_obj_get_refcount((nt_obj *)obj) == 2); // obj was retained by nt_obj_swap
  
  // release our extra reference
  nt_putref(obj); // put back one reference
  assert(nt_obj_get_refcount((nt_obj *)obj) == 1);
  
  // release the reference retained by nt_obj_init
  nt_putref(obj); // put back one reference -- should call the destructor
  
  // note:
  // it is a fault to access obj beyond this point, as there are no longer any
  // references to obj and it might have been freed.
  
  // try the warning mechanism
  nt_getref(obj); // emits a warning on stderr because refcount < 1
  
  return 0;
}