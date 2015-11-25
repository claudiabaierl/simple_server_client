#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[])
{

	struct addrinfo client_info;
	struct addrinfo *set_info, *rp;
	size_t length;


	memset(&client_info, 0, sizeof(struct addrinfo));
	client_info.ai_family = AF_UNSPEC;
	client_info.ai_socktype = SOCK_STREAM;
	client_info.ai_protocol = 0;







}
