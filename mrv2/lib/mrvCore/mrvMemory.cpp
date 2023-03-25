

#ifdef __linux__
#    include <sys/types.h>
#    include <sys/sysinfo.h>
#    include <stdlib.h>
#    include <stdio.h>
#    include <string.h>
#endif

#ifdef __APPLE__
#    include <sys/sysctl.h>
#    include <sys/param.h>
#    include <sys/mount.h>
#    include <mach/mach.h>
#endif

#ifdef _WIN32
#    include <windows.h>
#    include <psapi.h>
#endif

#include "mrvCore/mrvMemory.h"

#include "mrvFl/mrvIO.h"

namespace mrv
{

#ifdef _WIN32
    void memory_information(
        uint64_t& totalVirtualMem, uint64_t& virtualMemUsed,
        uint64_t& virtualMemUsedByMe, uint64_t& totalPhysMem,
        uint64_t& physMemUsed, uint64_t& physMemUsedByMe)
    {
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        GlobalMemoryStatusEx(&memInfo);
        totalVirtualMem = memInfo.ullTotalPageFile;
        virtualMemUsed = totalVirtualMem - memInfo.ullAvailPageFile;
        totalVirtualMem /= (1024 * 1024);
        virtualMemUsed /= (1024 * 1024);
        totalPhysMem = memInfo.ullTotalPhys;
        physMemUsed = totalPhysMem - memInfo.ullAvailPhys;
        totalPhysMem /= (1024 * 1024);
        physMemUsed /= (1024 * 1024);

        PROCESS_MEMORY_COUNTERS_EX pmc;
        GetProcessMemoryInfo(
            GetCurrentProcess(), (PPROCESS_MEMORY_COUNTERS)&pmc, sizeof(pmc));
        virtualMemUsedByMe = pmc.PrivateUsage;
        virtualMemUsedByMe /= (1024 * 1024);
        physMemUsedByMe = pmc.WorkingSetSize;
        physMemUsedByMe /= (1024 * 1024);
    }
#endif

#ifdef __linux__

    static int parseLine(char* line)
    {
        int i = strlen(line);
        while (*line < '0' || *line > '9')
            line++;
        line[i - 3] = '\0';
        i = atoi(line);
        return i;
    }

    static int getValue()
    { // Note: this value is in KB!
        FILE* file = fopen("/proc/self/status", "r");
        int result = -1;
        char line[128];

        while (fgets(line, 128, file) != NULL)
        {
            if (strncmp(line, "VmSize:", 7) == 0)
            {
                result = parseLine(line);
                break;
            }
        }
        fclose(file);
        return result;
    }

    static int getValue2()
    { // Note: this value is in KB!
        FILE* file = fopen("/proc/self/status", "r");
        int result = -1;
        char line[128];

        while (fgets(line, 128, file) != NULL)
        {
            if (strncmp(line, "VmRSS:", 6) == 0)
            {
                result = parseLine(line);
                break;
            }
        }
        fclose(file);
        return result;
    }

    void memory_information(
        uint64_t& totalVirtualMem, uint64_t& virtualMemUsed,
        uint64_t& virtualMemUsedByMe, uint64_t& totalPhysMem,
        uint64_t& physMemUsed, uint64_t& physMemUsedByMe)
    {

        struct sysinfo memInfo;
        sysinfo(&memInfo);

        totalVirtualMem = memInfo.totalram;
        // Add other values in next statement to avoid int overflow
        // on right hand side...
        totalVirtualMem += memInfo.totalswap;
        totalVirtualMem *= memInfo.mem_unit;
        totalVirtualMem /= (1024 * 1024);

        virtualMemUsed = memInfo.totalram - memInfo.freeram;
        // Add other values in next statement to avoid int overflow on
        // right hand side...
        virtualMemUsed += memInfo.totalswap - memInfo.freeswap;
        virtualMemUsed *= memInfo.mem_unit;
        virtualMemUsed /= (1024 * 1024);

        totalPhysMem = memInfo.totalram;
        // Multiply in next statement to avoid int overflow on right hand
        // side...
        totalPhysMem *= memInfo.mem_unit;
        totalPhysMem /= (1024 * 1024);

        physMemUsed = memInfo.totalram - memInfo.freeram;
        // Multiply in next statement to avoid int overflow on right hand
        // side...
        physMemUsed *= memInfo.mem_unit;
        physMemUsed /= (1024 * 1024);

        virtualMemUsedByMe = getValue();
        virtualMemUsedByMe /= 1024;

        physMemUsedByMe = getValue2();
        physMemUsedByMe /= 1024;

    } // memory_information

#endif

#ifdef __APPLE__

    void memory_information(
        uint64_t& totalVirtualMem, uint64_t& virtualMemUsed,
        uint64_t& virtualMemUsedByMe, uint64_t& totalPhysMem,
        uint64_t& physMemUsed, uint64_t& physMemUsedByMe)
    {
        static const char* kModule = "mem";

        //
        //  Total Virtual Memory
        //
        struct statfs stats;
        totalVirtualMem = 0;
        if (0 == statfs("/", &stats))
        {
            totalVirtualMem = (uint64_t)stats.f_bsize * stats.f_bfree;
            totalVirtualMem /= (1024 * 1024);
        }

        //
        //  Total Physical Memory
        //
        int mib[2] = {CTL_HW, HW_MEMSIZE};
        u_int namelen = sizeof(mib) / sizeof(mib[0]);
        size_t len = sizeof(totalPhysMem);

        if (sysctl(mib, namelen, &totalPhysMem, &len, NULL, 0) < 0)
        {
            LOG_ERROR(_("sysctl failed!"));
        }

        totalPhysMem /= (1024 * 1024);

        //
        // Physical Memory Used
        //
        vm_size_t page_size;
        mach_port_t mach_port;
        mach_msg_type_number_t count;
        vm_statistics64_data_t vm_stats;

        mach_port = mach_host_self();
        count = sizeof(vm_stats) / sizeof(natural_t);
        if (KERN_SUCCESS != host_page_size(mach_port, &page_size) ||
            KERN_SUCCESS !=
                host_statistics64(
                    mach_port, HOST_VM_INFO, (host_info64_t)&vm_stats, &count))
        {
            LOG_ERROR(_("host_statistics64 failed"));
        }

        int64_t active = (int64_t)vm_stats.active_count;
        int64_t inactive = (int64_t)vm_stats.inactive_count;
        int64_t wired = (int64_t)vm_stats.wire_count;
        physMemUsed = (active + inactive + wired) * (int64_t)page_size;
        physMemUsed /= 1024 * 1024;

        struct task_basic_info t_info;
        mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

        if (KERN_SUCCESS != task_info(
                                mach_task_self(), TASK_BASIC_INFO,
                                (task_info_t)&t_info, &t_info_count))
        {
            LOG_ERROR(_("task info failed"));
        }

        physMemUsedByMe = t_info.resident_size;
        physMemUsedByMe /= 1024 * 1024;

        virtualMemUsedByMe = t_info.virtual_size;
        virtualMemUsedByMe /= 1024 * 1024;

    } // memory_information

#endif // __APPLE__

} // namespace mrv
