#include <time.h>
#include "func_lib.h"
#include "mysql_lib.h"
#include "structures.h"

extern DatabaseCommandInterface database;
extern unsigned int sessionID;
using namespace std;

/*
 * handleClient() - handle each client connection
 * sock_fd: slave socket file descriptor
 */
void handleClient(int sock_fd)
{
	int sock_read, ret;


	/* Accept the request persistently*/
	while(1)
	{
		struct packet req;

		/* Read client request */
		sock_read = read_socket(sock_fd, req);
		if(sock_read < 0)
		{
			if (sock_read == -9)
				continue;
			printf("Error(read_socket)\n");
			break;
		}
		if (!sock_read ) /*Client connection EOF */
		{
			req.sessionId = sessionID;
			ret = database.logout(req);
			if (ret < 0)
			{
				printf("Error (logout): User logging out from database failed\n");
				return;
			}
			break;
		}

		/* Parse the packet for valid packet structure */
		ret = parsePacket(&req);
		if (ret < 0)
		{
			printf("Error (parsePacket): Packet parsing/checking failed\n");
			break;
		}

		/* Validate session of the client */
		ret = sessionValidity(&req);
		if (ret < 0)
		{
			sendPacket(sock_fd, req);
			if (ret == -2)
				printf("Error (sessionValidity): DB could not process session validity\nClosing Client Connection\n");
			break;
		}

		/* process the request */
		ret = processRequest(sock_fd, req);
		if (ret < 0)
			break;
	}
	destroy_socket(sock_fd);
	pthread_exit(NULL);
	return;
}

/*
 * parsePacket() - parse the packet and validate all fields
 * req: request structure
 * return 0(Valid Packet) -1(Invalid Packet)
 */
int parsePacket(struct packet *req)
{
	switch (req->cmd_code)
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
	case ACK:
		break;
	case NOTIFY:
		break;
	default:
		printf("Invalid command, code = %d\n", req->cmd_code);
		return -1;
	}
	return 0;
}

/*
 * sessionValidity() - validate the client session
 * req: request structure
 * return 0(Valid session) -1(Invalid session)
 */
int sessionValidity(struct packet *req)
{
	int ret = 0;
	if (req->cmd_code != LOGIN)
		ret = database.hasValidSession(*req);
	return ret;
}

