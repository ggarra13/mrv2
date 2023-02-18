// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvOS.h"
#include "mrvCPU.h"

#if defined(WIN32) || defined(_WIN64)
#    include <windows.h>
#    include <intrin.h>
#else
#    include <unistd.h>
#endif

#include <stdint.h>

#if defined(__i386__) || defined(_M_IX86)
#    define ARCH_X86
#endif

#if defined(_M_X64) || defined(__x86_64__)
#    define ARCH_X86_64
#endif

#ifdef ARCH_X86_64
#    define OPSIZE "q"
#    define REG_a "rax"
#    define REG_b "rbx"
#    define REG_c "rcx"
#    define REG_d "rdx"
#    define REG_D "rdi"
#    define REG_S "rsi"
#    define PTR_SIZE "8"

typedef int64_t x86_reg;

#    define REG_SP "rsp"
#    define REG_BP "rbp"
#    define REGBP rbp
#    define REGa rax
#    define REGb rbx
#    define REGc rcx
#    define REGd rdx
#    define REGSP rsp

#elif defined(ARCH_X86)

#    define OPSIZE "l"
#    define REG_a "eax"
#    define REG_b "ebx"
#    define REG_c "ecx"
#    define REG_d "edx"
#    define REG_D "edi"
#    define REG_S "esi"
#    define PTR_SIZE "4"

typedef int32_t x86_reg;

#    define REG_SP "esp"
#    define REG_BP "ebp"
#    define REGBP ebp
#    define REGa eax
#    define REGb ebx
#    define REGc ecx
#    define REGd edx
#    define REGSP esp
#else

typedef int x86_reg;

#endif

CpuCaps gCpuCaps;

#include <stdlib.h>

#include <iostream>
#include <sstream>

#define LOG_ERROR(x) out << x << std::endl
#define LOG_WARNING(x) out << x << std::endl
#define LOG_INFO(x) out << x << std::endl
#define LOG_VERBOSE(x) // std::cout << x << std::endl

/* returned value is malloc()'ed so free() it after use */
char* GetCpuFriendlyName(
    std::ostringstream& out, unsigned int regs[], unsigned int regs2[]);

#if defined(ARCH_X86) || defined(ARCH_X86_64)

#    include <stdio.h>
#    include <string.h>

#    if defined(__NetBSD__) || defined(__OpenBSD__)
#        include <sys/param.h>
#        include <sys/sysctl.h>
#        include <machine/cpu.h>
#    endif

#    if defined(__FreeBSD__) || defined(__DragonFly__)
#        include <sys/types.h>
#        include <sys/sysctl.h>
#    endif

#    ifdef __linux__
#        include <signal.h>
#    endif

#    ifdef WIN32
#        include <windows.h>
#    endif

#    ifdef __AMIGAOS4__
#        include <proto/exec.h>
#    endif

//#define X86_FXSR_MAGIC
/* Thanks to the FreeBSD project for some of this cpuid code, and
 * help understanding how to use it.  Thanks to the Mesa
 * team for SSE support detection and more cpu detect code.
 */

/* I believe this code works.  However, it has only been used on a PII and PIII
 */

static void check_os_katmai_support(void);

// return TRUE if cpuid supported
static bool has_cpuid(void)
{
#    ifdef ARCH_X86_64
    return true;
#    elif defined(_MSC_VER)
    unsigned long BitChanged;

    // We've to check if we can toggle the flag register bit 21
    // If we can't the processor does not support the CPUID command
    __asm
    {
      pushfd;
      pop REGa;
      mov REGb, REGa;
      xor REGa, 0x00200000;
      push REGa;
      popfd;
      pushfd;
      pop REGa;
      xor REGa, REGb;
      mov BitChanged, REGa;
    }

    return ((BitChanged) ? true : false);
#    else
    // code from libavcodec:
    long a, c;
    __asm __volatile(
        /* See if CPUID instruction is supported ... */
        /* ... Get copies of EFLAGS into eax and ecx */
        "pushf\n\t"
        "pop %0\n\t"
        "mov %0, %1\n\t"

        /* ... Toggle the ID bit in one copy and store */
        /*     to the EFLAGS reg */
        "xor $0x200000, %0\n\t"
        "push %0\n\t"
        "popf\n\t"

        /* ... Get the (hopefully modified) EFLAGS */
        "pushf\n\t"
        "pop %0\n\t"
        : "=a"(a), "=c"(c)
        :
        : "cc");

    return (a != c);
#    endif
}

