/*
    Copyright 2020 VUKOZ

    This file is part of 3D Forest.

    3D Forest is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    3D Forest is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with 3D Forest.  If not, see <https://www.gnu.org/licenses/>.
*/

/** @file Process.cpp */

// Include 3D Forest.
#include <Process.hpp>

// Include std.
#include <algorithm>
#include <cerrno>
#include <chrono>
#include <string>
#include <system_error>
#include <thread>
#include <vector>

#if defined(_WIN32)
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <Windows.h>

    #include <Shellapi.h> // After Windows.h
#else
    #include <signal.h>
    #include <spawn.h>
    #include <sys/types.h>
    #include <sys/wait.h>
#endif

namespace
{
#if defined(_WIN32)
std::wstring toWide(const std::string &value)
{
    if (value.empty())
    {
        return std::wstring();
    }

    int size = MultiByteToWideChar(CP_UTF8,
                                   MB_ERR_INVALID_CHARS,
                                   value.data(),
                                   static_cast<int>(value.size()),
                                   nullptr,
                                   0);
    if (size <= 0)
    {
        return std::wstring();
    }

    std::wstring result(static_cast<size_t>(size), L'\0');
    if (MultiByteToWideChar(CP_UTF8,
                            MB_ERR_INVALID_CHARS,
                            value.data(),
                            static_cast<int>(value.size()),
                            &result[0],
                            size) <= 0)
    {
        return std::wstring();
    }
    return result;
}

std::wstring resolveWindowsProgram(const std::wstring &program)
{
    // CreateProcessW does not search PATH when lpApplicationName is provided.
    // Resolve bare executable names explicitly so callers can use e.g.
    // "python".
    if (program.find_first_of(L"\\/") != std::wstring::npos ||
        (program.size() > 1 && program[1] == L':'))
    {
        return program;
    }

    DWORD required =
        SearchPathW(nullptr, program.c_str(), L".exe", 0, nullptr, nullptr);
    if (required == 0)
    {
        return std::wstring();
    }

    std::vector<wchar_t> buffer(static_cast<size_t>(required) + 1);
    DWORD length = SearchPathW(nullptr,
                               program.c_str(),
                               L".exe",
                               static_cast<DWORD>(buffer.size()),
                               buffer.data(),
                               nullptr);
    if (length == 0 || length >= buffer.size())
    {
        return std::wstring();
    }
    return std::wstring(buffer.data(), length);
}

void appendWindowsArgument(std::wstring &commandLine,
                           const std::wstring &argument)
{
    if (!commandLine.empty())
    {
        commandLine += L' ';
    }

    commandLine += L'"';
    size_t backslashes = 0;
    for (wchar_t c : argument)
    {
        if (c == L'\\')
        {
            ++backslashes;
        }
        else if (c == L'"')
        {
            commandLine.append(backslashes * 2 + 1, L'\\');
            commandLine += c;
            backslashes = 0;
        }
        else
        {
            commandLine.append(backslashes, L'\\');
            commandLine += c;
            backslashes = 0;
        }
    }
    commandLine.append(backslashes * 2, L'\\');
    commandLine += L'"';
}
#endif
} // namespace

struct Process::Impl
{
#if defined(_WIN32)
    HANDLE process = nullptr;
    DWORD processId = 0;
#else
    pid_t processId = -1;
#endif
};

Process::Process() : impl_(new Impl())
{
}

Process::~Process()
{
    setFinishedCallback(FinishedCallback());
    if (isRunning())
    {
        kill();
        waitForFinished();
    }
    if (monitorThread_.joinable())
    {
        monitorThread_.join();
    }
#if defined(_WIN32)
    if (impl_->process)
    {
        CloseHandle(impl_->process);
        impl_->process = nullptr;
    }
#endif
}

