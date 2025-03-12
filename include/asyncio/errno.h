#ifndef ASYNCIO_ERRNO_H
#define ASYNCIO_ERRNO_H

#ifdef _WIN32
    #include <winsock2.h>
    #define sockerrno WSAGetLastError()
#else
    #include <errno.h>
    #define sockerrno errno
#endif

#endif // ASYNCIO_ERRNO_H
