
int32_t startNetwork(const char *droneIpAdd, uint16_t port, int *droneCommandfd);
int32_t startHandshake(int sockfd, const char *command, char *responseBuf, ssize_t responseBufLen);
