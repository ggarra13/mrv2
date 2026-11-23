// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvOS/mrvCPU.h"

#include <cstdint>
#include <cstring>
#include <sstream>
#include <string>
#include <thread>
#if !defined(_WIN32)
#    include <unistd.h>
#endif

//
// Platform / architecture detection.
//
#if defined(_M_X64) || defined(__x86_64__)
#    define MRV_ARCH_X86_64
#endif
#if defined(__i386__) || defined(_M_IX86)
#    define MRV_ARCH_X86_32
#endif
#if defined(MRV_ARCH_X86_64) || defined(MRV_ARCH_X86_32)
#    define MRV_ARCH_X86
#endif
#if defined(_M_ARM64) || defined(__aarch64__)
#    define MRV_ARCH_ARM64
#elif defined(_M_ARM) || defined(__arm__)
#    define MRV_ARCH_ARM32
#endif
#if defined(__powerpc__) || defined(__ppc__) || defined(_M_PPC)
#    define MRV_ARCH_PPC
#endif

#if defined(_WIN32)
#    ifndef NOMINMAX
#        define NOMINMAX
#    endif
#    include <windows.h>
#    if defined(_MSC_VER) && defined(MRV_ARCH_X86)
#        include <intrin.h>
#    endif
#endif

#if defined(__APPLE__)
#    include <sys/types.h>
#    include <sys/sysctl.h>
#endif

#if defined(__linux__) &&                                                      \
    (defined(MRV_ARCH_ARM64) || defined(MRV_ARCH_ARM32) ||                     \
     defined(MRV_ARCH_PPC))
#    include <sys/auxv.h>
#    if defined(MRV_ARCH_ARM64) || defined(MRV_ARCH_ARM32)
#        include <asm/hwcap.h>
#    endif
#endif

#if defined(MRV_ARCH_X86) && !defined(_MSC_VER)
#    include <cpuid.h>
#endif

CpuCaps gCpuCaps;

namespace
{
    // Collects human readable diagnostics, returned from GetCpuCaps().
    inline void log_line(std::ostringstream& out, const std::string& msg)
    {
        out << msg << '\n';
    }

    inline const char* yesno(int value)
    {
        return value ? "yes" : "no";
    }

    void reset_caps(CpuCaps* caps)
    {
        std::memset(caps, 0, sizeof(CpuCaps));
        caps->cl_size = 64; // typical cache line size
    }

#if defined(__APPLE__)
    // Reads an integer sysctl by name; returns 0 if it does not exist.
    int sysctl_int(const char* name)
    {
        int value = 0;
        size_t len = sizeof(value);
        if (sysctlbyname(name, &value, &len, nullptr, 0) != 0)
            return 0;
        return value;
    }
#endif

    //
    // x86 / x86-64
    //
#if defined(MRV_ARCH_X86)

    // regs[0..3] = EAX, EBX, ECX, EDX
    void do_cpuid(unsigned int leaf, unsigned int regs[4], unsigned int sub = 0)
    {
#    if defined(_MSC_VER)
        int r[4];
        __cpuidex(r, static_cast<int>(leaf), static_cast<int>(sub));
        for (int i = 0; i < 4; ++i)
            regs[i] = static_cast<unsigned int>(r[i]);
#    else
        __cpuid_count(leaf, sub, regs[0], regs[1], regs[2], regs[3]);
#    endif
    }

    bool has_cpuid()
    {
#    if defined(MRV_ARCH_X86_64) || defined(_MSC_VER)
        return true;
#    else
        return __get_cpuid_max(0, nullptr) != 0;
#    endif
    }

    std::string cpu_brand_string()
    {
        unsigned int regs[4] = {0, 0, 0, 0};
        do_cpuid(0x80000000, regs);
        if (regs[0] < 0x80000004)
            return std::string();

        char name[49];
        std::memset(name, 0, sizeof(name));
        for (unsigned int i = 0; i < 3; ++i)
        {
            do_cpuid(0x80000002 + i, regs);
            std::memcpy(name + i * 16, regs, 16);
        }

        const char* p = name;
        while (*p == ' ') // strip leading spaces
            ++p;
        return p;
    }

