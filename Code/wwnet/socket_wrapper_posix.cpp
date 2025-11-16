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

    int SocketIoctl(SocketHandle s, int cmd, SocketIoctlParam* argp) {
        return ::ioctl(s, cmd, argp);
    }

    int SocketGetSockOpt(SocketHandle s, int level, int optname, char* optval, socklen_t* optlen) {
        return ::getsockopt(s, level, optname, optval, optlen);
    }

    int SocketSetSockOpt(SocketHandle s, int level, int optname, const char* optval, socklen_t optlen) {
        return ::setsockopt(s, level, optname, optval, optlen);
    }

    int SocketSendTo(SocketHandle s, const char* buf, size_t len, int flags, const struct sockaddr* to, socklen_t* tolen) {
        return ::sendto(s, buf, len, flags, to, tolen ? *tolen : 0);
    }

    int SocketRecvFrom(SocketHandle s, char* buf, size_t len, int flags, struct sockaddr* from, socklen_t* fromlen) {
        return ::recvfrom(s, buf, len, flags, from, fromlen);
    }

    int SocketGetHostName(char* name, int namelen) {
        return ::gethostname(name, namelen);
    }

    struct hostent* SocketGetHostByName(const char* name) {
        return ::gethostbyname(name);
    }

} // namespace wwnet
