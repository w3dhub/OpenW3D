#include "socket_wrapper.h"

namespace wwnet {

    int SocketStartup() {
        WSADATA wsa_data;
        int rc = ::WSAStartup(MAKEWORD(2, 2), &wsa_data);
        if (rc != 0) {
            return rc;
        }
        if (wsa_data.wVersion != MAKEWORD(2, 2)) {
            ::WSACleanup();
            return WSAVERNOTSUPPORTED;
        }
        return 0;
    }

    void SocketCleanup() {
        ::WSACleanup();
    }

    SocketHandle SocketCreate(int domain, int type, int protocol) {
        return ::socket(domain, type, protocol);
    }

    int SocketClose(SocketHandle s) {
        return ::closesocket(s);
    }

    int SocketShutdown(SocketHandle s, int how) {
        return ::shutdown(s, how);
    }

    int SocketGetLastError() {
        return ::WSAGetLastError();
    }

    void SocketSetLastError(int err) {
        ::WSASetLastError(err);
    }

    int SocketIoctl(SocketHandle s, long cmd, SocketIoctlParam* argp) {
        return ::ioctlsocket(s, cmd, argp);
    }

    int SocketGetSockOpt(SocketHandle s, int level, int optname, char* optval, socklen_t* optlen) {
        int len = optlen ? static_cast<int>(*optlen) : 0;
        int rc = ::getsockopt(s, level, optname, optval, optlen ? &len : nullptr);
        if (optlen) {
            *optlen = static_cast<socklen_t>(len);
        }
        return rc;
    }

    int SocketSetSockOpt(SocketHandle s, int level, int optname, const char* optval, socklen_t optlen) {
        return ::setsockopt(s, level, optname, optval, static_cast<int>(optlen));
    }

    int SocketSendTo(SocketHandle s, const char* buf, size_t len, int flags, const struct sockaddr* to, socklen_t* tolen) {
        int tlen = tolen ? static_cast<int>(*tolen) : 0;
        return ::sendto(s, buf, static_cast<int>(len), flags, to, tlen);
    }

    int SocketRecvFrom(SocketHandle s, char* buf, size_t len, int flags, struct sockaddr* from, socklen_t* fromlen) {
        int flen = fromlen ? static_cast<int>(*fromlen) : 0;
        int rc = ::recvfrom(s, buf, static_cast<int>(len), flags, from, fromlen ? &flen : nullptr);
        if (fromlen) {
            *fromlen = static_cast<socklen_t>(flen);
        }
        return rc;
    }

    int SocketGetHostName(char* name, int namelen) {
        return ::gethostname(name, namelen);
    }

    struct hostent* SocketGetHostByName(const char* name) {
        return ::gethostbyname(name);
    }

} // namespace wwne