    // Whether the OS actually enables SSE/SSE2 (only really relevant for
    // 32-bit Windows; every other supported OS enables them on SSE CPUs).
    void check_os_sse_support(CpuCaps* caps)
    {
#    if defined(MRV_ARCH_X86_64)
        caps->hasSSE = 1;
        caps->hasSSE2 = 1;
#    else
#        if defined(_WIN32)
        if (!IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE))
            caps->hasSSE = 0;
        if (!IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE))
            caps->hasSSE2 = 0;
#        endif
        if (!caps->hasSSE)
            caps->hasSSE2 = 0;
#    endif
    }

    void detect_x86(CpuCaps* caps, std::ostringstream& out)
    {
        caps->isX86 = 1;
        caps->cl_size = 32; // default

        if (!has_cpuid())
        {
            log_line(out, "CPUID not supported - (old 486?)");
            return;
        }

        unsigned int regs[4] = {0, 0, 0, 0};
        unsigned int regs2[4] = {0, 0, 0, 0};

        do_cpuid(0x00000000, regs); // max cpuid level and vendor name

        if (regs[0] >= 0x00000001)
        {
            do_cpuid(0x00000001, regs2);

            caps->cpuType = (regs2[0] >> 8) & 0xf;
            caps->cpuModel = (regs2[0] >> 4) & 0xf;

            // See AMD64 Architecture Programmer's Manual, Volume 3,
            // Table 3-2: Effective family computation.
            if (caps->cpuType == 0xf)
                caps->cpuType = 0xf + ((regs2[0] >> 20) & 0xff);
            if (caps->cpuType == 0xf || caps->cpuType == 6)
                caps->cpuModel |= ((regs2[0] >> 16) & 0xf) << 4;

            caps->cpuStepping = regs2[0] & 0xf;

            // General feature flags.
            caps->hasTSC = (regs2[3] >> 8) & 1;
            caps->hasMMX = (regs2[3] >> 23) & 1;
            caps->hasSSE = (regs2[3] >> 25) & 1;
            caps->hasSSE2 = (regs2[3] >> 26) & 1;
            caps->hasSSE3 = (regs2[2] & 0x00000001) != 0;
            caps->hasSSSE3 = (regs2[2] & 0x00000200) != 0;
            caps->hasSSE4 = (regs2[2] & 0x00080000) != 0;
            caps->hasSSE42 = (regs2[2] & 0x00100000) != 0;
            caps->hasAESNI = (regs2[2] & 0x02000000) != 0;
            caps->hasMMX2 = caps->hasSSE; // SSE CPUs support MMXEXT too

            const unsigned int cl_size = ((regs2[1] >> 8) & 0xFF) * 8;
            if (cl_size)
                caps->cl_size = static_cast<int>(cl_size);

            const std::string brand = cpu_brand_string();
            if (!brand.empty())
                log_line(out, brand);

            std::ostringstream fam;
            fam << "(Family: " << caps->cpuType << ", Model: " << caps->cpuModel
                << ", Stepping: " << caps->cpuStepping << ")";
            log_line(out, fam.str());
        }

        do_cpuid(0x80000000, regs);
        if (regs[0] >= 0x80000001)
        {
            do_cpuid(0x80000001, regs2);
            caps->hasMMX |= (regs2[3] >> 23) & 1;
            caps->hasMMX2 |= (regs2[3] >> 22) & 1;
            caps->has3DNow = (regs2[3] >> 31) & 1;
            caps->has3DNowExt = (regs2[3] >> 30) & 1;
        }
        if (regs[0] >= 0x80000006)
        {
            do_cpuid(0x80000006, regs2);
            const int l2_line = regs2[2] & 0xFF;
            if (l2_line)
                caps->cl_size = l2_line;
        }

        check_os_sse_support(caps);

        if (caps->hasMMX)
            out << "MMX ";
        if (caps->hasMMX2)
            out << "MMX2 ";
        if (caps->has3DNow)
            out << "3DNow ";
        if (caps->has3DNowExt)
            out << "3DNow2 ";
        if (caps->hasSSE)
            out << "SSE ";
        if (caps->hasSSE2)
            out << "SSE2 ";
        if (caps->hasSSE3)
            out << "SSE3 ";
        if (caps->hasSSSE3)
            out << "SSSE3 ";
        if (caps->hasSSE4)
            out << "SSE4 ";
        if (caps->hasSSE42)
            out << "SSE42 ";
        if (caps->hasAESNI)
            out << "AESNI ";
        out << '\n';
    }

