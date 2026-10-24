#include "compat.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
QString QCoreApplication::applicationDirPath() {
  std::vector<wchar_t> path(32768);
  auto size =
      GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
  if (!size || size == path.size())
    throw std::runtime_error("Cannot find executable directory");
  return QString(std::filesystem::path(std::wstring(path.data(), size))
                     .parent_path()
                     .generic_u8string());
}
void QProcess::startCommand(const QString &command) {
  SECURITY_ATTRIBUTES sa{sizeof(sa), nullptr, TRUE};
  HANDLE readOut = nullptr, writeOut = nullptr, readErr = nullptr,
         writeErr = nullptr;
  if (!CreatePipe(&readOut, &writeOut, &sa, 0) ||
      !CreatePipe(&readErr, &writeErr, &sa, 0))
    throw std::runtime_error("Cannot create process pipes");
  SetHandleInformation(readOut, HANDLE_FLAG_INHERIT, 0);
  SetHandleInformation(readErr, HANDLE_FLAG_INHERIT, 0);
  STARTUPINFOW startup{};
  startup.cb = sizeof startup;
  startup.dwFlags = STARTF_USESTDHANDLES;
  startup.hStdOutput = writeOut;
  startup.hStdError = writeErr;
  startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
  PROCESS_INFORMATION process{};
  std::wstring cmd;
  for (auto c : command)
    cmd += wchar_t(c);
  BOOL ok =
      CreateProcessW(nullptr, cmd.data(), nullptr, nullptr, TRUE,
                     CREATE_NO_WINDOW, nullptr, nullptr, &startup, &process);
  CloseHandle(writeOut);
  CloseHandle(writeErr);
  if (!ok) {
    CloseHandle(readOut);
    CloseHandle(readErr);
    throw std::runtime_error("Cannot start process");
  }
  auto drain = [](HANDLE h, QByteArray &b) {
    char buf[4096];
    DWORD n;
    while (ReadFile(h, buf, sizeof buf, &n, nullptr) && n)
      b.append(buf, n);
    CloseHandle(h);
  };
  std::thread stderrReader(drain, readErr, std::ref(error_));
  drain(readOut, output_);
  stderrReader.join();
  WaitForSingleObject(process.hProcess, INFINITE);
  DWORD code;
  GetExitCodeProcess(process.hProcess, &code);
  exit_ = static_cast<int>(code);
  CloseHandle(process.hThread);
  CloseHandle(process.hProcess);
}
#else
#include <sys/wait.h>
#include <unistd.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif
QString QCoreApplication::applicationDirPath() {
#ifdef __APPLE__
  uint32_t size = 0;
  _NSGetExecutablePath(nullptr, &size);
  std::vector<char> path(size);
  if (_NSGetExecutablePath(path.data(), &size))
    throw std::runtime_error("Cannot find executable directory");
  return QString(
      std::filesystem::canonical(path.data()).parent_path().generic_u8string());
#else
  std::error_code error;
  auto path = std::filesystem::read_symlink("/proc/self/exe", error);
  if (error)
    throw std::runtime_error("Cannot find executable directory");
  return QString(path.parent_path().generic_u8string());
#endif
}
void QProcess::startCommand(const QString &command) {
  // Tokenize a direct executable command, matching startCommand's shell-free
  // API.
  std::istringstream in(command.toStdString());
  std::vector<std::string> args;
  std::string arg;
  while (in >> std::quoted(arg))
    args.push_back(arg);
  if (args.empty())
    throw std::runtime_error("Empty command");
  int out[2], err[2];
  if (pipe(out) || pipe(err))
    throw std::runtime_error("Cannot create process pipes");
  pid_t pid = fork();
  if (pid == 0) {
    dup2(out[1], STDOUT_FILENO);
    dup2(err[1], STDERR_FILENO);
    close(out[0]);
    close(out[1]);
    close(err[0]);
    close(err[1]);
    std::vector<char *> argv;
    for (auto &s : args)
      argv.push_back(s.data());
    argv.push_back(nullptr);
    execvp(argv[0], argv.data());
    _exit(127);
  }
  close(out[1]);
  close(err[1]);
  if (pid < 0) {
    close(out[0]);
    close(err[0]);
    throw std::runtime_error("Cannot start process");
  }
  auto drain = [](int fd, QByteArray &b) {
    char buf[4096];
    ssize_t n;
    while ((n = read(fd, buf, sizeof buf)) > 0)
      b.append(buf, n);
    close(fd);
  };
  std::thread reader(drain, err[0], std::ref(error_));
  drain(out[0], output_);
  reader.join();
  int status;
  waitpid(pid, &status, 0);
  exit_ = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}
#endif
