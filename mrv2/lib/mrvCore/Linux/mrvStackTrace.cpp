
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <backtrace.h>

#include <iostream>

void error_callback(void* data, const char* msg, int errnum)
{
    fprintf(stderr, "%s - Error %d\n", msg, errnum);
}

int full_callback(
    void* data, uintptr_t pc, const char* filename, int lineno,
    const char* function)
{
    char demangled_name[1024];
    demangled_name[0] = '\0';

    
    char buffer[512];
    if (!filename || !function || strcmp(function, "(null)") == 0)
        return 0;
    
    snprintf(buffer, sizeof(buffer), "c++filt %s", function);
    FILE* pipe = popen(buffer, "r");
    if (!pipe) {
        perror("popen");
        return 1;
    }
    char* ret = fgets(demangled_name, sizeof(demangled_name), pipe);
    pclose(pipe);

    printf("0x%lx %s (%s:%d)\n", pc, demangled_name, filename, lineno);
    return 0;
}

void printStackTrace()
{
  // MRV2_ROOT contains the root path of the executable.
  char exe[1024];
  snprintf(exe, 1024, "%s/bin/mrv2", getenv("MRV2_ROOT"));

  auto state = backtrace_create_state(exe, 1, error_callback, nullptr);
  int ret = backtrace_full(state, 0, full_callback, error_callback, nullptr);
  
}

