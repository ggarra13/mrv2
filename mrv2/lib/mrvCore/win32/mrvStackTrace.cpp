
#include <windows.h>

#pragma warning(push)
#pragma warning(disable : 4091) // Disable warning due to a bug in dbghelp.h

#include <dbghelp.h>

#pragma warning(pop)

#include <iostream>
#include <fstream>

#include "mrvCore/mrvHome.h"

void printStackTrace()
{
    // Set up the symbol options so that we can gather information from the
    // current executable's PDB files, as well as the Microsoft symbol servers.
    // We also want to undecorate the symbol names we're returned.

    ::SymSetOptions(
        SYMOPT_DEFERRED_LOADS | SYMOPT_INCLUDE_32BIT_MODULES | SYMOPT_UNDNAME);
    if (!::SymInitialize(
            ::GetCurrentProcess(), "http://msdl.microsoft.com/download/symbols",
            TRUE))
        return;

    const int max_depth = 32;
    void* stack_addrs[max_depth];
    unsigned short frames =
        CaptureStackBackTrace(1, max_depth, stack_addrs, nullptr);
    SYMBOL_INFO* symbol =
        (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    // Crashes saved to %TMP%.  Usually C:\Users\%User%\AppData\Local\Temp
    std::ofstream file(mrv::tmppath() + "/mrv2.crash.log");
    
    for (int i = 0; i < frames; i++)
    {
        DWORD64 address = (DWORD64)(stack_addrs[i]);

        // Get symbol name
        SymFromAddr(GetCurrentProcess(), address, nullptr, symbol);

        // Print information with or without line number
        // Attempt to get line number information
        DWORD displacement;
        IMAGEHLP_LINE64 line;
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
        if (SymGetLineFromAddr64(
                GetCurrentProcess(), address, &displacement, &line))
        {
            std::cout << i << ": " << symbol->Name << " - 0x" << symbol->Address
                      << " (" << line.FileName << ":" << line.LineNumber << ")"
                      << std::endl;
            file << i << ": " << symbol->Name << " - 0x" << symbol->Address
                 << " (" << line.FileName << ":" << line.LineNumber << ")"
                 << std::endl;
        }
        else
        {
            std::cout << i << ": " << symbol->Name << " - 0x" << symbol->Address
                      << " (line number unavailable)" << std::endl;
            file << i << ": " << symbol->Name << " - 0x" << symbol->Address
                 << " (line number unavailable)" << std::endl;
        }
    }
    
    file.close();

    free(symbol);
    ::SymCleanup(GetCurrentProcess());
}
