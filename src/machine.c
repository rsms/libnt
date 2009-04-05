/**
  Copyright (c) 2009 Notion <http://notion.se/>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/
#include "machine.h"
#include <sys/sysctl.h>
#include <err.h>

#ifdef __APPLE__
#include <mach/machine.h>
#endif

int nt_machine_ncpu(int *fake64) {
  int ncpu = 0;
  *fake64 = 0;
  
  #ifdef __APPLE__
    size_t len = sizeof(int);
    int ret = 0, cputype = 0, cpusubtype = 0;
    if((ret = sysctlbyname("hw.ncpu", &ncpu, &len, NULL, 0)) != 0) {
      return -1;
    }
    if((ret = sysctlbyname("hw.cputype",    &cputype,    &len, NULL, 0)) != 0) {
      return -1;
    }
    if((ret = sysctlbyname("hw.cpusubtype", &cpusubtype, &len, NULL, 0)) != 0) {
      return -1;
    }
    *fake64 = (cputype == CPU_TYPE_POWERPC) && (cpusubtype == CPU_SUBTYPE_POWERPC_970) ? 1 : 0;
  #else
    /* todo: other systems */
    #warning Unsupported system -- defaulting to 32-bit mp
    ncpu = 2;
  #endif
  
  return ncpu;
}