#ifndef hybrid_socket
    #define hybrid_socket
    #include <stdlib.h>
    #include "OV7670.h"
    #ifdef _WIN32
        #include <WinSock2.h>
        #include <direct.h> // alternativa unix unistd.h
        typedef SOCKET HYBRID_SOCKET;
        typedef int BYTES_NUM;
    #else
        #include <unistd.h>
        typedef int HYBRID_SOCKET;
        typedef ssize_t BYTES_NUM;
    #endif

    int close_socket_wrapper(HYBRID_SOCKET);
    HYBRID_SOCKET init_socket_wrapper(long, struct sockaddr_in*, struct sockaddr_in*);
    BYTES_NUM recvfrom_socket_wrapper(HYBRID_SOCKET, void*, int, int, struct sockaddr*, int*); // TODO: in unix il ritorno è ssize_t

#endif // hybrid_socket
