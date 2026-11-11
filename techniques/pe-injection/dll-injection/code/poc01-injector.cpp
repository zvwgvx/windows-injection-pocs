// Technique 01: classic remote DLL injection.
//
// The injector uses only the target PID and a DLL path. It does not rely on any
// named event, pipe, RPC endpoint, exported target function, hook, or other
// cooperation from the target process.
#include <windows.h>
#include <tlhelp32.h>
#include <shellapi.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cwchar>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace {
constexpr DWORD kDefaultTimeoutMs = 15000;

enum ExitCode {
    kExitOk = 0,
    kExitUsage = 1,
    kExitSystemError = 2,
    kExitPayloadAbsent = 3,
    kExitVerificationFailed = 4,
    kExitRemoteLoadFailed = 5,
    kExitAlreadyLoaded = 6,
};

class Handle {
public:
    explicit Handle(HANDLE handle = nullptr) : value_(handle) {}
    ~Handle() {
        if (value_ != nullptr && value_ != INVALID_HANDLE_VALUE) {
            CloseHandle(value_);
        }
    }
    Handle(const Handle&) = delete;
    Handle& operator=(const Handle&) = delete;

    HANDLE get() const {
        return value_;
    }
    explicit operator bool() const {
        return value_ != nullptr && value_ != INVALID_HANDLE_VALUE;
    }

private:
    HANDLE value_;
};

struct ModuleInfo {
    std::wstring name;
    std::wstring path;
    std::uintptr_t base = 0;
    std::size_t size = 0;
};

struct RemoteProcedure {
    std::uintptr_t local_address = 0;
    std::uintptr_t remote_address = 0;
    std::uintptr_t provider_local_base = 0;
    std::uintptr_t provider_remote_base = 0;
    std::uintptr_t offset = 0;
    std::wstring provider_name;
};

struct Options {
    DWORD pid = 0;
    std::wstring dll;
    DWORD timeout_ms = kDefaultTimeoutMs;
    bool check_only = false;
    bool help = false;
};

struct LogCheck {
    std::wstring path;
    bool found = false;
    bool fresh = false;
    bool pid_match = false;
    bool base_match = false;
    DWORD error = 0;
};

std::string utf8(const std::wstring& text) {
    if (text.empty()) {
        return {};
    }
    const int length = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, nullptr, 0,
                                           nullptr, nullptr);
    if (length <= 1) {
        return {};
    }
    std::string result(static_cast<std::size_t>(length), '\0');
    if (WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, result.data(), length,
                            nullptr, nullptr) == 0) {
        return {};
    }
    result.pop_back();
    return result;
}

std::string win32_message(DWORD code) {
    wchar_t* buffer = nullptr;
    const DWORD length = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPWSTR>(&buffer), 0, nullptr);

    std::string result;
    if (length != 0 && buffer != nullptr) {
        std::wstring text(buffer, length);
        while (!text.empty() &&
               (text.back() == L'\r' || text.back() == L'\n' || text.back() == L' ')) {
            text.pop_back();
        }
        result = utf8(text);
    }
    if (buffer != nullptr) {
        LocalFree(buffer);
    }
    if (result.empty()) {
        result = "Win32 error " + std::to_string(code);
    }
    return result;
}

std::wstring file_name(const std::wstring& path) {
    const std::size_t position = path.find_last_of(L"\\/");
    return position == std::wstring::npos ? path : path.substr(position + 1);
}

bool same_text(const std::wstring& left, const std::wstring& right) {
    return _wcsicmp(left.c_str(), right.c_str()) == 0;
}

bool parse_dword(const wchar_t* text, DWORD& result) {
    if (text == nullptr || *text == 0 || *text == L'-') {
        return false;
    }
    wchar_t* end = nullptr;
    const unsigned long long value = std::wcstoull(text, &end, 10);
    if (end == text || *end != 0 || value == 0 || value > 0xFFFFFFFFULL) {
        return false;
    }
    result = static_cast<DWORD>(value);
    return true;
}

