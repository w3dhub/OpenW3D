#ifndef NETWORKTYPEDEFS_H
#define NETWORKTYPEDEFS_H

#ifdef _WIN32
#include <winsock2.h>
#define LastSocketError      (WSAGetLastError())
#else
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h> // for getaddrinfo() and freeaddrinfo()
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // for close()
typedef int SOCKET;
#define INVALID_SOCKET       (-1)
#define SOCKET_ERROR         (-1)
#define closesocket(x)       close(x)
#define ioctlsocket(x, y, z) ioctl(x, y, z)
#define LastSocketError      (errno)
typedef struct in_addr IN_ADDR;

#define WSAEISCONN       EISCONN
#define WSAEINPROGRESS   EINPROGRESS
#define WSAEALREADY      EALREADY
#define WSAEADDRINUSE    EADDRINUSE
#define WSAEADDRNOTAVAIL EADDRNOTAVAIL
#define WSAEACCES        EACCES
#define WSAEAFNOSUPPORT  EAFNOSUPPORT
#define WSAEBADF         EBADF
#define WSAECONNREFUSED  ECONNREFUSED
#define WSAEINTR         EINTR
#define WSAENOBUFS       ENOBUFS
#define WSAENOTSOCK      ENOTSOCK
#define WSAEWOULDBLOCK   EWOULDBLOCK
#define WSAEINVAL        EINVAL
#define WSAETIMEDOUT     ETIMEDOUT
#define WSAECONNRESET    ECONNRESET
#define WSAECONNABORTED  ECONNABORTED
#define WSAENETDOWN      ENETDOWN
#endif

#endif // NETWORKTYPEDEFS_H