void Process::start(const std::string &program,
                    const std::vector<std::string> &arguments,
                    const ProcessEnvironment &environment)
{
    if (isRunning())
    {
        return;
    }
    if (monitorThread_.joinable())
    {
        monitorThread_.join();
    }
#if defined(_WIN32)
    if (impl_->process)
    {
        CloseHandle(impl_->process);
        impl_->process = nullptr;
    }
#endif
    started_ = false;
    running_ = false;
    exitCode_ = -1;
    startError_.clear();
    {
        std::lock_guard<std::mutex> lock(mutex_);
        finished_ = true;
    }
    if (program.empty())
    {
        startError_ = "program path is empty";
        return;
    }

#if defined(_WIN32)
    std::wstring wideProgram = toWide(program);
    if (wideProgram.empty())
    {
        startError_ = "program path is not valid UTF-8";
        return;
    }
    std::wstring resolvedProgram = resolveWindowsProgram(wideProgram);
    if (resolvedProgram.empty())
    {
        startError_ = "program was not found on PATH";
        return;
    }
    std::wstring commandLine;
    appendWindowsArgument(commandLine, resolvedProgram);
    for (const std::string &argument : arguments)
    {
        std::wstring wideArgument = toWide(argument);
        if (!argument.empty() && wideArgument.empty())
        {
            startError_ = "an argument is not valid UTF-8";
            return;
        }
        appendWindowsArgument(commandLine, wideArgument);
    }

    std::vector<std::wstring> wideEnvironment;
    wideEnvironment.reserve(environment.values().size());
    for (const auto &entry : environment.values())
    {
        std::wstring name = toWide(entry.first);
        std::wstring value = toWide(entry.second);
        if (name.empty() || (!entry.second.empty() && value.empty()))
        {
            startError_ = "the process environment contains invalid UTF-8";
            return;
        }
        wideEnvironment.push_back(name + L"=" + value);
    }
    std::sort(wideEnvironment.begin(),
              wideEnvironment.end(),
              [](const std::wstring &a, const std::wstring &b)
              { return _wcsicmp(a.c_str(), b.c_str()) < 0; });
    std::vector<wchar_t> environmentBlock;
    for (const std::wstring &entry : wideEnvironment)
    {
        environmentBlock.insert(environmentBlock.end(),
                                entry.begin(),
                                entry.end());
        environmentBlock.push_back(L'\0');
    }
    environmentBlock.push_back(L'\0');
    if (wideEnvironment.empty())
    {
        environmentBlock.push_back(L'\0');
    }

    STARTUPINFOW startupInfo{};
    startupInfo.cb = sizeof(startupInfo);
    PROCESS_INFORMATION processInfo{};
    if (!CreateProcessW(resolvedProgram.c_str(),
                        &commandLine[0],
                        nullptr,
                        nullptr,
                        FALSE,
                        DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP |
                            CREATE_UNICODE_ENVIRONMENT,
                        environmentBlock.data(),
                        nullptr,
                        &startupInfo,
                        &processInfo))
    {
        const DWORD error = GetLastError();
        startError_ = std::system_category().message(static_cast<int>(error));
        return;
    }
    CloseHandle(processInfo.hThread);
    impl_->process = processInfo.hProcess;
    impl_->processId = processInfo.dwProcessId;
#else
    std::vector<char *> argv;
    argv.reserve(arguments.size() + 2);
    argv.push_back(const_cast<char *>(program.c_str()));
    for (const std::string &argument : arguments)
    {
        argv.push_back(const_cast<char *>(argument.c_str()));
    }
    argv.push_back(nullptr);

    std::vector<std::string> environmentStrings;
    std::vector<char *> envp;
    environmentStrings.reserve(environment.values().size());
    envp.reserve(environment.values().size() + 1);
    for (const auto &entry : environment.values())
    {
        environmentStrings.push_back(entry.first + "=" + entry.second);
    }
    for (std::string &entry : environmentStrings)
    {
        envp.push_back(&entry[0]);
    }
    envp.push_back(nullptr);

    pid_t child = 0;
    int result = posix_spawnp(&child,
                              program.c_str(),
                              nullptr,
                              nullptr,
                              argv.data(),
                              envp.data());
    if (result != 0)
    {
        startError_ = std::system_category().message(result);
        return;
    }
    impl_->processId = child;
#endif

    {
        std::lock_guard<std::mutex> lock(mutex_);
        finished_ = false;
    }
    started_ = true;
    running_ = true;
    monitorThread_ = std::thread(&Process::monitor, this);
}

