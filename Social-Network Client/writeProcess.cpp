#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <netdb.h>
#include <unordered_map>
#include <iostream>
#include <pthread.h>
#include <time.h>
#include "structures.h"
#include "func_lib.h"
#include "networking.h"

extern const char * getCommand(int enumVal);
extern unsigned int sessionID;

using namespace std;

void writeThread(int sock_fd);
int parsePacket(struct packet *req);
void displayContents(struct packet *resp);
int processResponse(int sock_fd, struct packet *resp);

/*
 * writeThread() - thread to write to stdout
 * sock_fd: socket file descriptor
 */
void writeThread(int sock_fd)
{
	int sock_read, sock_write, sock_close;
	struct packet resp;
	int resp_len, ret;
	/* Accept the response persistently*/
	while(1)
	{
		memset(&resp, 0, sizeof(struct packet));
		/* Read server response */
		sock_read = read_socket(sock_fd, resp);
		if(sock_read < 0)
		{
			printf("Error(read_socket)\n");
			return;
		}
		if(!sock_read)
			break;
		/* Parse the packet for valid packet structure */
		ret = parsePacket(&resp);
		if (ret < 0)
		{
			printf("Error (parsePacket): Packet parsing/checking failed\n");
			return;
		}
		/* process the response  */
		ret = processResponse(sock_fd, &resp);
		if (ret < 0)
		{
			printf("Error (processRequest): request processing failed\n");
			return;
		}
	}
	printf("Connection closed\nExiting Application\n");
	destroy_socket(sock_fd);
	exit(0);
}


/*
 * parsePacket() - parse the packet and validate all fields
 * req: request structure
 * return 0(Valid Packet) -1(Invalid Packet)
 */
int parsePacket(struct packet *resp)
{

	switch (resp->cmd_code)
	{
	case LOGIN:
		break;
	case LOGOUT:
		break;
	case POST:
		break;
	case LIST:
		break;
	case SHOW:
		break;
	case NOTIFY:
		break;
	case ACK:
		break;
	default:
		printf("Invalid command, code = %d\n", resp->cmd_code);
		return -1;
	}
	return 0;
}

/*
 * processResponse() - process the response from the server
 * resp: response structure
 * return 0(request processed successfully) -1(request processing failed)
 */
int processResponse(int sock_fd, struct packet *resp)
{

	if(resp->cmd_code == LIST || resp->cmd_code == SHOW || resp->cmd_code == POST || resp->cmd_code == NOTIFY || resp->cmd_code == LOGOUT)
		displayContents(resp);
	else if (resp->cmd_code == LOGIN)
		if (!resp->contents.rcvd_cnts.length())
		{
			sessionID = resp->sessionId;
		}
		else
		{
			displayContents(resp);
			destroy_socket(sock_fd);
			printf("Closing Connection\n");
			exit(0);
		}
	else
	{
		printf("Error (processResponse):Invalid Option\n");
		return -1;
	}
	return 0;
}

/*
 * displayContents() - display the contents of the response packet
 * resp: response from server
 */
void displayContents(struct packet *resp)
{
	cout<<resp->contents.rcvd_cnts<<endl;
	return;
}