// typedef struct cpuid_args_s {
//   unsigned int eax;
//   unsigned int ebx;
//   unsigned int ecx;
//   unsigned int edx;
// } CPUID_ARGS;

// #ifndef _M_X64
// void do_cpuid2(CPUID_ARGS* p) {
// #ifdef _MSC_VER
//      __asm {
//              mov	edi, p
//              mov eax, [edi].eax
//              mov ecx, [edi].ecx // for functions such as eax=4
//              cpuid
//              mov [edi].eax, eax
//              mov [edi].ebx, ebx
//              mov [edi].ecx, ecx
//              mov [edi].edx, edx
//      }
// #else
//   __asm __volatile (
//                  "mov %%"REG_b", %%"REG_S"\n\t"
//                  "cpuid\n\t"
//                  "xchg %%"REG_b", %%"REG_S
//                  : "=a" ( p.eax ), "=S" ( p.ebx ),
//                  "=c" ( p.ecx ), "=d" ( p.edx )
//                  : "0" ( p.eax )
//                  );
// #endif
// }
// #endif

static void do_cpuid(unsigned int eaxval, unsigned int* eregs)
{
#    ifdef _WIN64
    __cpuid((int*)eregs, eaxval);
    return;
#    elif defined(_MSC_VER)
    __asm {
    mov	edi, eregs
    mov REGa, eaxval
    mov REGc, 0
    cpuid
    mov [edi],      REGa
    mov [edi] + 4,  REGb
    mov [edi] + 8,  REGc
    mov [edi] + 12, REGd
    }
#    else
    // code from libavcodec:
    __asm __volatile("mov %%" REG_b ", %%" REG_S "\n\t"
                     "cpuid\n\t"
                     "xchg %%" REG_b ", %%" REG_S
                     : "=a"(eregs[0]), "=S"(eregs[1]), "=c"(eregs[2]),
                       "=d"(eregs[3])
                     : "0"(eaxval));
#    endif // _MSC_VER
}

