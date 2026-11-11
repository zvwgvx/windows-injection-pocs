// Technique 01 self-test. It launches an unmodified target, verifies that the
// payload is absent, performs the injection through the real injector binary,
// verifies the new module, and checks that a second injection is refused.
#include <windows.h>
#include <shellapi.h>
#include <cstdio>
#include <cwchar>
#include <iostream>
#include <string>
#include <vector>

namespace {
enum InjectorExit {
    kInjectorOk = 0,
    kInjectorPayloadAbsent = 3,
    kInjectorAlreadyLoaded = 6,
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

std::wstring quote(const std::wstring& value) {
    return L"\"" + value + L"\"";
}

bool run_and_wait(const std::wstring& command, DWORD timeout_ms, DWORD& exit_code) {
    std::vector<wchar_t> buffer(command.begin(), command.end());
    buffer.push_back(0);

    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION process_info{};
    if (!CreateProcessW(nullptr, buffer.data(), nullptr, nullptr, TRUE, 0, nullptr,
                        nullptr, &startup, &process_info)) {
        std::cerr << "error=CreateProcessW failed: " << win32_message(GetLastError())
                  << "\n";
        return false;
    }

    Handle process(process_info.hProcess);
    Handle thread(process_info.hThread);
    const DWORD wait = WaitForSingleObject(process.get(), timeout_ms);
    if (wait != WAIT_OBJECT_0) {
        TerminateProcess(process.get(), 99);
        WaitForSingleObject(process.get(), 5000);
        std::cerr << "error=child process timed out\n";
        return false;
    }
    if (!GetExitCodeProcess(process.get(), &exit_code)) {
        std::cerr << "error=GetExitCodeProcess failed: "
                  << win32_message(GetLastError()) << "\n";
        return false;
    }
    return true;
}

bool launch_hidden(const std::wstring& command, PROCESS_INFORMATION& process_info) {
    SECURITY_ATTRIBUTES security{};
    security.nLength = sizeof(security);
    security.bInheritHandle = TRUE;

    Handle null_device(CreateFileW(L"NUL", GENERIC_READ | GENERIC_WRITE,
                                   FILE_SHARE_READ | FILE_SHARE_WRITE, &security,
                                   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));

    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    if (null_device) {
        startup.dwFlags = STARTF_USESTDHANDLES;
        startup.hStdInput = null_device.get();
        startup.hStdOutput = null_device.get();
        startup.hStdError = null_device.get();
    }

    std::vector<wchar_t> buffer(command.begin(), command.end());
    buffer.push_back(0);

    const BOOL created = CreateProcessW(
        nullptr, buffer.data(), nullptr, nullptr, null_device ? TRUE : FALSE,
        CREATE_NEW_PROCESS_GROUP, nullptr, nullptr, &startup, &process_info);
    if (!created) {
        std::cerr << "error=CreateProcessW(target) failed: "
                  << win32_message(GetLastError()) << "\n";
        return false;
    }
    return true;
}

struct Options {
    std::wstring target;
    std::wstring injector;
    std::wstring payload;
    DWORD startup_ms = 1200;
    DWORD injector_timeout_ms = 15000;
};

bool parse_dword(const wchar_t* text, DWORD minimum, DWORD maximum, DWORD& result) {
    if (text == nullptr || *text == 0 || *text == L'-') {
        return false;
    }
    wchar_t* end = nullptr;
    const unsigned long long value = std::wcstoull(text, &end, 10);
    if (end == text || *end != 0 || value < minimum || value > maximum) {
        return false;
    }
    result = static_cast<DWORD>(value);
    return true;
}

bool parse_options(int argc, wchar_t** argv, Options& options) {
    for (int i = 1; i < argc; ++i) {
        const std::wstring argument = argv[i];
        if (argument == L"--target" || argument == L"--injector" ||
            argument == L"--payload" || argument == L"--startup-ms" ||
            argument == L"--injector-timeout-ms") {
            if (i + 1 >= argc) {
                return false;
            }
            const wchar_t* value = argv[++i];
            if (argument == L"--target") {
                options.target = value;
            } else if (argument == L"--injector") {
                options.injector = value;
            } else if (argument == L"--payload") {
                options.payload = value;
            } else if (argument == L"--startup-ms") {
                if (!parse_dword(value, 100, 10000, options.startup_ms)) {
                    return false;
                }
            } else {
                if (!parse_dword(value, 1000, 60000, options.injector_timeout_ms)) {
                    return false;
                }
            }
            continue;
        }
        return false;
    }
    return !options.target.empty() && !options.injector.empty() &&
           !options.payload.empty();
}

bool expect(const char* step, DWORD expected, DWORD actual) {
    const bool pass = expected == actual;
    std::cout << "step=" << step << " expected=" << expected << " actual=" << actual
              << " pass=" << (pass ? "true" : "false") << "\n";
    return pass;
}

int run(int argc, wchar_t** argv) {
    Options options;
    if (!parse_options(argc, argv, options)) {
        std::cerr << "Usage: poc01-selftest --target <exe> --injector <exe> "
                     "--payload <dll>\n";
        return 1;
    }

    const std::wstring target_command =
        quote(options.target) + L" --duration-sec 30 --interval-ms 100";
    PROCESS_INFORMATION target_info{};
    if (!launch_hidden(target_command, target_info)) {
        return 2;
    }
    Handle target_process(target_info.hProcess);
    Handle target_thread(target_info.hThread);
    const DWORD target_pid = target_info.dwProcessId;

    std::cout << "selftest=dll-injection\n"
              << "target_pid=" << target_pid << "\n"
              << "target_path=" << utf8(options.target) << "\n";

    Sleep(options.startup_ms);
    DWORD target_state = 0;
    if (!GetExitCodeProcess(target_process.get(), &target_state) ||
        target_state != STILL_ACTIVE) {
        std::cerr << "error=target exited before injection\n";
        return 3;
    }

    const std::wstring base_command = quote(options.injector) + L" --pid " +
        std::to_wstring(target_pid) + L" --dll " + quote(options.payload) +
        L" --timeout-ms " + std::to_wstring(options.injector_timeout_ms);

    bool passed = true;
    DWORD exit_code = 0;

    std::cout << "phase=precondition\n";
    if (run_and_wait(base_command + L" --check-only", options.injector_timeout_ms,
                     exit_code)) {
        passed &= expect("payload_absent_before_injection", kInjectorPayloadAbsent,
                         exit_code);
    } else {
        passed = false;
    }

    std::cout << "phase=inject\n";
    if (run_and_wait(base_command, options.injector_timeout_ms, exit_code)) {
        passed &= expect("inject", kInjectorOk, exit_code);
    } else {
        passed = false;
    }

    std::cout << "phase=postcondition\n";
    if (run_and_wait(base_command + L" --check-only", options.injector_timeout_ms,
                     exit_code)) {
        passed &= expect("payload_present_after_injection", kInjectorOk, exit_code);
    } else {
        passed = false;
    }

    std::cout << "phase=duplicate\n";
    if (run_and_wait(base_command, options.injector_timeout_ms, exit_code)) {
        passed &= expect("duplicate_injection_refused", kInjectorAlreadyLoaded,
                         exit_code);
    } else {
        passed = false;
    }

    std::cout << "phase=target_health\n";
    target_state = 0;
    if (!GetExitCodeProcess(target_process.get(), &target_state)) {
        std::cerr << "error=GetExitCodeProcess(target) failed: "
                  << win32_message(GetLastError()) << "\n";
        passed = false;
    } else {
        const bool alive = target_state == STILL_ACTIVE;
        std::cout << "step=target_still_alive expected=1 actual=" << (alive ? 1 : 0)
                  << " pass=" << (alive ? "true" : "false") << "\n";
        passed &= alive;
    }

    TerminateProcess(target_process.get(), 0);
    WaitForSingleObject(target_process.get(), 5000);

    std::cout << "result=" << (passed ? "pass" : "fail") << "\n";
    return passed ? 0 : 4;
}
}

int main() {
    int argc = 0;
    wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv == nullptr) {
        std::cerr << "error=CommandLineToArgvW failed: "
                  << win32_message(GetLastError()) << "\n";
        return 1;
    }
    const int result = run(argc, argv);
    LocalFree(argv);
    return result;
}