const std::string &Process::startError() const
{
    return startError_;
}

bool Process::waitForStarted(int timeoutMilliseconds) const
{
    (void)timeoutMilliseconds;
    return started_.load();
}

bool Process::waitForFinished(int timeoutMilliseconds)
{
    bool finished;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (timeoutMilliseconds < 0)
        {
            finishedCondition_.wait(lock, [this]() { return finished_; });
            finished = true;
        }
        else
        {
            finished = finishedCondition_.wait_for(
                lock,
                std::chrono::milliseconds(timeoutMilliseconds),
                [this]() { return finished_; });
        }
    }
    if (finished && monitorThread_.joinable() &&
        monitorThread_.get_id() != std::this_thread::get_id())
    {
        monitorThread_.join();
    }
    return finished;
}

void Process::terminate()
{
    if (!isRunning())
    {
        return;
    }
#if defined(_WIN32)
    TerminateProcess(impl_->process, 1);
#else
    ::kill(impl_->processId, SIGTERM);
#endif
}

void Process::kill()
{
    if (!isRunning())
    {
        return;
    }
#if defined(_WIN32)
    TerminateProcess(impl_->process, 1);
#else
    ::kill(impl_->processId, SIGKILL);
#endif
}

bool Process::isRunning() const
{
    return running_.load();
}

void Process::setFinishedCallback(FinishedCallback callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    finishedCallback_ = std::move(callback);
}

void Process::monitor()
{
    int code = -1;
#if defined(_WIN32)
    WaitForSingleObject(impl_->process, INFINITE);
    DWORD exitCode = 1;
    if (GetExitCodeProcess(impl_->process, &exitCode))
    {
        code = static_cast<int>(exitCode);
    }
#else
    int status = 0;
    pid_t result;
    do
    {
        result = waitpid(impl_->processId, &status, 0);
    } while (result < 0 && errno == EINTR);
    if (result == impl_->processId)
    {
        code = WIFEXITED(status)
                   ? WEXITSTATUS(status)
                   : (WIFSIGNALED(status) ? 128 + WTERMSIG(status) : -1);
    }
#endif
    exitCode_ = code;
    running_ = false;
    FinishedCallback callback;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        finished_ = true;
        callback = finishedCallback_;
    }
    finishedCondition_.notify_all();
    if (callback)
    {
        callback(code);
    }
}

bool Process::startDetached(const std::string &program,
                            const std::vector<std::string> &arguments,
                            int64_t *pid)
{
    return startDetached(program,
                         arguments,
                         ProcessEnvironment::systemEnvironment(),
                         pid);
}