std::string GetCpuCaps(CpuCaps* caps)
{
    std::ostringstream out;

    unsigned int regs[4];
    unsigned int regs2[4];
    memset(regs, 0, sizeof(unsigned int) * 4);
    memset(regs2, 0, sizeof(unsigned int) * 4);

    memset(caps, 0, sizeof(*caps));
    caps->isX86   = 1;
    caps->cl_size = 32; /* default */
    if (!has_cpuid())
    {
        LOG_WARNING("CPUID not supported - (old 486?)");
        return out.str();
    }
    do_cpuid(0x00000000, regs); // get _max_ cpuid level and vendor name

    char buf[1024];
    snprintf(
        buf, 1023, "CPU vendor name: %x %x %x %x", regs[1], regs[3], regs[2],
        regs[0]);
    LOG_VERBOSE(buf);

    snprintf(
        buf, 1023, "CPU vendor name: %.4s%.4s%.4s  max cpuid level: %d",
        (char*)(regs + 1), (char*)(regs + 3), (char*)(regs + 2), regs[0]);
    LOG_VERBOSE(buf);

    if (regs[0] >= 0x00000001)
    {
        char *tmpstr, *ptmpstr;
        unsigned cl_size;

        do_cpuid(0x00000001, regs2);

        caps->cpuType  = (regs2[0] >> 8) & 0xf;
        caps->cpuModel = (regs2[0] >> 4) & 0xf;

        // see AMD64 Architecture Programmer's Manual, Volume 3: General-purpose
        // and System Instructions, Table 3-2: Effective family computation,
        // page 120.
        if (caps->cpuType == 0xf)
        {
            // use extended family (P4, IA64, K8)
            caps->cpuType = 0xf + ((regs2[0] >> 20) & 255);
        }
        if (caps->cpuType == 0xf || caps->cpuType == 6)
            caps->cpuModel |= ((regs2[0] >> 16) & 0xf) << 4;

        caps->cpuStepping = regs2[0] & 0xf;

        // general feature flags:
        caps->hasTSC   = (regs2[3] & (1 << 8)) >> 8;   // 0x0000010
        caps->hasMMX   = (regs2[3] & (1 << 23)) >> 23; // 0x0800000
        caps->hasSSE   = (regs2[3] & (1 << 25)) >> 25; // 0x2000000
        caps->hasSSE2  = (regs2[3] & (1 << 26)) >> 26; // 0x4000000
        caps->hasSSE3  = (regs2[2] & 1);
        caps->hasSSSE3 = (regs2[2] & 0x00000200);
        caps->hasSSE4  = (regs2[2] & 0x00080000);
        caps->hasSSE42 = (regs2[2] & 0x00100000);
        caps->hasAESNI = (regs2[2] & 0x02000000);
        caps->hasMMX2  = caps->hasSSE; // SSE cpus supports mmxext too
        cl_size        = ((regs2[1] >> 8) & 0xFF) * 8;
        if (cl_size)
            caps->cl_size = cl_size;

        ptmpstr = tmpstr = GetCpuFriendlyName(out, regs, regs2);
        while (*ptmpstr == ' ') // strip leading spaces
            ptmpstr++;
        LOG_INFO(ptmpstr);
        free(tmpstr);

        LOG_INFO(
            "(Family: " << caps->cpuType << ", Model: " << caps->cpuModel
                        << ", Stepping: " << caps->cpuStepping << ")");
    }
    do_cpuid(0x80000000, regs);
    if (regs[0] >= 0x80000001)
    {
        LOG_VERBOSE("extended cpuid-level: " << (regs[0] & 0x7FFFFFFF));
        do_cpuid(0x80000001, regs2);
        caps->hasMMX |= (regs2[3] & (1 << 23)) >> 23;     // 0x0800000
        caps->hasMMX2 |= (regs2[3] & (1 << 22)) >> 22;    // 0x400000
        caps->has3DNow    = (regs2[3] & (1 << 31)) >> 31; // 0x80000000
        caps->has3DNowExt = (regs2[3] & (1 << 30)) >> 30;
    }
    if (regs[0] >= 0x80000006)
    {
        do_cpuid(0x80000006, regs2);
        LOG_VERBOSE("extended cache-info: " << (regs2[2] & 0x7FFFFFFF));
        caps->cl_size = regs2[2] & 0xFF;
    }

    LOG_VERBOSE("Detected cache-line size is " << caps->cl_size << " bytes");

    /* FIXME: Does SSE2 need more OS support, too? */
#    if defined(WIN32) || defined(WIN64) || defined(__linux__)                 \
        || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__CYGWIN__)  \
        || defined(__OpenBSD__) || defined(__DragonFly__)                      \
        || defined(__APPLE__)
    if (caps->hasSSE)
        check_os_katmai_support();
    if (!caps->hasSSE)
        caps->hasSSE2 = 0;
#    endif

    //
    // List CPU capabilities
    //
    if (gCpuCaps.hasMMX)
        out << "MMX ";
    if (gCpuCaps.hasMMX2)
        out << "MMX2 ";
    if (gCpuCaps.has3DNow)
        out << "3DNow ";
    if (gCpuCaps.has3DNowExt)
        out << "3DNow2 ";
    if (gCpuCaps.hasSSE)
        out << "SSE ";
    if (gCpuCaps.hasSSE2)
        out << "SSE2 ";
    if (gCpuCaps.hasSSE3)
        out << "SSE3 ";
    if (gCpuCaps.hasSSSE3)
        out << "SSSE3 ";
    if (gCpuCaps.hasSSE4)
        out << "SSE4 ";
    if (gCpuCaps.hasSSE42)
        out << "SSE42 ";
    if (gCpuCaps.hasAESNI)
        out << "AESNI ";
    if (gCpuCaps.hasAltiVec)
        out << "Altivec ";
    out << std::endl;

    return out.str();
}

