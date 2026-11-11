// Technique 01 payload variant: an ordinary DLL that appends evidence on load
// and then keeps appending a heartbeat line while it stays loaded in the host.
// This provides live, time-stamped proof of execution inside the target rather
// than a single attach record.
//
// Log contract (verified by poc01-injector):
//   every line contains "pid=<pid> " and "base=0x<16 hex digits>".
#include <windows.h>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cwchar>

namespace {
constexpr wchar_t kEvidenceDirectory[] = L"wininj-poc01";
constexpr unsigned long kHeartbeatIntervalMs = 500;

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

bool evidence_log_path(wchar_t* log_path, std::size_t capacity) {
    wchar_t temp_path[32768]{};
    const DWORD temp_length = GetTempPathW(32768, temp_path);
    if (temp_length == 0 || temp_length >= 32768) {
        return false;
    }

    wchar_t directory[32768]{};
    std::size_t directory_length = 0;
    if (!append_text(directory, 32768, directory_length, temp_path)) {
        return false;
    }
    if (directory_length != 0 && directory[directory_length - 1] != L'\\') {
        if (!append_text(directory, 32768, directory_length, L"\\")) {
            return false;
        }
    }
    if (!append_text(directory, 32768, directory_length, kEvidenceDirectory)) {
        return false;
    }
    if (!CreateDirectoryW(directory, nullptr) && GetLastError() != ERROR_ALREADY_EXISTS) {
        return false;
    }

    std::size_t log_length = 0;
    return append_text(log_path, capacity, log_length, directory) &&
           append_text(log_path, capacity, log_length, L"\\poc01-payload-") &&
           append_unsigned(log_path, capacity, log_length, GetCurrentProcessId()) &&
           append_text(log_path, capacity, log_length, L".log");
}

bool append_line(const wchar_t* log_path, const char* line, std::size_t line_length) {
    HANDLE file = CreateFileW(log_path, FILE_APPEND_DATA,
                              FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                              nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        return false;
    }
    DWORD written = 0;
    const bool ok = WriteFile(file, line, static_cast<DWORD>(line_length), &written,
                              nullptr) &&
                    written == static_cast<DWORD>(line_length);
    CloseHandle(file);
    return ok;
}

// The heartbeat thread stops as soon as its own module is no longer mapped,
// so unloading the DLL can never leave a thread running on freed code.
DWORD WINAPI heartbeat_thread(LPVOID parameter) {
    const auto instance = static_cast<HINSTANCE>(parameter);
    const auto base = reinterpret_cast<unsigned long long>(
        reinterpret_cast<std::uintptr_t>(instance));
    const ULONGLONG started = GetTickCount64();

    wchar_t log_path[32768]{};
    if (!evidence_log_path(log_path, 32768)) {
        return 0;
    }

    for (;;) {
        Sleep(kHeartbeatIntervalMs);

        HMODULE still_loaded = nullptr;
        if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                    GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                reinterpret_cast<LPCWSTR>(
                                    reinterpret_cast<std::uintptr_t>(instance)),
                                &still_loaded)) {
            return 0;
        }

        SYSTEMTIME now{};
        GetSystemTime(&now);

        char line[512]{};
        const int line_length = std::snprintf(
            line, sizeof(line),
            "event=heartbeat pid=%lu tid=%lu payload_id=poc01-heartbeat base=0x%016llx "
            "uptime_ms=%llu timestamp=%04u-%02u-%02uT%02u:%02u:%02u.%03uZ\r\n",
            GetCurrentProcessId(), GetCurrentThreadId(), base,
            static_cast<unsigned long long>(GetTickCount64() - started),
            static_cast<unsigned int>(now.wYear), static_cast<unsigned int>(now.wMonth),
            static_cast<unsigned int>(now.wDay), static_cast<unsigned int>(now.wHour),
            static_cast<unsigned int>(now.wMinute), static_cast<unsigned int>(now.wSecond),
            static_cast<unsigned int>(now.wMilliseconds));
        if (line_length <= 0 || static_cast<std::size_t>(line_length) >= sizeof(line)) {
            return 0;
        }
        append_line(log_path, line, static_cast<std::size_t>(line_length));
    }
}

void record_attach(HINSTANCE instance) {
    wchar_t log_path[32768]{};
    if (!evidence_log_path(log_path, 32768)) {
        return;
    }

    wchar_t module_path[32768]{};
    if (GetModuleFileNameW(instance, module_path, 32768) == 0) {
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
        "event=dll_attach pid=%lu tid=%lu payload_id=poc01-heartbeat base=0x%016llx "
        "module=%s timestamp=%04u-%02u-%02uT%02u:%02u:%02u.%03uZ\r\n",
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
    append_line(log_path, line, static_cast<std::size_t>(line_length));
}
}

extern "C" __declspec(dllexport) unsigned long __cdecl Poc01HeartbeatTicks() {
    return kHeartbeatIntervalMs;
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(instance);
        record_attach(instance);
        // Controlled PoC only: a short-lived thread is started here so the
        // payload can keep proving it is alive inside the host process.
        HANDLE worker = CreateThread(nullptr, 0, heartbeat_thread, instance, 0, nullptr);
        if (worker != nullptr) {
            CloseHandle(worker);
        }
    }
    return TRUE;
}
