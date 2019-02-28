#include "Process.h"

std::string readFirstLine(std::string filePath)
{
    std::ifstream mapsPathStream(filePath.c_str());
    std::string name;

    std::getline(mapsPathStream, name);
    return name;
}

std::optional<MemoryMapEntity> parseMemoryMapEntry(std::string line)
{
    MemoryMapEntity entity = {};

    std::istringstream inStream(line);

    std::string memorySpace, permissions, offset, device, inode, pathname;
    if (!(inStream >> memorySpace >> permissions >> offset >> device >> inode >> pathname)) {
        return std::nullopt;
    }

    std::stringstream ss;

    size_t memorySplit = memorySpace.find_first_of('-');
    if (memorySplit != -1) {
        ss << std::hex << memorySpace.substr(0, memorySplit);
        ss >> entity.start;
        ss.str("");
        ss.clear();

        ss << std::hex << memorySpace.substr(memorySplit + 1, memorySpace.size());
        ss >> entity.end;
        ss.str("");
        ss.clear();
    }

    size_t deviceSplit = device.find_first_of(':');
    if (deviceSplit != -1) {
        ss << std::hex << device.substr(0, deviceSplit);
        ss >> entity.deviceMajor;
        ss.str("");
        ss.clear();

        ss << std::hex << device.substr(deviceSplit + 1, device.size());
        ss >> entity.deviceMinor;
        ss.str("");
        ss.clear();
    }

    ss << std::hex << offset;
    ss >> entity.offset;
    ss.str("");
    ss.clear();

    ss << inode;
    ss >> entity.inodeFileNumber;

    entity.readable = (permissions[0] == 'r');
    entity.writable = (permissions[1] == 'w');
    entity.executable = (permissions[2] == 'x');
    entity.shared = (permissions[3] != '-');

    entity.path = fs::path(pathname);

    return std::optional<MemoryMapEntity>(entity);
}

Process::Process(pid_t pId)
{
    currentProcessId = pId;
    currentProcessIdStr = std::to_string(pId);
    refreshMemoryMap();
}

Process::Process(std::string pIdStr)
{
    currentProcessId = std::stoi(pIdStr);
    currentProcessIdStr = pIdStr;
    refreshMemoryMap();
}

void Process::refreshMemoryMap()
{
    std::ifstream maps("/proc/" + currentProcessIdStr + "/maps");

    std::vector<MemoryMapEntity> entities;

    std::string line;
    while (std::getline(maps, line)) {
        if (auto entity = parseMemoryMapEntry(line)) {
            entities.push_back(*entity);
        }
    }
    memoryMap = entities;
}

std::optional<MemoryMapEntity> Process::findFirstMemoryEntity(
    std::function<bool(MemoryMapEntity)> predicate)
{
    for (auto entity : memoryMap) {
        if (predicate(entity)) {
            return entity;
        }
    }
    return std::nullopt;
}

bool Process::readProcessMemory(void* address, void* buffer, size_t size)
{
    struct iovec local[1];
    struct iovec remote[1];

    local[0].iov_base = buffer;
    local[0].iov_len = size;
    remote[0].iov_base = address;
    remote[0].iov_len = size;

    return (process_vm_readv(currentProcessId, local, 1, remote, 1, 0) == size);
}

bool Process::writeProcessMemory(void* address, void* buffer, size_t size)
{
    struct iovec local[1];
    struct iovec remote[1];

    local[0].iov_base = buffer;
    local[0].iov_len = size;
    remote[0].iov_base = address;
    remote[0].iov_len = size;

    return (process_vm_writev(currentProcessId, local, 1, remote, 1, 0) == size);
}

bool Process::ptraceWriteProcessMemory(size_t address, void* buffer, size_t size)
{
    // Tracing the app. This fails:
    if (ptrace(PTRACE_ATTACH, currentProcessId, NULL, NULL) == -1)
        return false;
    // Force it to wait
    if (waitpid(currentProcessId, NULL, 0) == -1) {
        ptrace(PTRACE_DETACH, currentProcessId, NULL, NULL);
        return false;
    }
    pid_t pid = currentProcessId;
    const char* src = (const char*)buffer;

    while (size >= sizeof(long)) {
        ptrace(PTRACE_POKETEXT, pid, address, *(long*)src);
        address += sizeof(long);
        src += sizeof(long);
        size -= sizeof(long);
    }
    if (size != 0) {
        long word = ptrace(PTRACE_PEEKTEXT, pid, address, 0);
        char* dest = (char*)&word;
        if (word == -1 && errno != 0) {
            return false;
        }
        while (size--) {
            *(dest++) = *(src++);
        }
        ptrace(PTRACE_POKETEXT, pid, address, word);
    }
    // End petrace.
    ptrace(PTRACE_DETACH, currentProcessId, NULL, NULL);
    return true;
}

std::optional<Process> getProcessByName(std::string processName)
{
    for (auto& entry : fs::directory_iterator("/proc/")) {
        auto path = entry.path();
        std::string mapsPath = std::string(path) + "/comm";

        if (!fs::exists(mapsPath)) {
            continue;
        }

        if (processName == readFirstLine(mapsPath)) {
            std::cout << "Found process: " << processName << std::endl;
            Process process(std::string(path.filename()));
            return std::optional<Process>(process);
        }
    }

    return std::nullopt;
}
