#ifndef FUNC_LIB_H_
#define FUNC_LIB_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
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

using namespace std;

void getLoginInfo(string &pw);
int enterLoginMode(string servername, int serverport);
int enterWebBrowserMode(string servername, int serverport);


void readThread(int sock_fd);
int sendPacket(int sock_fd, enum commands cmd_code, string key, string value);
void list(int sock_fd);
void printCmdList();
void post(int sock_fd);
void show(int sock_fd);
void logout(int sock_fd);
void createLoginPacket(string username, string pw, struct packet &pkt);
void createPostPacket(string postee, string post, struct packet &pkt);
void createShowPacket(string wallOwner, struct packet &pkt);

void writeThread(int sock_fd);
int parsePacket(struct packet *req);
void displayContents(struct packet *resp);
int processResponse(int sock_fd, struct packet *resp);


#endif /* FUNC_LIB_H_ */