#    define CPUID_EXTFAMILY ((regs2[0] >> 20) & 0xFF) /* 27..20 */
#    define CPUID_EXTMODEL ((regs2[0] >> 16) & 0x0F)  /* 19..16 */
#    define CPUID_TYPE ((regs2[0] >> 12) & 0x04)      /* 13..12 */
#    define CPUID_FAMILY ((regs2[0] >> 8) & 0x0F)     /* 11..08 */
#    define CPUID_MODEL ((regs2[0] >> 4) & 0x0F)      /* 07..04 */
#    define CPUID_STEPPING ((regs2[0] >> 0) & 0x0F)   /* 03..00 */

char* GetCpuFriendlyName(
    std::ostringstream& out, unsigned int regs[], unsigned int regs2[])
{
    char vendor[13];
    char* ret;
    unsigned int i;

    if (nullptr == (ret = (char*)malloc(256)))
    {
        std::cerr << "Not enough memory" << std::endl;
        exit(1);
    }
    ret[0] = '\0';

    snprintf(
        vendor, 13, "%.4s%.4s%.4s", (char*)(regs + 1), (char*)(regs + 3),
        (char*)(regs + 2));

    do_cpuid(0x80000000, regs);
    if (regs[0] >= 0x80000004)
    {
        // CPU has built-in namestring
        for (i = 0x80000002; i <= 0x80000004; i++)
        {
            do_cpuid(i, regs);
            strncat(ret, (char*)regs, 16);
        }
        return ret;
    }

    return ret;
}

#    undef CPUID_EXTFAMILY
#    undef CPUID_EXTMODEL
#    undef CPUID_TYPE
#    undef CPUID_FAMILY
#    undef CPUID_MODEL
#    undef CPUID_STEPPING

#    if defined(__linux__) && defined(_POSIX_SOURCE) && defined(X86_FXSR_MAGIC)
static void sigill_handler_sse(int signal, struct sigcontext sc)
{
    LOG_VERBOSE("SIGILL, ");

    /* Both the "xorps %%xmm0,%%xmm0" and "divps %xmm0,%%xmm1"
     * instructions are 3 bytes long.  We must increment the instruction
     * pointer manually to avoid repeated execution of the offending
     * instruction.
     *
     * If the SIGILL is caused by a divide-by-zero when unmasked
     * exceptions aren't supported, the SIMD FPU status and control
     * word will be restored at the end of the test, so we don't need
     * to worry about doing it here.  Besides, we may not be able to...
     */
    sc.eip += 3;

    gCpuCaps.hasSSE = 0;
}

static void sigfpe_handler_sse(int signal, struct sigcontext sc)
{
    LOG_VERBOSE("SIGFPE, ");

    if (sc.fpstate->magic != 0xffff)
    {
        /* Our signal context has the extended FPU state, so reset the
         * divide-by-zero exception mask and clear the divide-by-zero
         * exception bit.
         */
        sc.fpstate->mxcsr |= 0x00000200;
        sc.fpstate->mxcsr &= 0xfffffffb;
    } else
    {
        /* If we ever get here, we're completely hosed.
         */
        LOG_VERBOSE("SSE enabling test failed badly!");
    }
}
#    endif /* __linux__ && _POSIX_SOURCE && X86_FXSR_MAGIC */

#    if (                                                                      \
        (defined(__MINGW32__) || defined(__CYGWIN__) || defined(_MSC_VER))     \
        && !defined(ARCH_X86_64))
