#include "socket_wrapper.h"

namespace wwnet {

    int SocketStartup() {
        return 0;
    }

    void SocketCleanup() {
        // no-op on POSIX
    }

    SocketHandle SocketCreate(int domain, int type, int protocol) {
        return ::socket(domain, type, protocol);
    }

    int SocketClose(SocketHandle s) {
        return ::close(s);
    }

    int SocketShutdown(SocketHandle s, int how) {
        return ::shutdown(s, how);
    }

    int SocketGetLastError() {
        return errno;
    }

    void SocketSetLastError(int err) {
        errno = err;
    }

    int SocketIoctl(SocketHandle s, long cmd, SocketIoctlParam* argp) {
        return ::ioctl(s, cmd, argp);
    }

    int SocketGetSockOpt(SocketHandle s, int level, int optname, char* optval, int* optlen) {
        socklen_t len = static_cast<socklen_t>(*optlen);
        int rc = ::getsockopt(s, level, optname, optval, &len);
        *optlen = static_cast<int>(len);
        return rc;
    }

    int SocketSetSockOpt(SocketHandle s, int level, int optname, const char* optval, int optlen) {
        return ::setsockopt(s, level, optname, optval, optlen);
    }

    int SocketSendTo(SocketHandle s, const char* buf, int len, int flags, const struct sockaddr* to, int tolen) {
        return ::sendto(s, buf, len, flags, to, tolen);
    }

    int SocketRecvFrom(SocketHandle s, char* buf, int len, int flags, struct sockaddr* from, int* fromlen) {
        socklen_t flen = fromlen ? static_cast<socklen_t>(*fromlen) : 0;
        int rc = ::recvfrom(s, buf, len, flags, from, fromlen ? &flen : nullptr);
        if (fromlen) {
            *fromlen = static_cast<int>(flen);
        }
        return rc;
    }

    int SocketGetHostName(char* name, int namelen) {
        return ::gethostname(name, namelen);
    }

    struct hostent* SocketGetHostByName(const char* name) {
        return ::gethostbyname(name);
    }

} // namespace wwnet
