#pragma once

#include <cstdint>
#include <cstddef>

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
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
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
	int SocketGetSockOpt(SocketHandle s, int level, int optname, char* optval, int* optlen);
	int SocketSetSockOpt(SocketHandle s, int level, int optname, const char* optval, int optlen);
	int SocketSendTo(SocketHandle s, const char* buf, size_t len, int flags, const struct sockaddr* to, socklen_t* tolen);
	int SocketRecvFrom(SocketHandle s, char* buf, size_t len, int flags, struct sockaddr* from, socklen_t* fromlen);
	int SocketGetHostName(char* name, int namelen);
	struct hostent* SocketGetHostByName(const char* name);
}