#ifndef NETWORKTYPEDEFS_H
#define NETWORKTYPEDEFS_H

#ifdef _WINDOWS
#include <winsock.h>
#else
#include <arpa/inet.h>
typedef struct sockaddr_in SOCKADDR_IN;
typedef int SOCKET;
typedef SOCKADDR_IN* LPSOCKADDR_IN;
#endif

#endif // NETWORKTYPEDEFS_H
