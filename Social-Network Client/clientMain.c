#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <tr1/functional>
#include <string.h>
#include <string>
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

using namespace std;

string username;
unsigned int sessionID;

void getLoginInfo(string &pw);
int enterLoginMode(string servername, int serverport);
int enterWebBrowserMode(string servername, int serverport);


int main(int argc, char *argv[])
{
	string servername = "localhost";
	int serverport = 5354;
	int ret = 0;
	switch (argc)
	{
	case 1:
			break;
	case 2:
			servername = argv[1];
			break;
	case 3:
			servername = argv[1];
			serverport = stoi(argv[2]);
			break;
	default:
			printf("Error: Usage is ./[executable] [hostname] [port]\n");
			return -1;
	}
	ret = enterLoginMode(servername, serverport);
	if (ret < 0)
	{
		printf("Error (processClient): Client processing failed\n");
		return -1;
	}
	return 0;
}

/**
 * enterLoginMode() - accept user request to exit or login
 * servername: ip of endpoint
 * serverport: port number of server
 * return 0 or -1
 **/
int enterLoginMode(string servername, int serverport)
{
    int option = 0;
    string input;
    while (1)
    {
   	    cout<<"Enter an Option (1 or 2)"<<endl;
        cout<<"1. Login\n2. Exit"<<endl;
        getline(std::cin, input);
        if (!input.length())
        	continue;
        option = atoi(input.c_str());
        if (option == 1)
        {
            enterWebBrowserMode(servername, serverport);
            break;
        }
        else if (option == 2)
        {
            cout<<"Gracefully exiting"<<endl;
            break;
        }
        else
        {
            cout<<"Invalid option. Try again!!"<<endl;
        }
    }
    return 0;
}


int enterWebBrowserMode(string servername, int serverport)
{
    int  sock_fd;
    struct packet *resp = (struct packet *)malloc(sizeof(struct packet));
    char *buffer = (char *)resp;
    int resp_len;
    string pw;
    int ret;
    sock_fd = create_client_socket(servername, serverport);
    if (sock_fd < 0)
    {
    	printf("Error (create_client_socket): Client Socket creation failed\n");
    	return -1;
    }

    /*Getting username and password from stdin*/
    getLoginInfo(pw);

    /*Creating and sending packet*/
    sendPacket(sock_fd, LOGIN, username, pw);

    /* create 2 threads to handle read and write */
    pthread_t th1, th2;
    pthread_attr_t ta;
    pthread_attr_init(&ta);
    pthread_attr_setdetachstate(&ta, PTHREAD_CREATE_DETACHED);
    ret = pthread_create(&th1, &ta, (void * (*) (void *)) readThread, (void *)((long) sock_fd));
    if (ret < 0)
    {
        perror("\nRead Thread create error");
        return -1;
    }
    ret = pthread_create(&th2, &ta, (void * (*) (void *)) writeThread, (void *)((long) sock_fd));
    if (ret < 0)
    {
        perror("\nWrite Thread create error");
        return -1;
    }
    while (1) {}
    free(resp);
    free(buffer);
    return 0;
}

void getLoginInfo(string &pw)
{
	std::tr1::hash<string> hashfun;
	string password;

    cout<<"Enter User Name:";
    cin>>username;
    password = getpass("Enter Password:");
    cout<<endl;
    pw = to_string(hashfun(password));
    return;
}

