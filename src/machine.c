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