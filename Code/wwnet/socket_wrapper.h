#pragma once

#include <cstddef>
#include <cstdint>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
namespace wwnet {
	using SocketHandle = SOCKET;
	constexpr SocketHandle INVALID_SOCKET_VALUE = INVALID_SOCKET;
	constexpr int SOCKET_ERROR_VALUE = SOCKET_ERROR;
	using SocketIoctlParam = u_long;
}
#else
#include <arpa/inet.h>
#include <cerrno>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef SOCKET
#define SOCKET int
#endif
#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif
#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif
#ifndef closesocket
#define closesocket(x) ::close(x)
#endif
#ifndef ioctlsocket
#define ioctlsocket(x, y, z) ::ioctl(x, y, z)
#endif
#ifndef LastSocketError
#define LastSocketError (errno)
#endif
#ifndef WSAEISCONN
#define WSAEISCONN EISCONN
#endif
#ifndef WSAEINPROGRESS
#define WSAEINPROGRESS EINPROGRESS
#endif
#ifndef WSAEALREADY
#define WSAEALREADY EALREADY
#endif
#ifndef WSAEADDRINUSE
#define WSAEADDRINUSE EADDRINUSE
#endif
#ifndef WSAEADDRNOTAVAIL
#define WSAEADDRNOTAVAIL EADDRNOTAVAIL
#endif
#ifndef WSAEAFNOSUPPORT
#define WSAEAFNOSUPPORT EAFNOSUPPORT
#endif
#ifndef WSAEBADF
#define WSAEBADF EBADF
#endif
#ifndef WSAECONNREFUSED
#define WSAECONNREFUSED ECONNREFUSED
#endif
#ifndef WSAEINTR
#define WSAEINTR EINTR
#endif
#ifndef WSAEACCES
#define WSAEACCES EACCES
#endif
#ifndef WSAENOBUFS
#define WSAENOBUFS ENOBUFS
#endif
#ifndef WSAENOTSOCK
#define WSAENOTSOCK ENOTSOCK
#endif
#ifndef WSAEWOULDBLOCK
#define WSAEWOULDBLOCK EWOULDBLOCK
#endif
#ifndef WSAEINVAL
#define WSAEINVAL EINVAL
#endif
#ifndef WSAETIMEDOUT
#define WSAETIMEDOUT ETIMEDOUT
#endif

namespace wwnet {
	using SocketHandle = int;
	constexpr SocketHandle INVALID_SOCKET_VALUE = -1;
	constexpr int SOCKET_ERROR_VALUE = -1;
	using SocketIoctlParam = int;
}
#endif

namespace wwnet {
	int SocketStartup();
	void SocketCleanup();
	SocketHandle SocketCreate(int domain, int type, int protocol);
	int SocketClose(SocketHandle s);
	int SocketShutdown(SocketHandle s, int how);
	int SocketGetLastError();
	void SocketSetLastError(int err);
	int SocketIoctl(SocketHandle s, long cmd, SocketIoctlParam* argp);
	int SocketGetSockOpt(SocketHandle s, int level, int optname, char* optval, socklen_t* optlen);
	int SocketSetSockOpt(SocketHandle s, int level, int optname, const char* optval, socklen_t optlen);
	int SocketSendTo(SocketHandle s, const char* buf, size_t len, int flags, const struct sockaddr* to, socklen_t* tolen);
	int SocketRecvFrom(SocketHandle s, char* buf, size_t len, int flags, struct sockaddr* from, socklen_t* fromlen);
	int SocketGetHostName(char* name, int namelen);
	struct hostent* SocketGetHostByName(const char* name);
}
