// Technique 01 payload: an ordinary DLL whose DllMain records that it was
// loaded inside the target process. The target has no knowledge of this DLL.
#include <windows.h>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cwchar>

namespace {
constexpr wchar_t kEvidenceDirectory[] = L"wininj-poc01";

bool append_text(wchar_t* buffer, std::size_t capacity, std::size_t& length,
                 const wchar_t* text) {
    const std::size_t addition = wcslen(text);
    if (length + addition + 1 > capacity) {
        return false;
    }
    memcpy(buffer + length, text, (addition + 1) * sizeof(wchar_t));
    length += addition;
    return true;
}

bool append_unsigned(wchar_t* buffer, std::size_t capacity, std::size_t& length,
                     unsigned long value) {
    wchar_t digits[16]{};
    int count = 0;
    if (value == 0) {
        digits[count++] = L'0';
    }
    while (value != 0) {
        digits[count++] = static_cast<wchar_t>(L'0' + (value % 10));
        value /= 10;
    }
    if (length + static_cast<std::size_t>(count) + 1 > capacity) {
        return false;
    }
    for (int i = count - 1; i >= 0; --i) {
        buffer[length++] = digits[i];
    }
    buffer[length] = 0;
    return true;
}

bool to_utf8(const wchar_t* text, char* output, std::size_t output_size) {
    if (text == nullptr || output == nullptr || output_size == 0 ||
        output_size > static_cast<std::size_t>(INT_MAX)) {
        return false;
    }
    const int length = WideCharToMultiByte(CP_UTF8, 0, text, -1, output,
                                           static_cast<int>(output_size), nullptr, nullptr);
    return length > 0;
}

void write_evidence(HINSTANCE instance) {
    wchar_t temp_path[32768]{};
    const DWORD temp_length = GetTempPathW(32768, temp_path);
    if (temp_length == 0 || temp_length >= 32768) {
        return;
    }

    wchar_t directory[32768]{};
    std::size_t directory_length = 0;
    if (!append_text(directory, 32768, directory_length, temp_path)) {
        return;
    }
    if (directory_length != 0 && directory[directory_length - 1] != L'\\') {
        if (!append_text(directory, 32768, directory_length, L"\\")) {
            return;
        }
    }
    if (!append_text(directory, 32768, directory_length, kEvidenceDirectory)) {
        return;
    }
    if (!CreateDirectoryW(directory, nullptr) && GetLastError() != ERROR_ALREADY_EXISTS) {
        return;
    }

    wchar_t module_path[32768]{};
    if (GetModuleFileNameW(instance, module_path, 32768) == 0) {
        return;
    }

    wchar_t log_path[32768]{};
    std::size_t log_length = 0;
    if (!append_text(log_path, 32768, log_length, directory) ||
        !append_text(log_path, 32768, log_length, L"\\poc01-payload-") ||
        !append_unsigned(log_path, 32768, log_length, GetCurrentProcessId()) ||
        !append_text(log_path, 32768, log_length, L".log")) {
        return;
    }

    char module_utf8[32768]{};
    if (!to_utf8(module_path, module_utf8, sizeof(module_utf8))) {
        return;
    }

    SYSTEMTIME now{};
    GetSystemTime(&now);

    char line[33792]{};
    const int line_length = std::snprintf(
        line, sizeof(line),
        "event=dll_attach pid=%lu tid=%lu base=0x%016llx module=%s "
        "timestamp=%04u-%02u-%02uT%02u:%02u:%02u.%03uZ\r\n",
        GetCurrentProcessId(), GetCurrentThreadId(),
        static_cast<unsigned long long>(reinterpret_cast<std::uintptr_t>(instance)),
        module_utf8, static_cast<unsigned int>(now.wYear),
        static_cast<unsigned int>(now.wMonth), static_cast<unsigned int>(now.wDay),
        static_cast<unsigned int>(now.wHour), static_cast<unsigned int>(now.wMinute),
        static_cast<unsigned int>(now.wSecond),
        static_cast<unsigned int>(now.wMilliseconds));
    if (line_length <= 0 || static_cast<std::size_t>(line_length) >= sizeof(line)) {
        return;
    }

    HANDLE file = CreateFileW(log_path, FILE_APPEND_DATA,
                              FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                              nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        return;
    }

    DWORD written = 0;
    WriteFile(file, line, static_cast<DWORD>(line_length), &written, nullptr);
    CloseHandle(file);
}
}

extern "C" __declspec(dllexport) const char* __cdecl Poc01Marker() {
    return "WININJ_POC01_PAYLOAD_V1";
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(instance);
        // Controlled PoC only: keep this file write small and allocation-free.
        // Production DLLs should avoid file I/O while the loader lock is held.
        write_evidence(instance);
    }
    return TRUE;
}
