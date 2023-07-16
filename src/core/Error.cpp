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

/** @file Error.cpp */

#include <cerrno>
#include <cstdio>

#if defined(_MSC_VER)
    #include <iomanip>
    #include <sstream>
    #include <windows.h>
#endif

#include <Error.hpp>

#define LOG_MODULE_NAME "Error"
#include <Log.hpp>

std::string getErrorString(int errnum)
{
    const char *msg = nullptr;

#if defined(EPERM)
    if (errnum == EPERM)
        msg = "Operation not permitted.";
#endif
#if defined(ENOENT)
    if (errnum == ENOENT)
        msg = "No such file or directory.";
#endif
#if defined(ESRCH)
    if (errnum == ESRCH)
        msg = "No such process.";
#endif
#if defined(EINTR)
    if (errnum == EINTR)
        msg = "Interrupted function.";
#endif
#if defined(EIO)
    if (errnum == EIO)
        msg = "I/O error.";
#endif
#if defined(ENXIO)
    if (errnum == ENXIO)
        msg = "No such device or address.";
#endif
#if defined(E2BIG)
    if (errnum == E2BIG)
        msg = "Argument list too long.";
#endif
#if defined(ENOEXEC)
    if (errnum == ENOEXEC)
        msg = "Executable file format error.";
#endif
#if defined(EBADF)
    if (errnum == EBADF)
        msg = "Bad file descriptor.";
#endif
#if defined(ECHILD)
    if (errnum == ECHILD)
        msg = "No child processes.";
#endif
#if defined(EAGAIN)
    if (errnum == EAGAIN)
        msg = "Resource unavailable, try again.";
#endif
#if defined(ENOMEM)
    if (errnum == ENOMEM)
        msg = "Not enough space.";
#endif
#if defined(EACCES)
    if (errnum == EACCES)
        msg = "Permission denied.";
#endif
#if defined(EFAULT)
    if (errnum == EFAULT)
        msg = "Bad address.";
#endif
#if defined(ENOTBLK)
    if (errnum == ENOTBLK)
        msg = "Block device required.";
#endif
#if defined(EBUSY)
    if (errnum == EBUSY)
        msg = "Device or resource busy.";
#endif
#if defined(EEXIST)
    if (errnum == EEXIST)
        msg = "File exists.";
#endif
#if defined(EXDEV)
    if (errnum == EXDEV)
        msg = "Invalid cross-device link. ";
#endif
#if defined(ENODEV)
    if (errnum == ENODEV)
        msg = "No such device.";
#endif
#if defined(ENOTDIR)
    if (errnum == ENOTDIR)
        msg = "Not a directory or a symbolic link to a directory.";
#endif
#if defined(EISDIR)
    if (errnum == EISDIR)
        msg = "Is a directory.";
#endif
#if defined(EINVAL)
    if (errnum == EINVAL)
        msg = "Invalid argument.";
#endif
#if defined(ENFILE)
    if (errnum == ENFILE)
        msg = "Too many files open in system.";
#endif
#if defined(EMFILE)
    if (errnum == EMFILE)
        msg = "File descriptor value too large.";
#endif
#if defined(ENOTTY)
    if (errnum == ENOTTY)
        msg = "Inappropriate I/O control operation.";
#endif
#if defined(ETXTBSY)
    if (errnum == ETXTBSY)
        msg = "Text file busy.";
#endif
#if defined(EFBIG)
    if (errnum == EFBIG)
        msg = "File too large.";
#endif
#if defined(ENOSPC)
    if (errnum == ENOSPC)
        msg = "No space left on device.";
#endif
#if defined(ESPIPE)
    if (errnum == ESPIPE)
        msg = "Invalid seek.";
#endif
#if defined(EROFS)
    if (errnum == EROFS)
        msg = "Read-only file system.";
#endif
#if defined(EMLINK)
    if (errnum == EMLINK)
        msg = "Too many links.";
#endif
#if defined(EPIPE)
    if (errnum == EPIPE)
        msg = "Broken pipe.";
#endif
#if defined(EDOM)
    if (errnum == EDOM)
        msg = "Mathematics argument out of domain of function.";
#endif
#if defined(ERANGE)
    if (errnum == ERANGE)
        msg = "Result too large.";
#endif
#if defined(EDEADLK)
    if (errnum == EDEADLK)
        msg = "Resource deadlock would occur.";
#endif
#if defined(ENAMETOOLONG)
    if (errnum == ENAMETOOLONG)
        msg = "Filename too long.";
#endif
#if defined(ENOLCK)
    if (errnum == ENOLCK)
        msg = "No locks available.";
#endif
#if defined(ENOSYS)
    if (errnum == ENOSYS)
        msg = "Functionality not supported.";
#endif
#if defined(ENOTEMPTY)
    if (errnum == ENOTEMPTY)
        msg = "Directory not empty.";
#endif
#if defined(ELOOP)
    if (errnum == ELOOP)
        msg = "Too many levels of symbolic links.";
#endif
#if defined(EWOULDBLOCK)
    if (errnum == EWOULDBLOCK)
        msg = "Operation would block.";
#endif
#if defined(ENOMSG)
    if (errnum == ENOMSG)
        msg = "No message of the desired type.";
#endif
#if defined(EIDRM)
    if (errnum == EIDRM)
        msg = "Identifier removed.";
#endif
#if defined(ECHRNG)
    if (errnum == ECHRNG)
        msg = "Channel number out of range.";
#endif
#if defined(EL2NSYNC)
    if (errnum == EL2NSYNC)
        msg = "Level 2 not synchronized.";
#endif
#if defined(EL3HLT)
    if (errnum == EL3HLT)
        msg = "Level 3 halted.";
#endif
#if defined(EL3RST)
    if (errnum == EL3RST)
        msg = "Level 3 reset.";
#endif
#if defined(ELNRNG)
    if (errnum == ELNRNG)
        msg = "Link number out of range.";
#endif
#if defined(EUNATCH)
    if (errnum == EUNATCH)
        msg = "Protocol driver not attached.";
#endif
#if defined(ENOCSI)
    if (errnum == ENOCSI)
        msg = "No CSI structure available.";
#endif
#if defined(EL2HLT)
    if (errnum == EL2HLT)
        msg = "Level 2 halted.";
#endif
#if defined(EBADE)
    if (errnum == EBADE)
        msg = "Invalid exchange.";
#endif
#if defined(EBADR)
    if (errnum == EBADR)
        msg = "Invalid request descriptor.";
#endif
#if defined(EXFULL)
    if (errnum == EXFULL)
        msg = "Exchange full.";
#endif
#if defined(ENOANO)
    if (errnum == ENOANO)
        msg = "No anode.";
#endif
#if defined(EBADRQC)
    if (errnum == EBADRQC)
        msg = "Invalid request code.";
#endif
#if defined(EBADSLT)
    if (errnum == EBADSLT)
        msg = "Invalid slot.";
#endif
#if defined(EDEADLOCK)
    if (errnum == EDEADLOCK)
        msg = "Resource deadlock avoided.";
#endif
#if defined(EBFONT)
    if (errnum == EBFONT)
        msg = "Bad font file format.";
#endif
#if defined(ENOSTR)
    if (errnum == ENOSTR)
        msg = "Not a STREAM.";
#endif
#if defined(ENODATA)
    if (errnum == ENODATA)
        msg = "No message is available on the STREAM head read queue.";
#endif
#if defined(ETIME)
    if (errnum == ETIME)
        msg = "Stream ioctl() timeout.";
#endif
#if defined(ENOSR)
    if (errnum == ENOSR)
        msg = "No STREAM resources.";
#endif
#if defined(ENONET)
    if (errnum == ENONET)
        msg = "Machine is not on the network.";
#endif
#if defined(ENOPKG)
    if (errnum == ENOPKG)
        msg = "Package not installed.";
#endif
#if defined(EREMOTE)
    if (errnum == EREMOTE)
        msg = "Object is remote.";
#endif
#if defined(ENOLINK)
    if (errnum == ENOLINK)
        msg = "Link has been severed.";
#endif
#if defined(EADV)
    if (errnum == EADV)
        msg = "Advertise error.";
#endif
#if defined(ESRMNT)
    if (errnum == ESRMNT)
        msg = "Srmount error.";
#endif
#if defined(ECOMM)
    if (errnum == ECOMM)
        msg = "Communication error on send.";
#endif
#if defined(EPROTO)
    if (errnum == EPROTO)
        msg = "Protocol error.";
#endif
#if defined(EMULTIHOP)
    if (errnum == EMULTIHOP)
        msg = "Multihop is not allowed.";
#endif
#if defined(EDOTDOT)
    if (errnum == EDOTDOT)
        msg = "RFS specific error.";
#endif
#if defined(EBADMSG)
    if (errnum == EBADMSG)
        msg = "Bad message.";
#endif
#if defined(EOVERFLOW)
    if (errnum == EOVERFLOW)
        msg = "Value too large to be stored in data type.";
#endif
#if defined(ENOTUNIQ)
    if (errnum == ENOTUNIQ)
        msg = "Name not unique on network.";
#endif
#if defined(EBADFD)
    if (errnum == EBADFD)
        msg = "File descriptor in bad state.";
#endif
#if defined(EREMCHG)
    if (errnum == EREMCHG)
        msg = "Remote address changed.";
#endif
#if defined(ELIBACC)
    if (errnum == ELIBACC)
        msg = "Can not access a needed shared library.";
#endif
#if defined(ELIBBAD)
    if (errnum == ELIBBAD)
        msg = "Accessing a corrupted shared library.";
#endif
#if defined(ELIBSCN)
    if (errnum == ELIBSCN)
        msg = ".lib section in a.out corrupted.";
#endif
#if defined(ELIBMAX)
    if (errnum == ELIBMAX)
        msg = "Attempting to link in too many shared libraries.";
#endif
#if defined(ELIBEXEC)
    if (errnum == ELIBEXEC)
        msg = "Cannot exec a shared library directly.";
#endif
#if defined(EILSEQ)
    if (errnum == EILSEQ)
        msg = "Illegal byte sequence.";
#endif
#if defined(ERESTART)
    if (errnum == ERESTART)
        msg = "Interrupted system call should be restarted.";
#endif
#if defined(ESTRPIPE)
    if (errnum == ESTRPIPE)
        msg = "Streams pipe error.";
#endif
#if defined(EUSERS)
    if (errnum == EUSERS)
        msg = "Too many users.";
#endif
#if defined(ENOTSOCK)
    if (errnum == ENOTSOCK)
        msg = "Not a socket.";
#endif
#if defined(EDESTADDRREQ)
    if (errnum == EDESTADDRREQ)
        msg = "Destination address required.";
#endif
#if defined(EMSGSIZE)
    if (errnum == EMSGSIZE)
        msg = "Message too large.";
#endif
#if defined(EPROTOTYPE)
    if (errnum == EPROTOTYPE)
        msg = "Protocol wrong type for socket.";
#endif
#if defined(ENOPROTOOPT)
    if (errnum == ENOPROTOOPT)
        msg = "Protocol not available.";
#endif
#if defined(EPROTONOSUPPORT)
    if (errnum == EPROTONOSUPPORT)
        msg = "Protocol not supported.";
#endif
#if defined(ESOCKTNOSUPPORT)
    if (errnum == ESOCKTNOSUPPORT)
        msg = "Socket type not supported.";
#endif
#if defined(EOPNOTSUPP)
    if (errnum == EOPNOTSUPP)
        msg = "Operation not supported on socket.";
#endif
#if defined(ENOTSUP)
    if (errnum == ENOTSUP)
        msg = "Not supported.";
#endif
#if defined(EPFNOSUPPORT)
    if (errnum == EPFNOSUPPORT)
        msg = "Protocol family not supported.";
#endif
#if defined(EAFNOSUPPORT)
    if (errnum == EAFNOSUPPORT)
        msg = "Address family not supported.";
#endif
#if defined(EADDRINUSE)
    if (errnum == EADDRINUSE)
        msg = "Address in use.";
#endif
#if defined(EADDRNOTAVAIL)
    if (errnum == EADDRNOTAVAIL)
        msg = "Address not available.";
#endif
#if defined(ENETDOWN)
    if (errnum == ENETDOWN)
        msg = "Network is down.";
#endif
#if defined(ENETUNREACH)
    if (errnum == ENETUNREACH)
        msg = "Network unreachable.";
#endif
#if defined(ENETRESET)
    if (errnum == ENETRESET)
        msg = "Connection aborted by network.";
#endif
#if defined(ECONNABORTED)
    if (errnum == ECONNABORTED)
        msg = "Connection aborted.";
#endif
#if defined(ECONNRESET)
    if (errnum == ECONNRESET)
        msg = "Connection reset.";
#endif
#if defined(ENOBUFS)
    if (errnum == ENOBUFS)
        msg = "No buffer space available.";
#endif
#if defined(EISCONN)
    if (errnum == EISCONN)
        msg = "Socket is connected.";
#endif
#if defined(ENOTCONN)
    if (errnum == ENOTCONN)
        msg = "The socket is not connected.";
#endif
#if defined(ESHUTDOWN)
    if (errnum == ESHUTDOWN)
        msg = "Cannot send after transport endpoint shutdown.";
#endif
#if defined(ETOOMANYREFS)
    if (errnum == ETOOMANYREFS)
        msg = "Too many references: cannot splice.";
#endif
#if defined(ETIMEDOUT)
    if (errnum == ETIMEDOUT)
        msg = "Connection timed out.";
#endif
#if defined(ECONNREFUSED)
    if (errnum == ECONNREFUSED)
        msg = "Connection refused.";
#endif
#if defined(EHOSTDOWN)
    if (errnum == EHOSTDOWN)
        msg = "Host is down.";
#endif
#if defined(EHOSTUNREACH)
    if (errnum == EHOSTUNREACH)
        msg = "Host is unreachable.";
#endif
#if defined(EALREADY)
    if (errnum == EALREADY)
        msg = "Connection already in progress.";
#endif
#if defined(EINPROGRESS)
    if (errnum == EINPROGRESS)
        msg = "Operation in progress.";
#endif
#if defined(ESTALE)
    if (errnum == ESTALE)
        msg = "The file handle has expired.";
#endif
#if defined(EUCLEAN)
    if (errnum == EUCLEAN)
        msg = "Structure needs cleaning.";
#endif
#if defined(ENOTNAM)
    if (errnum == ENOTNAM)
        msg = "Not a XENIX named type file.";
#endif
#if defined(ENAVAIL)
    if (errnum == ENAVAIL)
        msg = "No XENIX semaphores available.";
#endif
#if defined(EISNAM)
    if (errnum == EISNAM)
        msg = "Is a named type file.";
#endif
#if defined(EREMOTEIO)
    if (errnum == EREMOTEIO)
        msg = "Remote I/O error.";
#endif
#if defined(EDQUOT)
    if (errnum == EDQUOT)
        msg = "Disk quota exceeded.";
#endif
#if defined(ENOMEDIUM)
    if (errnum == ENOMEDIUM)
        msg = "No medium found.";
#endif
#if defined(EMEDIUMTYPE)
    if (errnum == EMEDIUMTYPE)
        msg = "Wrong medium type.";
#endif
#if defined(ECANCELED)
    if (errnum == ECANCELED)
        msg = "Operation canceled.";
#endif
#if defined(ENOKEY)
    if (errnum == ENOKEY)
        msg = "Required key not available.";
#endif
#if defined(EKEYEXPIRED)
    if (errnum == EKEYEXPIRED)
        msg = "Key has expired.";
#endif
#if defined(EKEYREVOKED)
    if (errnum == EKEYREVOKED)
        msg = "Key has been revoked.";
#endif
#if defined(EKEYREJECTED)
    if (errnum == EKEYREJECTED)
        msg = "Key was rejected by service.";
#endif
#if defined(EOWNERDEAD)
    if (errnum == EOWNERDEAD)
        msg = "Previous owner died.";
#endif
#if defined(ENOTRECOVERABLE)
    if (errnum == ENOTRECOVERABLE)
        msg = "State not recoverable.";
#endif
#if defined(ERFKILL)
    if (errnum == ERFKILL)
        msg = "Operation not possible due to RF-kill.";
#endif
#if defined(EHWPOISON)
    if (errnum == EHWPOISON)
        msg = "Memory page has hardware error.";
#endif

#if defined(ELOAD)
    if (errnum == ELOAD)
        msg = "Load error.";
#endif

    if (msg)
    {
        return std::string(msg);
    }

    return std::string("Unknown error ") + std::to_string(errnum);
}

std::string getErrorString()
{
    return getErrorString(errno);
}

std::string getErrorString(const std::string &message)
{
    return message + ": " + getErrorString();
}

std::string getErrorStringWin(const std::string &message)
{
#if defined(_MSC_VER)
    LPTSTR errorText = NULL;
    DWORD error = GetLastError();

    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER |
                      FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  error,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR)&errorText,
                  0,
                  NULL);

    std::stringstream errorStream;
    errorStream << ": error code 0x" << std::hex << error << ": " << errorText;

    LocalFree(errorText);

    return message + errorStream.str();
#else
    return message;
#endif
}