LONG CALLBACK win32_sig_handler_sse(EXCEPTION_POINTERS* ep)
{
    if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_ILLEGAL_INSTRUCTION)
    {
        LOG_VERBOSE("SIGILL, ");
        ep->ContextRecord->Eip += 3;
        gCpuCaps.hasSSE = 0;
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    return EXCEPTION_CONTINUE_SEARCH;
}
#    endif /* defined(__MINGW32__) || defined(__CYGWIN__) */

/* If we're running on a processor that can do SSE, let's see if we
 * are allowed to or not.  This will catch 2.4.0 or later kernels that
 * haven't been configured for a Pentium III but are running on one,
 * and RedHat patched 2.2 kernels that have broken exception handling
 * support for user space apps that do SSE.
 */

#    if defined(__FreeBSD__) || defined(__DragonFly__)
#        define SSE_SYSCTL_NAME "hw.instruction_sse"
#    elif defined(__APPLE__)
#        define SSE_SYSCTL_NAME "hw.optional.sse"
#    endif

static void check_os_katmai_support(void)
{
#    ifdef ARCH_X86_64
    gCpuCaps.hasSSE  = 1;
    gCpuCaps.hasSSE2 = 1;
#    elif defined(__FreeBSD__) || defined(__DragonFly__) || defined(__APPLE__)
    int has_sse = 0, ret;
    size_t len  = sizeof(has_sse);

    ret = sysctlbyname(SSE_SYSCTL_NAME, &has_sse, &len, NULL, 0);
    if (ret || !has_sse)
        gCpuCaps.hasSSE = 0;

#    elif defined(__NetBSD__) || defined(__OpenBSD__)
#        if __NetBSD_Version__ >= 105250000 || (defined __OpenBSD__)
    int has_sse, has_sse2, ret, mib[2];
    size_t varlen;

    mib[0] = CTL_MACHDEP;
    mib[1] = CPU_SSE;
    varlen = sizeof(has_sse);

    LOG_VERBOSE("Testing OS support for SSE... ");
    ret = sysctl(mib, 2, &has_sse, &varlen, NULL, 0);
    if (ret < 0 || !has_sse)
    {
        gCpuCaps.hasSSE = 0;
        LOG_VERBOSE("no!");
    } else
    {
        gCpuCaps.hasSSE = 1;
        LOG_VERBOSE("yes!");
    }

    mib[1] = CPU_SSE2;
    varlen = sizeof(has_sse2);

    LOG_VERBOSE("Testing OS support for SSE2... ");
    ret = sysctl(mib, 2, &has_sse2, &varlen, NULL, 0);
    if (ret < 0 || !has_sse2)
    {
        gCpuCaps.hasSSE2 = 0;
        LOG_VERBOSE("no!");
    } else
    {
        gCpuCaps.hasSSE2 = 1;
        LOG_VERBOSE("yes!");
    }
#        else
    gCpuCaps.hasSSE = 0;
    LOG_VERBOSE("No OS support for SSE, disabling to be safe.\n");
#        endif
#    elif defined(WIN32)
    LPTOP_LEVEL_EXCEPTION_FILTER exc_fil;
    if (gCpuCaps.hasSSE)
    {
        LOG_VERBOSE("Testing OS support for SSE... ");
        exc_fil = SetUnhandledExceptionFilter(win32_sig_handler_sse);
#        ifdef _MSC_VER
        __asm {
      xorps xmm0, xmm0;
        }
#        else
        __asm __volatile("xorps %xmm0, %xmm0");
#        endif
        SetUnhandledExceptionFilter(exc_fil);
        if (gCpuCaps.hasSSE)
            LOG_VERBOSE("yes.");
        else
            LOG_VERBOSE("no!");
    }
#    elif defined(__linux__)
#        if defined(_POSIX_SOURCE) && defined(X86_FXSR_MAGIC)
    struct sigaction saved_sigill;
    struct sigaction saved_sigfpe;

    /* Save the original signal handlers.
     */
    sigaction(SIGILL, NULL, &saved_sigill);
    sigaction(SIGFPE, NULL, &saved_sigfpe);

    signal(SIGILL, (void (*)(int))sigill_handler_sse);
    signal(SIGFPE, (void (*)(int))sigfpe_handler_sse);

    /* Emulate test for OSFXSR in CR4.  The OS will set this bit if it
     * supports the extended FPU save and restore required for SSE.  If
     * we execute an SSE instruction on a PIII and get a SIGILL, the OS
     * doesn't support Streaming SIMD Exceptions, even if the processor
     * does.
     */
    if (gCpuCaps.hasSSE)
    {
        LOG_VERBOSE("Testing OS support for SSE... ");

        //      __asm __volatile ("xorps %%xmm0, %%xmm0");
        __asm __volatile("xorps %xmm0, %xmm0");

        if (gCpuCaps.hasSSE)
        {
            LOG_VERBOSE("yes");
        } else
        {
            LOG_VERBOSE("no!");
        }
    }

    /* Emulate test for OSXMMEXCPT in CR4.  The OS will set this bit if
     * it supports unmasked SIMD FPU exceptions.  If we unmask the
     * exceptions, do a SIMD divide-by-zero and get a SIGILL, the OS
     * doesn't support unmasked SIMD FPU exceptions.  If we get a SIGFPE
     * as expected, we're okay but we need to clean up after it.
     *
     * Are we being too stringent in our requirement that the OS support
     * unmasked exceptions?  Certain RedHat 2.2 kernels enable SSE by
     * setting CR4.OSFXSR but don't support unmasked exceptions.  Win98
     * doesn't even support them.  We at least know the user-space SSE
     * support is good in kernels that do support unmasked exceptions,
     * and therefore to be safe I'm going to leave this test in here.
     */
    if (gCpuCaps.hasSSE)
    {
        LOG_VERBOSE("Testing OS support for SSE unmasked exceptions... ");

        //      test_os_katmai_exception_support();

        if (gCpuCaps.hasSSE)
        {
            LOG_VERBOSE("yes.\n");
        } else
        {
            LOG_VERBOSE("no!\n");
        }
    }

    /* Restore the original signal handlers.
     */
    sigaction(SIGILL, &saved_sigill, NULL);
    sigaction(SIGFPE, &saved_sigfpe, NULL);

    /* If we've gotten to here and the XMM CPUID bit is still set, we're
     * safe to go ahead and hook out the SSE code throughout Mesa.
     */
    if (gCpuCaps.hasSSE)
    {
        LOG_VERBOSE("Tests of OS support for SSE passed.\n");
    } else
    {
        LOG_VERBOSE("Tests of OS support for SSE failed!\n");
    }
#        else
    /* We can't use POSIX signal handling to test the availability of
     * SSE, so we disable it by default.
     */
    LOG_VERBOSE("Cannot test OS support for SSE, disabling to be safe.\n");
    gCpuCaps.hasSSE = 0;
#        endif /* _POSIX_SOURCE && X86_FXSR_MAGIC */
#    else
    /* Do nothing on other platforms for now.
     */
    LOG_VERBOSE("Cannot test OS support for SSE, leaving disabled.\n");
    gCpuCaps.hasSSE = 0;
#    endif /* __linux__ */
}
#else /* ARCH_X86 || ARCH_X86_64 */