void print_usage() {
    std::cout
        << "Usage: poc01-injector --pid <pid> --dll <absolute-path> [--timeout-ms 15000]\n"
        << "       poc01-injector --pid <pid> --dll <absolute-path> --check-only\n"
        << "\n"
        << "  --check-only   Do not modify the target. Exit 0 if the exact DLL path is\n"
        << "                 already loaded, exit 3 if it is absent.\n";
}

bool parse_options(int argc, wchar_t** argv, Options& options) {
    for (int i = 1; i < argc; ++i) {
        const std::wstring argument = argv[i];
        if (argument == L"--help" || argument == L"-h") {
            options.help = true;
            continue;
        }
        if (argument == L"--check-only") {
            options.check_only = true;
            continue;
        }
        if (argument == L"--pid" || argument == L"--dll" || argument == L"--timeout-ms") {
            if (i + 1 >= argc) {
                std::cerr << "Missing value for " << utf8(argument) << "\n";
                return false;
            }
            const wchar_t* value = argv[++i];
            if (argument == L"--pid") {
                if (!parse_dword(value, options.pid)) {
                    std::cerr << "Invalid PID\n";
                    return false;
                }
            } else if (argument == L"--dll") {
                options.dll = value;
            } else {
                DWORD timeout = 0;
                if (!parse_dword(value, timeout) || timeout < 100 || timeout > 300000) {
                    std::cerr << "Invalid timeout (100..300000 ms)\n";
                    return false;
                }
                options.timeout_ms = timeout;
            }
            continue;
        }
        std::cerr << "Unknown option: " << utf8(argument) << "\n";
        return false;
    }

    if (options.help) {
        return true;
    }
    if (options.pid == 0 || options.dll.empty()) {
        std::cerr << "--pid and --dll are required\n";
        return false;
    }
    return true;
}

std::optional<std::vector<ModuleInfo>> snapshot_modules(DWORD pid, std::string& error) {
    for (int attempt = 0; attempt < 5; ++attempt) {
        Handle snapshot(CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid));
        if (!snapshot) {
            const DWORD code = GetLastError();
            if (code == ERROR_BAD_LENGTH) {
                Sleep(20);
                continue;
            }
            error = "CreateToolhelp32Snapshot failed: " + win32_message(code);
            return std::nullopt;
        }

        MODULEENTRY32W entry{};
        entry.dwSize = sizeof(entry);
        if (!Module32FirstW(snapshot.get(), &entry)) {
            const DWORD code = GetLastError();
            if (code == ERROR_BAD_LENGTH) {
                Sleep(20);
                continue;
            }
            if (code == ERROR_NO_MORE_FILES) {
                return std::vector<ModuleInfo>{};
            }
            error = "Module32FirstW failed: " + win32_message(code);
            return std::nullopt;
        }

        std::vector<ModuleInfo> modules;
        do {
            ModuleInfo module;
            module.name = entry.szModule;
            module.path = entry.szExePath;
            module.base = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
            module.size = static_cast<std::size_t>(entry.modBaseSize);
            modules.push_back(std::move(module));
            entry.dwSize = sizeof(entry);
        } while (Module32NextW(snapshot.get(), &entry));

        return modules;
    }

    error = "Module snapshot did not stabilize";
    return std::nullopt;
}

const ModuleInfo* find_module_by_path(const std::vector<ModuleInfo>& modules,
                                      const std::wstring& path) {
    for (const auto& module : modules) {
        if (!module.path.empty() && same_text(module.path, path)) {
            return &module;
        }
    }
    return nullptr;
}

const ModuleInfo* find_module_by_name(const std::vector<ModuleInfo>& modules,
                                      const std::wstring& name) {
    for (const auto& module : modules) {
        if (same_text(module.name, name)) {
            return &module;
        }
    }
    return nullptr;
}

std::wstring process_image_path(HANDLE process) {
    wchar_t path[32768]{};
    DWORD length = static_cast<DWORD>(sizeof(path) / sizeof(path[0]));
    if (!QueryFullProcessImageNameW(process, 0, path, &length)) {
        return {};
    }
    return std::wstring(path, length);
}

