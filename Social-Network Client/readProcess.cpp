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
#include "structures.h"
#include "func_lib.h"
#include "networking.h"

extern string username;
extern unsigned int sessionID;

using namespace std;

void readThread(int sock_fd);
int sendPacket(int sock_fd, enum commands cmd_code, string key, string value);
void printCmdList();
void list(int sock_fd);
void post(int sock_fd);
void show(int sock_fd);
void logout(int sock_fd);
void createLoginPacket(string username, string pw, struct packet &pkt);
void createPostPacket(string postee, string post, struct packet &pkt);
void createShowPacket(string wallOwner, struct packet &pkt);


/*
 * readThread() - thread to handle user input from stdin
 * sock_fd: socket file descriptor
 */
void readThread(int sock_fd)
{
    /*Print the list of commands*/
    int cmd_entered;
    string input;

    printCmdList();
    cin.ignore();
    while(1)
    {
    	getline(std::cin, input);
        if (strcmp(input.c_str(), "0") == 0)
        {
            printCmdList();
        }
        else if (strcmp(input.c_str(), "1") == 0)
        {
            list(sock_fd);
        }
        else if (strcmp(input.c_str(), "2") == 0)
        {
            post(sock_fd);
        }
        else if (strcmp(input.c_str(), "3") == 0)
        {
        	show(sock_fd);
        }
        else if (strcmp(input.c_str(), "4") == 0)
        {
        	logout(sock_fd);
        }
        else
        {
            cout<<"Invalid Option. Try again !!!\n";
        }
    }
    pthread_exit(NULL);
}

/*
 * printCmdList() - print the default command list
 */
void printCmdList()
{
    cout<<"Commands (Enter 0- 4)\n"<<"--------------\n";
    cout<<"1. List all users\n";
    cout<<"2. Post to wall\n";
    cout<<"3. Show wall\n";
    cout<<"4. Logout\n";
    cout<<"[Enter 0 to print the command list]\n\n";
    return;
}

/*
 * list() - send a request to list all users
 * sock_fd: socket file descriptor
 */
void list(int sock_fd)
{
    sendPacket(sock_fd, LIST, "", "");
    return;
}

/*
 * post() - send a request to post to a wall
 * sock_fd: socket file descriptor
 */
void post(int sock_fd)
{
    int postWall;
    string name;
    string post;
    string input;

    cout<<"1. Own wall\n";
    cout<<"2. Others wall\n";
    getline(std::cin, input);
    postWall = atoi(input.c_str());
	if (postWall == 1)
	{
		name = username;
	}
	else if (postWall == 2)
	{
		cout<<"Whose wall: ";
		getline(std::cin, name);
	}
	else
	{
		cout<<"Invalid Option\n";
		return;
	}
	cout<<"Enter Post Msg: ";
	getline(std::cin, post);
	sendPacket(sock_fd, POST, name, post);
    return;
}

/*
 * show() - send a request to show a wall
 * sock_fd: socket file descriptor
 */
void show(int sock_fd)
{
    int showWall;
    string name;
    string input;

    cout<<"1. Own wall\n";
    cout<<"2. Others wall\n";
    getline(std::cin, input);
    showWall = atoi(input.c_str());
	if (showWall == 1)
	{
		name = username;
	}
	else if (showWall == 2)
	{
		cout<<"Whose wall: ";
		getline(std::cin, name);
	}
	else
	{
		cout<<"Invalid Option\n";
		return;
	}
	sendPacket(sock_fd, SHOW, name, "");
    return;
}

/*
 * logout() - send a request to logout
 * sock_fd: socket file descriptor
 */
void logout(int sock_fd)
{
	sendPacket(sock_fd, LOGOUT, "", "");
}

/*
 * sendPacket() - send a request packet to server
 * sock_fd: socket file descriptor
 * cmd_code: command code
 * value1 & value2: depending on the command
 * return 0 (success) -1(error)
 */
int sendPacket(int sock_fd, enum commands cmd_code, string value1, string value2)
{
    struct packet req;
    int send_bytes;

    req.cmd_code = cmd_code;
    req.sessionId = sessionID;
    switch(cmd_code)
    {
    case LOGIN:
    	createLoginPacket(value1, value2, req);
    	break;
    case LOGOUT:
    	break;
    case LIST:
    	break;
    case POST:
    	createPostPacket(value1, value2, req);
    	break;
    case SHOW:
    	createShowPacket(value1, req);
    	break;
    default:
    	printf("Invalid Command Code\n");
    	return -1;
    }
    send_bytes = write_socket(sock_fd, req);
    //send_bytes = write(sock_fd, &req, sizeof(req));
    if (send_bytes < 0)
    {
    	printf("Error (write_socket)\n");
        return -1;
    }
    return 0;
}

/*
 * createLoginPacket() - create login packet
 * username: username of the user
 * pw: password of the user
 * pkt: request packet where the username and password are stored
 */
void createLoginPacket(string username, string pw, struct packet &pkt)
{
	pkt.contents.username = username;
	pkt.contents.password = pw;
}

/*
 * createPostPacket() - create post packet
 * postee: username of postee
 * post: message to post
 * pkt: request packet where the details are stored
 */
void createPostPacket(string postee, string post, struct packet &pkt)
{
	pkt.contents.postee = postee;
	pkt.contents.post = post;
}

/*
 * createShowPacket() - create show packet
 * wallOwner: username of wall owner
 * pkt: request packet where the details are stored
 */
void createShowPacket(string wallOwner, struct packet &pkt)
{
	pkt.contents.wallOwner = wallOwner;
}


