#pragma once

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <unistd.h>
#include <sys/resource.h>
#endif

/**
 * @brief Get the current memory usage of the process.
 * 
 * This function retrieves the current memory consumption of the running process.
 * On Windows, it utilizes the GetProcessMemoryInfo function.
 * On Linux, it reads from /proc/self/statm.
 * 
 * @return size_t Returns the memory used by the process in bytes. Returns 0 if it fails to get the information.
 */
size_t getCurrentMemoryDraw() 
{
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
    {
        return pmc.WorkingSetSize;
    }
    else
    {
        std::cerr << "Failed to get memory info on Windows." << std::endl;
        return 0;
    }
#else
    long rss = 0L;
    FILE *fp = NULL;
    if ((fp = fopen("/proc/self/statm", "r")) == NULL)
    {
        std::cerr << "Failed to get memory info on Linux." << std::endl;
        return 0;
    }
    if (fscanf(fp, "%*s%ld", &rss) != 1)
    {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return (size_t)rss * (size_t)sysconf(_SC_PAGESIZE);
#endif
}