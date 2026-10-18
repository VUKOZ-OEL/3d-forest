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

/** @file Process.hpp */

#ifndef PROCESS_HPP
#define PROCESS_HPP

// Include std.
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

// Include 3D Forest.
#include <ProcessEnvironment.hpp>

// Include local.
#include <ExportCore.hpp>
#include <WarningsDisable.hpp>

/** Cross-platform process control. */
class EXPORT_CORE Process
{
public:
    using FinishedCallback = std::function<void(int)>;

    Process();
    ~Process();
    Process(const Process &) = delete;
    Process &operator=(const Process &) = delete;

    void start(const std::string &program,
               const std::vector<std::string> &arguments,
               const ProcessEnvironment &environment);
    const std::string &startError() const;
    bool waitForStarted(int timeoutMilliseconds = 30000) const;
    bool waitForFinished(int timeoutMilliseconds = -1);
    void terminate();
    void kill();
    bool isRunning() const;
    void setFinishedCallback(FinishedCallback callback);

    /** Start a detached process. Arguments exclude the program name.
     *  Returns true if the process was created, and optionally stores its PID.
     */
    static bool startDetached(const std::string &program,
                              const std::vector<std::string> &arguments,
                              int64_t *pid = nullptr);

    static bool startDetached(const std::string &program,
                              const std::vector<std::string> &arguments,
                              const ProcessEnvironment &environment,
                              int64_t *pid = nullptr);

    /** Open url with the supplied port in the system's default browser.
     *  The URL should omit its port, for example "http://localhost".
     */
    static bool openUrl(const std::string &url, int port);

private:
    struct Impl;
    void monitor();

    std::unique_ptr<Impl> impl_;
    std::thread monitorThread_;
    mutable std::mutex mutex_;
    std::condition_variable finishedCondition_;
    FinishedCallback finishedCallback_;
    std::atomic<bool> running_{false};
    std::atomic<bool> started_{false};
    std::atomic<int> exitCode_{-1};
    bool finished_{true};
    std::string startError_;
};

#include <WarningsEnable.hpp>

#endif /* PROCESS_HPP */
