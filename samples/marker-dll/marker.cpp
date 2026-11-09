#include <atomic>

namespace {
std::atomic_uint counter{0};
}

extern "C" __declspec(dllexport) const char* __cdecl LabMarker() {
    return "WININJ_POC_MARKER_001";
}

extern "C" __declspec(dllexport) unsigned int __cdecl LabIncrement() {
    return ++counter;
}
