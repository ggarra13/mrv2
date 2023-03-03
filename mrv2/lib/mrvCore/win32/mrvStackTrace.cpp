
#include <windows.h>

#pragma warning( push )
#pragma warning( disable: 4091 )  // Disable warning due to a bug in dbghelp.h

#include <dbghelp.h>

#pragma warning( pop )

#include <iostream>

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
    for (int i = 0; i < frames; i++)
    {
        DWORD64 address = (DWORD64)(stack_addrs[i]);
        SymFromAddr(GetCurrentProcess(), address, nullptr, symbol);
        std::cout << i << ": " << symbol->Name << " - 0x" << symbol->Address
                  << std::endl;
    }
    free(symbol);

    ::SymCleanup(::GetCurrentProcess());
}