#    ifdef SYS_DARWIN
#        include <sys/sysctl.h>
#    else
#        ifndef __AMIGAOS4__
#            include <signal.h>
#            include <setjmp.h>

static sigjmp_buf jmpbuf;
static volatile sig_atomic_t canjump = 0;

static void sigill_handler(int sig)
{
    if (!canjump)
    {
        signal(sig, SIG_DFL);
        raise(sig);
    }

    canjump = 0;
    siglongjmp(jmpbuf, 1);
}
#        endif //__AMIGAOS4__
#    endif

std::string GetCpuCaps(CpuCaps* caps)
{
    std::ostringstream out;

    caps->cpuType     = 0;
    caps->cpuModel    = 0;
    caps->cpuStepping = 0;
    caps->hasMMX      = 0;
    caps->hasMMX2     = 0;
    caps->has3DNow    = 0;
    caps->has3DNowExt = 0;
    caps->hasSSE      = 0;
    caps->hasSSE2     = 0;
    caps->isX86       = 0;
    caps->hasAltiVec  = 0;
#    ifdef HAVE_ALTIVEC
#        ifdef SYS_DARWIN
    /*
      rip-off from ffmpeg altivec detection code.
      this code also appears on Apple's AltiVec pages.
    */
    {
        int sels[2] = {CTL_HW, HW_VECTORUNIT};
        int has_vu  = 0;
        size_t len  = sizeof(has_vu);
        int err;

        err = sysctl(sels, 2, &has_vu, &len, NULL, 0);

        if (err == 0)
            if (has_vu != 0)
                caps->hasAltiVec = 1;
    }
#        else /* SYS_DARWIN */
#            ifdef __AMIGAOS4__
    ULONG result = 0;

    GetCPUInfoTags(GCIT_VectorUnit, &result, TAG_DONE);
    if (result == VECTORTYPE_ALTIVEC)
        caps->hasAltiVec = 1;
#            else
    /* no Darwin, do it the brute-force way */
    /* this is borrowed from the libmpeg2 library */
    {
        signal(SIGILL, sigill_handler);
        if (sigsetjmp(jmpbuf, 1))
        {
            signal(SIGILL, SIG_DFL);
        } else
        {
            canjump = 1;

            asm volatile("mtspr 256, %0\n\t"
                         "vand %%v0, %%v0, %%v0"
                         :
                         : "r"(-1));

            signal(SIGILL, SIG_DFL);
            caps->hasAltiVec = 1;
        }
    }
#            endif //__AMIGAOS4__
#        endif     /* SYS_DARWIN */
    LOG_INFO("AltiVec " << (caps->hasAltiVec ? "" : "not ") << "found");
#    endif         /* HAVE_ALTIVEC */

#    if defined(__ia64__) || defined(_M_IA64)
    LOG_INFO("CPU: Intel Itanium");
#    endif

#    ifdef ARCH_SPARC
    LOG_INFO("CPU: Sun Sparc");
#    endif

#    ifdef ARCH_ARMV4L
    LOG_INFO("CPU: ARM");
#    endif

#    if defined(__powerpc__) || defined(__ppc__) || defined(_M_PPC)
    LOG_INFO("CPU: PowerPC");
#    endif

#    if defined(__alpha__) || defined(_M_ALPHA)
    LOG_INFO("CPU: Digital Alpha");
#    endif

#    if defined(ARCH_SGI_MIPS) || defined(_M_MRX000)
    LOG_INFO("CPU: SGI MIPS");
#    endif

#    ifdef ARCH_PA_RISC
    LOG_INFO("CPU: Hewlett-Packard PA-RISC");
#    endif

#    ifdef ARCH_S390
    LOG_INFO("CPU: IBM S/390");
#    endif

#    ifdef ARCH_S390X
    LOG_INFO("CPU: IBM S/390X");
#    endif

#    ifdef ARCH_VAX
    LOG_INFO("CPU: Digital VAX");
#    endif

    return out.str();
}
#endif /* !ARCH_X86 */

unsigned int cpu_count()
{
#if defined(WIN32) || defined(WIN64)
#    ifndef _SC_NPROCESSORS_ONLN
    SYSTEM_INFO info;
    GetSystemInfo(&info);
#        define sysconf(a) info.dwNumberOfProcessors
#        define _SC_NPROCESSORS_ONLN
#    endif
#endif

#ifdef _SC_NPROCESSORS_ONLN
    return sysconf(_SC_NPROCESSORS_ONLN);
#else
#    error Could not determine number of CPUs for this OS.
#endif
}
