#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "src/obj.h"

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
  
  nt_obj_init((nt_obj *)obj, &myobj_destructor); // new reference
  assert(obj->ntobj.refcount == 1);
  
  nt_getref(obj); // aquire a new reference
  assert(obj->ntobj.refcount == 2);
  printf("obj refcount is %d\n", nt_obj_get_refcount((nt_obj *)obj));
  
  nt_putref(obj); // put back one reference
  assert(obj->ntobj.refcount == 1);
  
  nt_putref(obj); // put back one reference -- should call the destructor
  
  // it is a fault to access obj beyond this point, as there are no longer any
  // references to obj and it might have been freed.
  
  nt_getref(obj); // emits a warning on stderr because refcount < 1
  
  return 0;
}