#endif // MRV_ARCH_X86

    //
    // ARM (64 and 32 bit)
    //
#if defined(MRV_ARCH_ARM64) || defined(MRV_ARCH_ARM32)

    void print_arm_caps(const CpuCaps* caps, std::ostringstream& out)
    {
        log_line(out, std::string("NEON: ") + yesno(caps->hasNEON));
        log_line(out, std::string("ARMv8-A: ") + yesno(caps->hasARMv8));
        log_line(
            out, std::string("ARMv8-A Crypto: ") + yesno(caps->hasARMv8Crypto));
        log_line(
            out, std::string("ARMv8-A CRC32: ") + yesno(caps->hasARMv8Crc));
        log_line(
            out,
            std::string("ARMv8.2-A Dotprod: ") + yesno(caps->hasARMv8Dotprod));
        log_line(
            out, std::string("ARMv8.2-A FP16: ") + yesno(caps->hasARMv8Fp16));
        log_line(out, std::string("SVE: ") + yesno(caps->hasSVE));
        log_line(out, std::string("SVE2: ") + yesno(caps->hasSVE2));

        if (caps->hasNEON)
            out << "NEON ";
        if (caps->hasARMv8)
            out << "ARMv8-A ";
        if (caps->hasARMv8Crypto)
            out << "ARMv8-A-Crypto ";
        if (caps->hasARMv8Crc)
            out << "ARMv8-A-CRC32 ";
        if (caps->hasARMv8Dotprod)
            out << "ARMv8.2-A-Dotprod ";
        if (caps->hasARMv8Fp16)
            out << "ARMv8.2-A-FP16 ";
        if (caps->hasSVE)
            out << "SVE ";
        if (caps->hasSVE2)
            out << "SVE2 ";
        out << '\n';
    }

    void detect_arm(CpuCaps* caps, std::ostringstream& out)
    {
#    if defined(MRV_ARCH_ARM64)
        log_line(out, "CPU: ARM64");
        caps->hasARMv8 = 1; // every AArch64 CPU is ARMv8-A or later
        caps->hasNEON = 1;  // Advanced SIMD is mandatory on AArch64
#    else
        log_line(out, "CPU: ARM (32-bit)");
#    endif

#    if defined(_WIN32)
        // Note: PF_ARM_VFP_32_REGISTERS_AVAILABLE is *not* NEON.
        caps->hasNEON =
            IsProcessorFeaturePresent(PF_ARM_NEON_INSTRUCTIONS_AVAILABLE) != 0;
#        ifdef PF_ARM_V8_INSTRUCTIONS_AVAILABLE
        caps->hasARMv8 =
            IsProcessorFeaturePresent(PF_ARM_V8_INSTRUCTIONS_AVAILABLE) != 0;
#        endif
#        ifdef PF_ARM_V8_CRYPTO_INSTRUCTIONS_AVAILABLE
        caps->hasARMv8Crypto =
            IsProcessorFeaturePresent(PF_ARM_V8_CRYPTO_INSTRUCTIONS_AVAILABLE) !=
            0;
#        endif
        // Newer SDK flags: guard so older SDKs still build.
#        ifdef PF_ARM_V8_CRC32_INSTRUCTIONS_AVAILABLE
        caps->hasARMv8Crc =
            IsProcessorFeaturePresent(PF_ARM_V8_CRC32_INSTRUCTIONS_AVAILABLE) !=
            0;
#        endif
#        ifdef PF_ARM_V82_DP_INSTRUCTIONS_AVAILABLE
        caps->hasARMv8Dotprod =
            IsProcessorFeaturePresent(PF_ARM_V82_DP_INSTRUCTIONS_AVAILABLE) !=
            0;
#        endif
#        ifdef PF_ARM_V82_FP16_INSTRUCTIONS_AVAILABLE
        caps->hasARMv8Fp16 =
            IsProcessorFeaturePresent(PF_ARM_V82_FP16_INSTRUCTIONS_AVAILABLE) !=
            0;
#        endif
        // No SVE/SVE2 detection on Windows.

#    elif defined(__APPLE__)
        caps->hasNEON = 1;
        caps->hasARMv8 = 1;
        caps->hasARMv8Crypto = sysctl_int("hw.optional.arm.FEAT_AES") != 0;
        caps->hasARMv8Crc = sysctl_int("hw.optional.armv8_crc32") != 0;
        caps->hasARMv8Dotprod = sysctl_int("hw.optional.arm.FEAT_DotProd") != 0;
        caps->hasARMv8Fp16 = sysctl_int("hw.optional.neon_fp16") != 0 ||
                             sysctl_int("hw.optional.arm.FEAT_FP16") != 0;
        const int line = sysctl_int("hw.cachelinesize");
        if (line > 0)
            caps->cl_size = line;

#    elif defined(__linux__)
        const unsigned long hwcap = getauxval(AT_HWCAP);
#        if defined(MRV_ARCH_ARM64)
        caps->hasNEON = (hwcap & HWCAP_ASIMD) != 0;
#            ifdef HWCAP_AES
        caps->hasARMv8Crypto = (hwcap & (HWCAP_AES | HWCAP_PMULL)) != 0;
#            endif
#            ifdef HWCAP_CRC32
        caps->hasARMv8Crc = (hwcap & HWCAP_CRC32) != 0;
#            endif
#            ifdef HWCAP_ASIMDDP
        caps->hasARMv8Dotprod = (hwcap & HWCAP_ASIMDDP) != 0;
#            endif
#            if defined(HWCAP_FPHP) && defined(HWCAP_ASIMDHP)
        caps->hasARMv8Fp16 = (hwcap & (HWCAP_FPHP | HWCAP_ASIMDHP)) != 0;
#            endif
#            ifdef HWCAP_SVE
        caps->hasSVE = (hwcap & HWCAP_SVE) != 0;
#            endif
#            if defined(AT_HWCAP2) && defined(HWCAP2_SVE2)
        caps->hasSVE2 = (getauxval(AT_HWCAP2) & HWCAP2_SVE2) != 0;
#            endif
#        else // 32-bit ARM
#            ifdef HWCAP_NEON
        caps->hasNEON = (hwcap & HWCAP_NEON) != 0;
#            endif
#        endif
#    endif

        print_arm_caps(caps, out);
    }

