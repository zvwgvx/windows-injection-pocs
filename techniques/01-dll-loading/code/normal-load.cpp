#include <windows.h>
#include <cstdlib>
#include <iostream>
#include <string>

using MarkerFunction = const char* (__cdecl*)();
using IncrementFunction = unsigned int (__cdecl*)();

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: dll-load-demo <absolute-path-to-marker-dll>\n";
        return 1;
    }

    const DWORD pid = GetCurrentProcessId();
    std::cout << "experiment=dll-loading pid=" << pid << std::endl;

    HMODULE module = LoadLibraryA(argv[1]);
    if (module == nullptr) {
        std::cerr << "LoadLibrary error=" << GetLastError() << "\n";
        return 2;
    }

    char path[MAX_PATH]{};
    if (GetModuleFileNameA(module, path, MAX_PATH) == 0) {
        std::cerr << "GetModuleFileName error=" << GetLastError() << "\n";
        FreeLibrary(module);
        return 3;
    }

    MEMORY_BASIC_INFORMATION info{};
    if (!VirtualQuery(module, &info, sizeof(info))) {
        std::cerr << "VirtualQuery error=" << GetLastError() << "\n";
        FreeLibrary(module);
        return 4;
    }

    const auto marker = reinterpret_cast<MarkerFunction>(GetProcAddress(module, "LabMarker"));
    const auto increment = reinterpret_cast<IncrementFunction>(GetProcAddress(module, "LabIncrement"));
    if (marker == nullptr || increment == nullptr) {
        std::cerr << "Required exports not found\n";
        FreeLibrary(module);
        return 5;
    }

    std::cout << "module_path=" << path
              << " base=" << static_cast<void*>(module)
              << " type=0x" << std::hex << info.Type
              << " protect=0x" << info.Protect << std::dec << std::endl;

    std::cout << "marker=" << marker()
              << " count1=" << increment()
              << " count2=" << increment() << std::endl;

    FreeLibrary(module);
    std::cout << "module_unloaded=true" << std::endl;
    return 0;
}