bool is_x64_target(HANDLE process, std::string& detail) {
    using IsWow64Process2Fn = BOOL(WINAPI*)(HANDLE, USHORT*, USHORT*);
    const HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
    const auto is_wow64_process2 = kernel32 != nullptr
        ? reinterpret_cast<IsWow64Process2Fn>(GetProcAddress(kernel32, "IsWow64Process2"))
        : nullptr;

    if (is_wow64_process2 != nullptr) {
        USHORT process_machine = IMAGE_FILE_MACHINE_UNKNOWN;
        USHORT native_machine = IMAGE_FILE_MACHINE_UNKNOWN;
        if (!is_wow64_process2(process, &process_machine, &native_machine)) {
            detail = "IsWow64Process2 failed: " + win32_message(GetLastError());
            return false;
        }

        char buffer[128]{};
        std::snprintf(buffer, sizeof(buffer),
                      "process_machine=0x%04x native_machine=0x%04x",
                      static_cast<unsigned int>(process_machine),
                      static_cast<unsigned int>(native_machine));
        detail = buffer;

        if (process_machine == IMAGE_FILE_MACHINE_AMD64) {
            return true;
        }
        return process_machine == IMAGE_FILE_MACHINE_UNKNOWN &&
               native_machine == IMAGE_FILE_MACHINE_AMD64;
    }

    BOOL wow64 = FALSE;
    if (!IsWow64Process(process, &wow64)) {
        detail = "IsWow64Process failed: " + win32_message(GetLastError());
        return false;
    }
    detail = wow64 ? "wow64=1 (32-bit target)" : "wow64=0";
    return wow64 == FALSE;
}

bool resolve_remote_load_library(const std::vector<ModuleInfo>& modules,
                                 RemoteProcedure& result, std::string& error) {
    const HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
    if (kernel32 == nullptr) {
        error = "Local kernel32.dll is not loaded";
        return false;
    }
    const FARPROC local_load_library = GetProcAddress(kernel32, "LoadLibraryW");
    if (local_load_library == nullptr) {
        error = "LoadLibraryW is not exported by kernel32.dll";
        return false;
    }

    const auto local_address = reinterpret_cast<std::uintptr_t>(local_load_library);
    HMODULE provider = nullptr;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            reinterpret_cast<LPCWSTR>(local_address), &provider)) {
        error = "GetModuleHandleExW failed: " + win32_message(GetLastError());
        return false;
    }

    wchar_t provider_path[32768]{};
    if (GetModuleFileNameW(provider, provider_path, 32768) == 0) {
        error = "GetModuleFileNameW failed: " + win32_message(GetLastError());
        return false;
    }
    const std::wstring provider_name = file_name(provider_path);
    const ModuleInfo* remote_provider = find_module_by_name(modules, provider_name);
    if (remote_provider == nullptr) {
        error = "Remote target does not have provider module " + utf8(provider_name);
        return false;
    }
    if (!remote_provider->path.empty() &&
        !same_text(remote_provider->path, provider_path)) {
        error = "Remote provider module path differs from the local provider: " +
                utf8(remote_provider->path);
        return false;
    }

    const auto provider_local_base = reinterpret_cast<std::uintptr_t>(provider);
    if (local_address < provider_local_base) {
        error = "Local LoadLibraryW address precedes its provider base";
        return false;
    }
    const std::uintptr_t offset = local_address - provider_local_base;
    if (remote_provider->size != 0 && offset >= remote_provider->size) {
        error = "LoadLibraryW offset is outside the remote provider module";
        return false;
    }

    result.local_address = local_address;
    result.remote_address = remote_provider->base + offset;
    result.provider_local_base = provider_local_base;
    result.provider_remote_base = remote_provider->base;
    result.offset = offset;
    result.provider_name = provider_name;
    return true;
}

std::wstring payload_log_path(DWORD pid, std::string& error) {
    wchar_t temp_path[32768]{};
    const DWORD length = GetTempPathW(32768, temp_path);
    if (length == 0 || length >= 32768) {
        error = "GetTempPathW failed: " + win32_message(GetLastError());
        return {};
    }

    std::wstring directory(temp_path, length);
    if (directory.empty() || directory.back() != L'\\') {
        directory.push_back(L'\\');
    }
    directory += L"wininj-poc01";
    directory += L"\\poc01-payload-";
    directory += std::to_wstring(pid);
    directory += L".log";
    return directory;
}