bool Process::startDetached(const std::string &program,
                            const std::vector<std::string> &arguments,
                            const ProcessEnvironment &environment,
                            int64_t *pid)
{
    if (program.empty())
    {
        return false;
    }

#if defined(_WIN32)
    std::wstring wideProgram = toWide(program);
    if (wideProgram.empty())
    {
        return false;
    }

    std::wstring commandLine;
    appendWindowsArgument(commandLine, wideProgram);
    for (const std::string &argument : arguments)
    {
        std::wstring wideArgument = toWide(argument);
        if (!argument.empty() && wideArgument.empty())
        {
            return false;
        }
        appendWindowsArgument(commandLine, wideArgument);
    }

    std::vector<std::wstring> wideEnvironment;
    wideEnvironment.reserve(environment.values().size());
    for (const auto &entry : environment.values())
    {
        std::wstring name = toWide(entry.first);
        std::wstring value = toWide(entry.second);
        if (name.empty() || (!entry.second.empty() && value.empty()))
        {
            return false;
        }
        wideEnvironment.push_back(name + L"=" + value);
    }
    std::sort(wideEnvironment.begin(),
              wideEnvironment.end(),
              [](const std::wstring &a, const std::wstring &b)
              { return _wcsicmp(a.c_str(), b.c_str()) < 0; });
    std::vector<wchar_t> environmentBlock;
    for (const std::wstring &entry : wideEnvironment)
    {
        environmentBlock.insert(environmentBlock.end(),
                                entry.begin(),
                                entry.end());
        environmentBlock.push_back(L'\0');
    }
    environmentBlock.push_back(L'\0');
    if (wideEnvironment.empty())
    {
        environmentBlock.push_back(L'\0');
    }

    STARTUPINFOW startupInfo{};
    startupInfo.cb = sizeof(startupInfo);
    PROCESS_INFORMATION processInfo{};
    BOOL created = CreateProcessW(wideProgram.c_str(),
                                  &commandLine[0],
                                  nullptr,
                                  nullptr,
                                  FALSE,
                                  DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP |
                                      CREATE_UNICODE_ENVIRONMENT,
                                  environmentBlock.data(),
                                  nullptr,
                                  &startupInfo,
                                  &processInfo);
    if (!created)
    {
        return false;
    }

    if (pid)
    {
        *pid = static_cast<int64_t>(processInfo.dwProcessId);
    }
    CloseHandle(processInfo.hThread);
    CloseHandle(processInfo.hProcess);
    return true;
#else
    std::vector<char *> argv;
    argv.reserve(arguments.size() + 2);
    argv.push_back(const_cast<char *>(program.c_str()));
    for (const std::string &argument : arguments)
    {
        argv.push_back(const_cast<char *>(argument.c_str()));
    }
    argv.push_back(nullptr);

    std::vector<std::string> environmentStrings;
    std::vector<char *> envp;
    environmentStrings.reserve(environment.values().size());
    envp.reserve(environment.values().size() + 1);
    for (const auto &entry : environment.values())
    {
        environmentStrings.push_back(entry.first + "=" + entry.second);
    }
    for (std::string &entry : environmentStrings)
    {
        envp.push_back(&entry[0]);
    }
    envp.push_back(nullptr);

    pid_t child = 0;
    int result = posix_spawnp(&child,
                              program.c_str(),
                              nullptr,
                              nullptr,
                              argv.data(),
                              envp.data());
    if (result != 0)
    {
        errno = result;
        return false;
    }

    if (pid)
    {
        *pid = static_cast<int64_t>(child);
    }
    return true;
#endif
}

bool Process::openUrl(const std::string &url, int port)
{
    if (url.empty() || port < 1 || port > 65535)
    {
        return false;
    }

    std::string urlWithPort = url + ":" + std::to_string(port);
#if defined(_WIN32)
    std::wstring wideUrl = toWide(urlWithPort);
    if (wideUrl.empty())
    {
        return false;
    }
    HINSTANCE result = ShellExecuteW(nullptr,
                                     L"open",
                                     wideUrl.c_str(),
                                     nullptr,
                                     nullptr,
                                     SW_SHOWNORMAL);
    return reinterpret_cast<intptr_t>(result) > 32;
#elif defined(__APPLE__)
    std::shared_ptr<Process> browser = std::make_shared<Process>();
    browser->start("open",
                   {urlWithPort},
                   ProcessEnvironment::systemEnvironment());
    if (!browser->waitForStarted())
    {
        return false;
    }
    std::thread([browser]() { (void)browser->waitForFinished(); }).detach();
    return true;
#else
    std::shared_ptr<Process> browser = std::make_shared<Process>();
    browser->start("xdg-open",
                   {urlWithPort},
                   ProcessEnvironment::systemEnvironment());
    if (!browser->waitForStarted())
    {
        return false;
    }
    std::thread([browser]() { (void)browser->waitForFinished(); }).detach();
    return true;
#endif
}
