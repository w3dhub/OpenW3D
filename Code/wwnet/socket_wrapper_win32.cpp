#include "socket_wrapper.h"

namespace wwnet {

    int SocketStartup() {
        WSADATA wsa_data;
        return ::WSAStartup(MAKEWORD(1, 1), &wsa_data);
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

    int SocketGetSockOpt(SocketHandle s, int level, int optname, char* optval, int* optlen) {
        return ::getsockopt(s, level, optname, optval, optlen);
    }

    int SocketSetSockOpt(SocketHandle s, int level, int optname, const char* optval, int optlen) {
        return ::setsockopt(s, level, optname, optval, optlen);
    }

    int SocketSendTo(SocketHandle s, const char* buf, int len, int flags, const struct sockaddr* to, int tolen) {
        return ::sendto(s, buf, len, flags, to, tolen);
    }

    int SocketRecvFrom(SocketHandle s, char* buf, int len, int flags, struct sockaddr* from, int* fromlen) {
        return ::recvfrom(s, buf, len, flags, from, fromlen);
    }

    int SocketGetHostName(char* name, int namelen) {
        return ::gethostname(name, namelen);
    }

    struct hostent* SocketGetHostByName(const char* name) {
        return ::gethostbyname(name);
    }

} // namespace wwne