std::uint64_t filetime_value(const FILETIME& time) {
    ULARGE_INTEGER value{};
    value.LowPart = time.dwLowDateTime;
    value.HighPart = time.dwHighDateTime;
    return value.QuadPart;
}

LogCheck inspect_payload_log(const std::wstring& path, DWORD pid, std::uintptr_t base,
                             const FILETIME& started) {
    LogCheck check;
    check.path = path;

    Handle file(CreateFileW(path.c_str(), GENERIC_READ,
                            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                            nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
    if (!file) {
        check.error = GetLastError();
        return check;
    }
    check.found = true;

    FILETIME last_write{};
    if (GetFileTime(file.get(), nullptr, nullptr, &last_write)) {
        // Allow a small tolerance for file systems with coarse timestamps.
        constexpr std::uint64_t kTwoSeconds = 20'000'000ULL;
        check.fresh = filetime_value(last_write) + kTwoSeconds >= filetime_value(started);
    }

    char content[8192]{};
    DWORD read = 0;
    if (ReadFile(file.get(), content, sizeof(content) - 1, &read, nullptr)) {
        content[read] = 0;
        const std::string text(content, read);

        const std::string pid_token = "pid=" + std::to_string(pid) + " ";
        check.pid_match = text.find(pid_token) != std::string::npos;

        char base_token[32]{};
        std::snprintf(base_token, sizeof(base_token), "base=0x%016llx",
                      static_cast<unsigned long long>(base));
        check.base_match = text.find(base_token) != std::string::npos;
    }
    return check;
}

int run(int argc, wchar_t** argv) {
    Options options;
    if (!parse_options(argc, argv, options)) {
        print_usage();
        return kExitUsage;
    }
    if (options.help) {
        print_usage();
        return kExitOk;
    }

    wchar_t full_path[32768]{};
    const DWORD full_length =
        GetFullPathNameW(options.dll.c_str(), 32768, full_path, nullptr);
    if (full_length == 0 || full_length >= 32768) {
        std::cerr << "error=unable to resolve DLL path: "
                  << win32_message(GetLastError()) << "\n";
        return kExitUsage;
    }
    options.dll.assign(full_path, full_length);

    const DWORD attributes = GetFileAttributesW(options.dll.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES ||
        (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
        std::cerr << "error=DLL path is not a readable file: " << utf8(options.dll) << "\n";
        return kExitUsage;
    }

    std::cout << "experiment=dll-injection\n"
              << "mode=" << (options.check_only ? "check-only" : "inject") << "\n"
              << "target_pid=" << options.pid << "\n"
              << "payload_path=" << utf8(options.dll) << "\n";

    const DWORD access = options.check_only
        ? PROCESS_QUERY_INFORMATION
        : (PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION |
           PROCESS_VM_OPERATION | PROCESS_VM_WRITE);
    Handle process(OpenProcess(access, FALSE, options.pid));
    if (!process) {
        const DWORD code = GetLastError();
        Handle probe(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, options.pid));
        if (probe) {
            std::cerr << "error=OpenProcess failed: the process exists but the injector "
                         "was denied the required rights (try matching integrity level); "
                      << win32_message(code) << "\n";
        } else {
            std::cerr << "error=OpenProcess failed: the PID may not exist or the process "
                         "is protected; " << win32_message(code) << "\n";
        }
        return kExitSystemError;
    }

    const std::wstring target_path = process_image_path(process.get());
    if (!target_path.empty()) {
        std::cout << "target_path=" << utf8(target_path) << "\n";
    }

    std::string architecture_detail;
    if (!is_x64_target(process.get(), architecture_detail)) {
        std::cerr << "error=target is not a native x64 process ("
                  << architecture_detail << ")\n";
        return kExitSystemError;
    }
    std::cout << "target_architecture=" << architecture_detail << "\n";

    DWORD target_exit = 0;
    if (!GetExitCodeProcess(process.get(), &target_exit)) {
        std::cerr << "error=GetExitCodeProcess failed: "
                  << win32_message(GetLastError()) << "\n";
        return kExitSystemError;
    }
    if (target_exit != STILL_ACTIVE) {
        std::cerr << "error=target has already exited\n";
        return kExitSystemError;
    }

    std::string snapshot_error;
    auto before = snapshot_modules(options.pid, snapshot_error);
    if (!before) {
        std::cerr << "error=before module snapshot failed: " << snapshot_error << "\n";
        return kExitSystemError;
    }

    const std::wstring payload_name = file_name(options.dll);
    const ModuleInfo* before_by_path = find_module_by_path(*before, options.dll);
    const ModuleInfo* before_by_name = find_module_by_name(*before, payload_name);
    std::cout << "module_before=" << ((before_by_path != nullptr) ? "present" : "absent")
              << "\n"
              << "modules_before=" << before->size() << "\n";

    if (before_by_path != nullptr) {
        if (options.check_only) {
            std::cout << "module_path_match=true\n"
                      << "result=present\n";
            return kExitOk;
        }
        std::cerr << "error=payload DLL is already loaded in the target; no injection "
                     "was attempted\n";
        return kExitAlreadyLoaded;
    }
    if (before_by_name != nullptr) {
        std::cerr << "error=a module with the same name is already loaded from a "
                     "different path: "
                  << utf8(before_by_name->path) << "\n";
        return kExitAlreadyLoaded;
    }

    if (options.check_only) {
        std::cout << "result=absent\n";
        return kExitPayloadAbsent;
    }

    std::string log_path_error;
    const std::wstring evidence_path = payload_log_path(options.pid, log_path_error);
    if (evidence_path.empty()) {
        std::cerr << "error=payload log path: " << log_path_error << "\n";
        return kExitSystemError;
    }
    if (DeleteFileW(evidence_path.c_str())) {
        std::cout << "stale_payload_log_removed=true\n";
    } else {
        const DWORD code = GetLastError();
        if (code != ERROR_FILE_NOT_FOUND && code != ERROR_PATH_NOT_FOUND &&
            code != ERROR_DIRECTORY) {
            std::cout << "stale_payload_log_removed=false error="
                      << win32_message(code) << "\n";
        }
    }

    RemoteProcedure remote_load_library;
    std::string resolve_error;
    if (!resolve_remote_load_library(*before, remote_load_library, resolve_error)) {
        std::cerr << "error=" << resolve_error << "\n";
        return kExitSystemError;
    }

    std::cout << "provider_module=" << utf8(remote_load_library.provider_name) << "\n"
              << "provider_local_base=0x" << std::hex << remote_load_library.provider_local_base
              << std::dec << "\n"
              << "provider_remote_base=0x" << std::hex
              << remote_load_library.provider_remote_base << std::dec << "\n"
              << "load_library_offset=0x" << std::hex << remote_load_library.offset
              << std::dec << "\n"
              << "load_library_remote=0x" << std::hex
              << remote_load_library.remote_address << std::dec << "\n";

    const std::size_t path_bytes = (options.dll.size() + 1) * sizeof(wchar_t);
    void* remote_path = VirtualAllocEx(process.get(), nullptr, path_bytes,
                                       MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (remote_path == nullptr) {
        std::cerr << "error=VirtualAllocEx failed: " << win32_message(GetLastError())
                  << "\n";
        return kExitSystemError;
    }
    std::cout << "remote_buffer=0x" << std::hex
              << reinterpret_cast<std::uintptr_t>(remote_path) << std::dec << "\n";

    SIZE_T written = 0;
    if (!WriteProcessMemory(process.get(), remote_path, options.dll.c_str(), path_bytes,
                            &written) ||
        written != path_bytes) {
        const DWORD code = GetLastError();
        std::cerr << "error=WriteProcessMemory failed: " << win32_message(code) << "\n";
        VirtualFreeEx(process.get(), remote_path, 0, MEM_RELEASE);
        return kExitSystemError;
    }

    FILETIME started{};
    GetSystemTimeAsFileTime(&started);

    DWORD remote_thread_id = 0;
    Handle remote_thread(CreateRemoteThread(
        process.get(), nullptr, 0,
        reinterpret_cast<LPTHREAD_START_ROUTINE>(remote_load_library.remote_address),
        remote_path, 0, &remote_thread_id));
    if (!remote_thread) {
        std::cerr << "error=CreateRemoteThread failed: " << win32_message(GetLastError())
                  << "\n";
        VirtualFreeEx(process.get(), remote_path, 0, MEM_RELEASE);
        return kExitSystemError;
    }
    std::cout << "remote_thread_id=" << remote_thread_id << "\n";

    const DWORD wait_result = WaitForSingleObject(remote_thread.get(), options.timeout_ms);
    DWORD thread_exit_code = 0;
    bool thread_signaled = false;
    if (wait_result == WAIT_OBJECT_0) {
        thread_signaled = true;
        if (!GetExitCodeThread(remote_thread.get(), &thread_exit_code)) {
            std::cerr << "error=GetExitCodeThread failed: "
                      << win32_message(GetLastError()) << "\n";
            return kExitSystemError;
        }
        VirtualFreeEx(process.get(), remote_path, 0, MEM_RELEASE);
        std::cout << "thread_wait=signaled\n"
                  << "thread_exit_code=0x" << std::hex << thread_exit_code << std::dec
                  << "\n";
    } else if (wait_result == WAIT_TIMEOUT) {
        std::cout << "thread_wait=timeout\n"
                  << "remote_buffer_released=false\n";
    } else {
        std::cerr << "error=WaitForSingleObject failed: "
                  << win32_message(GetLastError()) << "\n";
        return kExitSystemError;
    }

    std::optional<std::vector<ModuleInfo>> after;
    const ModuleInfo* after_module = nullptr;
    for (int attempt = 0; attempt < 10; ++attempt) {
        std::string after_error;
        after = snapshot_modules(options.pid, after_error);
        if (!after) {
            std::cerr << "error=after module snapshot failed: " << after_error << "\n";
            return kExitSystemError;
        }

        after_module = find_module_by_path(*after, options.dll);
        if (after_module == nullptr) {
            after_module = find_module_by_name(*after, payload_name);
        }
        if (after_module != nullptr) {
            std::cout << "modules_after=" << after->size() << "\n";
            break;
        }
        if (thread_signaled && attempt >= 2) {
            break;
        }
        Sleep(50);
    }

    if (after_module == nullptr) {
        std::cerr << "error=payload module was not found in the target after the remote "
                     "thread completed\n";
        std::cout << "module_after=absent\n"
                  << "result=remote-load-failed\n";
        return kExitRemoteLoadFailed;
    }

    const std::uintptr_t module_base = after_module->base;
    std::cout << "module_after=present\n"
              << "module_name=" << utf8(after_module->name) << "\n"
              << "module_base=0x" << std::hex << module_base << std::dec << "\n"
              << "module_path=" << utf8(after_module->path) << "\n"
              << "module_path_match="
              << (same_text(after_module->path, options.dll) ? "true" : "false")
              << "\n";

    const LogCheck log = inspect_payload_log(evidence_path, options.pid, module_base,
                                             started);
    std::cout << "payload_log=" << utf8(log.path) << "\n"
              << "payload_log_present=" << (log.found ? "true" : "false") << "\n";
    if (log.found) {
        std::cout << "payload_log_fresh=" << (log.fresh ? "true" : "false") << "\n"
                  << "payload_log_pid_match=" << (log.pid_match ? "true" : "false")
                  << "\n"
                  << "payload_log_base_match=" << (log.base_match ? "true" : "false")
                  << "\n";
    } else {
        std::cout << "payload_log_error=" << win32_message(log.error) << "\n";
    }

    const bool verified = same_text(after_module->path, options.dll) && log.found &&
                          log.fresh && log.pid_match && log.base_match;
    if (!verified) {
        std::cerr << "error=injection occurred but the evidence chain is incomplete; "
                     "refusing to report success\n";
        std::cout << "result=verification-failed\n";
        return kExitVerificationFailed;
    }

    std::cout << "result=success\n";
    return kExitOk;
}
}

int main() {
    int argc = 0;
    wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv == nullptr) {
        std::cerr << "error=CommandLineToArgvW failed: "
                  << win32_message(GetLastError()) << "\n";
        return kExitUsage;
    }
    const int result = run(argc, argv);
    LocalFree(argv);
    return result;
}
