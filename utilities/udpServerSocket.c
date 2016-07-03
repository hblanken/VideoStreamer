#include "udpCLientSocket.h"
#include <pthread.h>

#define MAX_RECV_BUF_LEN (1024 * 1024)

typedef struct UDP_SOCKET_T {
	uint16_t port;
	int32_t fd;
	struct sockaddr_in serveraddr;
	pthread_t   threadId;
	int32_t isRunning;
}UDP_SOCKET_T;

void *serverThread(void *data);

void *initUdpServerSocket(uint16_t port, char *hostname) {
	int32_t err = 0;
	int32_t optval;
	UDP_SOCKET_T *udpSocketData = NULL;

	assert(hostname);
	udpSocketData = (UDP_SOCKET_T*)malloc(sizeof(UDP_SOCKET_T));
	if (NULL == udpSocketData) {
		printf("ERROR malloc error\n");
		return NULL;
	}
	memset(udpSocketData, 0, sizeof(UDP_SOCKET_T));
	udpSocketData->port = port;

	udpSocketData->fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (udpSocketData->fd < 0) {
		printf("ERROR opening socket\n");
		free(udpSocketData);
		return NULL;
	}

	optval = 1;//enable
	setsockopt(udpSocketData->fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(optval));

	memset(udpSocketData->serveraddr, 0, sizeof(udpSocketData->serveraddr));
	udpSocketData->serveraddr->sin_family = AF_INET;
	udpSocketData->serveraddr->sin_port = htons(port);
	udpSocketData->serveraddr->sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(udpSocketData->fd, (struct sockaddr *)&udpSocketData->serveraddr, sizeof(udpSocketData->serveraddr)) < 0) {
		printf("ERROR on binding");
		(udpSocketData);
		return NULL;
	}

	err = pthread_create(&udpSocketData->threadId, NULL, serverThread, (void *)udpSocketData);
	if (err) {
		printf("ERROR thread creation failed\n");
		close(udpSocketData->fd);
		free(udpSocketData);
	}
	return udpSocketData;
}

void *serverThread(void *args) {
	UDP_SOCKET_T *udpSocketData = args;
	int32_t nBytes = 0;
	int32_t addrSize = 0;
	unsigned char buffer[MAX_RECV_BUF_LEN];
	struct sockaddr_in clientaddr; /* client addr */
	struct hostent *hostp;
	char *hostaddrp;

	addrSize = sizeof(clientaddr);
	udpSocketData->isRunning = 1;
	while (udpSocketData->isRunning) {
		nBytes = recvfrom(udpSocketData->fd, buffer, MAX_RECV_BUF_LEN, 0, &clientaddr, &addrSize);
		if (nBytes < 0) {
			printf("ERROR in recvfrom\n");
		}
		hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);

		if (NULL == hostp) {
			printf("ERROR on gethostbyaddr\n");
		}
			
		hostaddrp = inet_ntoa(clientaddr.sin_addr);
		if (hostaddrp == NULL) {
			printf("ERROR on inet_ntoa\n");
		}	
		printf("server received datagram from %s (%s)\n", hostp->h_name, hostaddrp);
	}
	pthread_exit(NULL);
}

int32_t sendUdpDataToClient(void *handle, void *client, char *buffer, int32_t bufLen) {
	int32_t addrSize = 0;
	int32_t nBytes = 0;
	int32_t addrSize = 0;
	UDP_SOCKET_T *udpSocketData = handle;
	struct sockaddr_in *clientaddr = client;

	if (NULL == buffer) {
		return 0;
	}

	addrSize = sizeof(struct sockaddr_in);
	nBytes = sendto(udpSocketData->fd, buffer, bufLen, 0, clientaddr, addrSize);
	if (nBytes < 0) {
		error("ERROR in sendto");
	}
	return nBytes;
}

int32_t isUdpServerRunning(void *handle) {
	if (NULL == handle) {
		return 0;
	}
	UDP_SOCKET_T *udpSocketData = handle;
	return udpSocketData->isRunning;
}

int32_t closeUdpServer(void *handle) {
	if (NULL == handle) {
		return 0;
	}
	UDP_SOCKET_T *udpSocketData = handle;
	if (udpSocketData->isRunning) {
		udpSocketData->isRunning = 0;
		pthread_join(udpSocketData->threadId, NULL);
		close(udpSocketData->fd);
		free(udpSocketData);
	}
	return 1;
}

