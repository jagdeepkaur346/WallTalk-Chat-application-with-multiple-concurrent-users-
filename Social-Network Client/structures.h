#ifndef STRUCTURES_H_
#define STRUCTURES_H_

#include <string>

enum commands {
	LOGIN,
	LOGOUT,
	POST,
	SHOW,
	LIST,
	NOTIFY,
	ACK
};

/*
 * content - structure to store the packet contents
 * username: to store username
 * password: to store password
 * postee: username of postee
 * post: post contents
 * wallOwner: username of wall owner
 * rcvd_cnts: contents sent by server
 */
struct content {
	std::string username;
	std::string password;
	std::string postee;
	std::string post;
	std::string wallOwner;
	std::string rcvd_cnts;
};

/**
 * packet - structure to store the request & response messages
 * content_len: total packet length
 * command: to store the command portion of the request
 * req_num: request number
 * sessionId: session ID of the client
 * contents: contents of the packet
 */
struct packet {
    unsigned int content_len;
    enum commands cmd_code;
    unsigned int req_num;
    unsigned int sessionId;
    struct content contents;
};

#endif /* STRUCTURES_H_ */
