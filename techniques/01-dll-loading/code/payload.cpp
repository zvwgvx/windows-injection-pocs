#include <atomic>

namespace {
std::atomic_uint count{0};
}

extern "C" __declspec(dllexport) const char* __cdecl LabMarker() {
    return "POC01_PAYLOAD_INSIDE_TARGET";
}

extern "C" __declspec(dllexport) unsigned int __cdecl LabIncrement() {
    return ++count;
}
