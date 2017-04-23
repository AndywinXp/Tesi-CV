#include "hybrid_socket.h"

void close_socket_wrapper(HYBRID_SOCKET sd)
{
    #ifdef _WIN32
    closesocket((SOCKET) sd); // SPECIFICO PER WINDOWS, funziona solo su socket
                     // e non su file descriptor come la close(fd) di unix
    WSACleanup();
    #endif // _WIN32
}

HYBRID_SOCKET init_socket_wrapper(long buffer, struct sockaddr_in *clientaddr, struct sockaddr_in *servaddr)
{
    #ifdef _WIN32

    WSADATA wsa;
	//Initialise winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	/* PREPARAZIONE INDIRIZZO CLIENT E SERVER ----------------------------- */
	memset((char *)clientaddr, 0, sizeof(struct sockaddr_in));
	(*clientaddr).sin_family = AF_INET;
	(*clientaddr).sin_addr.s_addr = INADDR_ANY;
	(*clientaddr).sin_port = htons(5555);

	printf("Client avviato\n");

	/* CREAZIONE SOCKET ---------------------------- */
	SOCKET sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sd == INVALID_SOCKET) { perror("apertura socket"); exit(3); }

	printf("Creata la socket sd=%d\n", (int) sd);

	/* BIND SOCKET, a una porta scelta dal sistema --------------- */
	if (bind(sd, (struct sockaddr *) clientaddr, sizeof(*clientaddr)) == SOCKET_ERROR) //dubbio puntatore
	{
		perror("bind socket ");
		exit(1);
	}
	printf("Client: bind socket ok, alla porta %i\n", ntohs((*clientaddr).sin_port));

	if (setsockopt(sd, SOL_SOCKET, SO_RCVBUF, (char*)&buffer, 8) == -1) {
		fprintf(stderr, "Error setting socket opts: %d\n", WSAGetLastError());
	}
    else printf("Settato buffer\n");
    #endif // _WIN32

    return (HYBRID_SOCKET) sd;
}

int recvfrom_socket_wrapper(HYBRID_SOCKET s, void *buf, int len, int flags, struct sockaddr *from, int *fromlen)
{
    #ifdef _WIN32
        return recvfrom((SOCKET) s, (char*)buf, len, flags, (struct sockaddr *)&from, fromlen);
    #else
        return 1;
        // return ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
    #endif // _WIN32
}