#endif // ARM

    //
    // PowerPC (AltiVec)
    //
#if defined(MRV_ARCH_PPC)

    void detect_ppc(CpuCaps* caps, std::ostringstream& out)
    {
        log_line(out, "CPU: PowerPC");
#    if defined(__APPLE__)
        int sels[2] = {CTL_HW, HW_VECTORUNIT};
        int has_vu = 0;
        size_t len = sizeof(has_vu);
        if (sysctl(sels, 2, &has_vu, &len, nullptr, 0) == 0 && has_vu != 0)
            caps->hasAltiVec = 1;
#    elif defined(__linux__) && defined(PPC_FEATURE_HAS_ALTIVEC)
        caps->hasAltiVec = (getauxval(AT_HWCAP) & PPC_FEATURE_HAS_ALTIVEC) != 0;
#    elif defined(__ALTIVEC__)
        caps->hasAltiVec = 1; // enabled at compile time
#    endif
        log_line(out, std::string("AltiVec ") + (caps->hasAltiVec ? "" : "not ") + "found");
        if (caps->hasAltiVec)
            out << "AltiVec ";
        out << '\n';
    }

#endif // PPC

} // namespace

std::string GetCpuCaps(CpuCaps* caps)
{
    std::ostringstream out;
    reset_caps(caps);

#if defined(MRV_ARCH_X86)
    detect_x86(caps, out);
#elif defined(MRV_ARCH_ARM64) || defined(MRV_ARCH_ARM32)
    detect_arm(caps, out);
#elif defined(MRV_ARCH_PPC)
    detect_ppc(caps, out);
#elif defined(__EMSCRIPTEN__)
    log_line(out, "CPU: WebAssembly");
#else
    log_line(out, "CPU: unknown architecture, no SIMD features detected");
#endif

    return out.str();
}

unsigned int cpu_count()
{
#if defined(__EMSCRIPTEN__)
    return 1;
#else
    const unsigned int n = std::thread::hardware_concurrency();
    if (n > 0)
        return n;
#    if defined(_WIN32)
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwNumberOfProcessors ? info.dwNumberOfProcessors : 1;
#    elif defined(_SC_NPROCESSORS_ONLN)
    const long c = sysconf(_SC_NPROCESSORS_ONLN);
    return c > 0 ? static_cast<unsigned int>(c) : 1;
#    else
    return 1;
#    endif
#endif
}
