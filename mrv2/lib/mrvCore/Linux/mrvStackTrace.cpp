

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>

void printStackTrace()
{
    void* callstack[128];
    int frames = backtrace(callstack, 128);
    char** symbols = backtrace_symbols(callstack, frames);

    for (int i = 0; i < frames; i++) {
        printf("frame %d: ", i);

        // Use addr2line and c++filt to get the demangled symbol and line number
		char exe[256];
		snprintf( exe, 256, "%s/bin/mrv2", getenv( "MRV_ROOT" ) );
		
        char command[1024];
        snprintf(command, sizeof(command), "addr2line -e %s %p | c++filt",
				 exe, callstack[i]);
        FILE* fp = popen(command, "r");

        char output[512];
        fgets(output, sizeof(output), fp);
        printf("%s", output);

        pclose(fp);

        // Print the raw symbol as well
        printf("         %s\n", symbols[i]);
    }

    free(symbols);
}
