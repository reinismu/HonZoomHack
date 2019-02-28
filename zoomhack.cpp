#include <iostream>

#include "Process.h"

const std::string HON_PROCESS_NAME = "hon-x86_64";
const std::string HON_SHARED_LIB = "libgame_shared-x86_64.so";

const unsigned long ZOOM_OUT_LOC = 0x0C998B0;
const unsigned long PREPARE_CLIENT_STATE_LOC = 0xC97780;
const unsigned long SETUP_CAMERA_LOC = 0xC97100;

bool patch(Process process, size_t address, void* buf, size_t size)
{
    std::cout << "Try to write memory: " << std::hex << address << std::endl;
    if (!process.ptraceWriteProcessMemory(address, buf, size)) {
        std::cout << "Failed writing memory: " << errno << " at " << std::hex << address << std::endl;
        return false;
    }
    return true;
}

void enableZoomhack(Process process)
{
    auto sharedLibMap = process.findFirstMemoryEntity([&](MemoryMapEntity entity) {
        return entity.executable && entity.path.filename() == HON_SHARED_LIB;
    });

    if (!sharedLibMap) {
        std::cout << "Couldn't find " << HON_SHARED_LIB << " memory map!" << std::endl;
        return;
    }
    std::cout << "SharedLibLoc: " << std::hex << sharedLibMap->start << std::endl;

    auto zoomOut = sharedLibMap->start + ZOOM_OUT_LOC;
    // ZoomOut + 0x4f (minss   xmm1, xmm2) -> (movss   xmm1, xmm2)
    char* patch1 = "\xF3\x0F\x10\xCA";
    if (!patch(process, zoomOut + 0x4f, patch1, 4)) {
        return;
    }

    // ZoomOut + 0x5e (minss   xmm1, xmm3) -> (movss   xmm1, xmm3)
    char* patch2 = "\xF3\x0F\x10\xCB";
    if (!patch(process, zoomOut + 0x5e, patch2, 4)) {
        return;
    }

    auto prepareClientState = sharedLibMap->start + PREPARE_CLIENT_STATE_LOC;

    // PrepareClientState + 0x222 (movss   cs:currentCammeraZoom, xmm0) -> (NOP, NOP ..)
    char* patch3 = "\x90\x90\x90\x90\x90\x90\x90\x90";
    if (!patch(process, prepareClientState + 0x222, patch3, 8)) {
        return;
    }

    //PrepareClientState + 0xc78 (minss   xmm2, xmm3 ) -> (movss   xmm2, xmm3)
    char* patch4 = "\xF3\x0F\x10\xD3";
    if (!patch(process, prepareClientState + 0xc78, patch4, 4)) {
        return;
    }

    //PrepareClientState + 0xc91 (ja      short loc_C9847C ) -> (jmp      short loc_C9847C)
    char* patch5 = "\x90\x90";
    if (!patch(process, prepareClientState + 0xc91, patch5, 2)) {
        return;
    }

    //PrepareClientState + 0xc93 (minss   xmm1, xmm2) -> movss   xmm1, xmm2)
    char* patch6 = "\xF3\x0F\x10\xCA";
    if (!patch(process, prepareClientState + 0xc93, patch6, 4)) {
        return;
    }

    auto setupCamera = sharedLibMap->start + SETUP_CAMERA_LOC;

    //SetupCamera + 0x5df (jbe     short loc_C97702 ) -> (jmp     short loc_C97702)
    char* patch7 = "\xEB";
    if (!patch(process, setupCamera + 0x5df, patch7, 1)) {
        return;
    }
}

int main()
{
    auto process = getProcessByName(HON_PROCESS_NAME);

    if (!process) {
        std::cout << "Process " << HON_PROCESS_NAME << " not found!" << std::endl;
        return 1;
    }

    enableZoomhack(process.value());

    return 0;
}