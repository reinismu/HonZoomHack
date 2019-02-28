#pragma once

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>

#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct MemoryMapEntity {
    // Memory
    unsigned long start;
    unsigned long end;

    // Permissions
    bool readable;
    bool writable;
    bool executable;
    bool shared;

    // File data
    unsigned long offset;
    unsigned char deviceMajor;
    unsigned char deviceMinor;
    unsigned long inodeFileNumber;
    fs::path path;
};

class Process {
public:
    Process(pid_t pId);
    Process(std::string pIdStr);

    void refreshMemoryMap();
    std::optional<MemoryMapEntity> findFirstMemoryEntity(
        std::function<bool(MemoryMapEntity)> predicate);

    bool readProcessMemory(void* address, void* buffer, size_t size);
    bool writeProcessMemory(void* address, void* buffer, size_t size);

    bool ptraceWriteProcessMemory(size_t address, void* buffer, size_t size);

    std::vector<MemoryMapEntity> memoryMap;

private:
    pid_t currentProcessId;
    std::string currentProcessIdStr;
};

std::optional<Process> getProcessByName(std::string processName);