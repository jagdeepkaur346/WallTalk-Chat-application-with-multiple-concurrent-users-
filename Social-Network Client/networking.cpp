#include "networking.h"

unsigned int packetSeqNum = 0;
bool isServer = false;
int bufferOccupied = 0;
struct packet bufferPkts[10];

pthread_mutex_t seqNumlock;
pthread_mutex_t bufferPktlock;
pthread_mutex_t logFilelock;

const char * getCommand(int enumVal)
{
  return commandList[enumVal];
}

int create_server_socket(int portNum) {
	isServer = true;
	int socketfd = socket(AF_INET, SOCK_STREAM, 0);

	if(socketfd < 0) {
		char errorMessage[ERR_LEN];
		fprintf(stderr, "Failed to create Listening Socket; Error Message: %s\n", strerror_r(errno, errorMessage, ERR_LEN));
		return -1;
	}

	//set reuse addr and port
	int enable = 1;
	if(setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
		char errorMessage[ERR_LEN];
		fprintf(stderr, "setsockopt(SO_REUSEADDR) failed; Error Message: %s\n", strerror_r(errno, errorMessage, ERR_LEN));
		return -4;
	}
	if(setsockopt(socketfd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0) {
		char errorMessage[ERR_LEN];
		fprintf(stderr, "setsockopt(SO_REUSEPORT) failed; Error Message: %s\n", strerror_r(errno, errorMessage, ERR_LEN));
		return -4;
	}

	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr)); //Clear structure
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(portNum);

	if(bind(socketfd,(struct sockaddr *)&serverAddr , sizeof(serverAddr)) < 0) {
		char errorMessage[ERR_LEN];
		fprintf(stderr, "Failed to Bind; Error Message: %s\n", strerror_r(errno, errorMessage, ERR_LEN));
		return -2;
	}

	if(listen(socketfd, LISTEN_QUEUE_LENGTH) < 0) {
		char errorMessage[ERR_LEN];
		fprintf(stderr, "Failed to Listen; Error Message: %s\n", strerror_r(errno, errorMessage, ERR_LEN));
		return -3;
	}

	return socketfd;
}

int create_client_socket(string serverName, int portNum) {
	struct addrinfo *serverInfo;
	struct addrinfo hints;
	string portString = to_string(portNum);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
	hints.ai_flags = 0;

	int error;
	error = getaddrinfo(serverName.c_str(), portString.c_str(), NULL, &serverInfo);
	if(error != 0) {
		fprintf(stderr, "Failed to find addrinfo for the URL: %s, error: %s\n", serverName.c_str(), gai_strerror(error));
		return -1;
	}

	struct addrinfo *currentInfo;
	int socketfd = 0;
	for(currentInfo = serverInfo; currentInfo != NULL; currentInfo = currentInfo->ai_next) {
		//create socket
		socketfd = socket(currentInfo->ai_family, currentInfo->ai_socktype, currentInfo->ai_protocol);
		if(socketfd < 0) {
			char errorMessage[ERR_LEN];
			fprintf(stderr, "Failed to Create Socket; Error Message: %s\n", strerror_r(errno, errorMessage, ERR_LEN));
			return -2;
		}
		//connect socket
		if(connect(socketfd, currentInfo->ai_addr, currentInfo->ai_addrlen) > -1) {
			break; //success
		}
		close(socketfd);
		socketfd = -3;		//none success
	}

	//set reuse addr and port
	int enable = 1;
	if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
		char errorMessage[ERR_LEN];
		strerror_r(errno, errorMessage, ERR_LEN);
		fprintf(stderr, "setsockopt(SO_REUSEADDR) failed; Error Message: %s\n", strerror_r(errno, errorMessage, ERR_LEN));
		return -4;
	}
	if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0) {
		char errorMessage[ERR_LEN];
		strerror_r(errno, errorMessage, ERR_LEN);
		fprintf(stderr, "setsockopt(SO_REUSEPORT) failed; Error Message: %s\n", strerror_r(errno, errorMessage, ERR_LEN));
		return -4;
	}

	return socketfd;
}

int accept_socket(int socketfd) {
	struct sockaddr_in clientAddr;
	unsigned int clientSize = sizeof(clientAddr);
	int slaveSocket = accept(socketfd, (struct sockaddr *)&clientAddr, &clientSize);
	if(slaveSocket < 0) {
		char errorMessage[ERR_LEN];
		fprintf(stderr, "Failed to Accept Socket; Error Message: %s\n", strerror_r(errno, errorMessage, ERR_LEN));
		return -1;
	}

	return slaveSocket;
}

int destroy_socket(int socketfd) {
	if(close(socketfd) < 0) {
		char errorMessage[ERR_LEN];
		fprintf(stderr, "Failed to Close Socket; Error Message: %s\n", strerror_r(errno, errorMessage, ERR_LEN));
		return -1;
	}
	return 0;
}

int write_socket_helper(int socketfd, struct packet &pkt) {
	char pktString[MAX_PACKET_LEN];

	strcpy(pktString, "content_len:");	//12, the length of fixed format of packet
	strcat(pktString, to_string(pkt.content_len).c_str());
	strcat(pktString, ",cmd_code:");	//10
	strcat(pktString, to_string(pkt.cmd_code).c_str());
	strcat(pktString, ",req_num:");		//9
	strcat(pktString, to_string(pkt.req_num).c_str());
	strcat(pktString, ",sessionId:");	//11
	strcat(pktString, to_string(pkt.sessionId).c_str());
	strcat(pktString, ",username:");	//10
	strcat(pktString, pkt.contents.username.c_str());
	strcat(pktString, ",password:");	//10
	strcat(pktString, pkt.contents.password.c_str());
	strcat(pktString, ",postee:");		//8
	strcat(pktString, pkt.contents.postee.c_str());
	strcat(pktString, ",post:");		//6
	strcat(pktString, pkt.contents.post.c_str());
	strcat(pktString, ",wallOwner:");	//11
	strcat(pktString, pkt.contents.wallOwner.c_str());
	strcat(pktString, ",rcvd_cnts:");	//11
	strcat(pktString, pkt.contents.rcvd_cnts.c_str());
	//total is 12+10+9+11+10+10+8+6+11+11 = 98

	if(write(socketfd, pktString, strlen(pktString)) < 0) {
		char errorMessage[ERR_LEN];
		fprintf(stderr, "Error (write): %s\n", strerror_r(errno, errorMessage, ERR_LEN));
		return -1;
	}
	return (int)strlen(pktString);
}

int read_socket_helper(int socketfd, struct packet &pkt) {
	int byteRead = 0;
	int totalRead = 0;
	int startIndex = -1;
	int endIndex = -1;
	char errorMessage[ERR_LEN];
	char *buffer = (char *)malloc(sizeof(char) * MAX_PACKET_LEN);
	char *bufferHead = buffer;
	int packetLength = 90;

	// Read each request stream repeatedly
	while (1 == 1)
	{
		byteRead = read(socketfd, buffer, packetLength-totalRead);
		if (byteRead < 0)
		{
			if (errno == EAGAIN)
				return -9;
			fprintf(stderr, "Error (read): %s\n", strerror_r(errno, errorMessage, ERR_LEN));
			free(bufferHead);
			return -2;
		}
		buffer += byteRead;
		totalRead += byteRead;
		//check actual length of the packet
		string temp(bufferHead);
		startIndex = temp.find("content_len:");
		endIndex = temp.find(",cmd_code");
		if(startIndex != -1 && endIndex != -1) {
			string contentLengthString = temp.substr(startIndex + 12, endIndex - startIndex - 12);
			packetLength = 98 + contentLengthString.length() + stoi(contentLengthString);
		} else {
			packetLength = MAX_PACKET_LEN;
		}
		//Break the loop when the request structure is read completely
		if (byteRead == 0 || packetLength <= totalRead)
			break;
	}

	if(bufferHead == NULL || totalRead == 0) {
		free(bufferHead);
		return 0;
	}

	string pktString(bufferHead);
	free(bufferHead);

	startIndex = pktString.find("content_len:");	//first ':'
	endIndex = pktString.find(",cmd_code:");		//first ','
	if(startIndex == -1 || endIndex == -1) {
		fprintf(stderr, "Packet Format Wrong1\n");
		return -1;
	}
	string component = pktString.substr(startIndex + 12, endIndex - startIndex - 12);
	pkt.content_len = (unsigned int) stoi(component);

	startIndex = pktString.find(",cmd_code:", endIndex);	//next component start
	endIndex = pktString.find(",req_num:", startIndex);	//next component end
	if(startIndex == -1 || endIndex == -1) {
		fprintf(stderr, "Packet Format Wrong2\n");
		return -1;
	}
	component = pktString.substr(startIndex + 10, endIndex - startIndex - 10);
	pkt.cmd_code = static_cast<commands>(stoi(component));

	startIndex = pktString.find(",req_num:", endIndex);
	endIndex = pktString.find(",sessionId:", startIndex);
	if(startIndex == -1 || endIndex == -1) {
		fprintf(stderr, "Packet Format Wrong3\n");
		return -1;
	}
	component = pktString.substr(startIndex + 9, endIndex - startIndex - 9);
	pkt.req_num = (unsigned int) stoul(component);

	startIndex = pktString.find(",sessionId:", endIndex);
	endIndex = pktString.find(",username:", startIndex);
	if(startIndex == -1 || endIndex == -1) {
		fprintf(stderr, "Packet Format Wrong4\n");
		return -1;
	}
	component = pktString.substr(startIndex + 11, endIndex - startIndex - 11);
	pkt.sessionId = (unsigned int) stoul(component);

	startIndex = pktString.find(",username:", endIndex);
	endIndex = pktString.find(",password:", startIndex);
	if(startIndex == -1 || endIndex == -1) {
		fprintf(stderr, "Packet Format Wrong5\n");
		return -1;
	}
	component = pktString.substr(startIndex + 10, endIndex - startIndex - 10);
	if (component.length() > 0)
		pkt.contents.username = component;

	startIndex = pktString.find(",password:", endIndex);
	endIndex = pktString.find(",postee:", startIndex);
	if(startIndex == -1 || endIndex == -1) {
		fprintf(stderr, "Packet Format Wrong6\n");
		return -1;
	}
	component = pktString.substr(startIndex + 10, endIndex - startIndex - 10);
	if (component.length() > 0)
		pkt.contents.password = component;

	startIndex = pktString.find(",postee:", endIndex);
	endIndex = pktString.find(",post:", startIndex);
	if(startIndex == -1 || endIndex == -1) {
		fprintf(stderr, "Packet Format Wrong7\n");
		return -1;
	}
	component = pktString.substr(startIndex + 8, endIndex - startIndex - 8);
	if (component.length() > 0)
		pkt.contents.postee = component;

	startIndex = pktString.find(",post:", endIndex);
	endIndex = pktString.find(",wallOwner:", startIndex);
	if(startIndex == -1 || endIndex == -1) {
		fprintf(stderr, "Packet Format Wrong8\n");
		return -1;
	}
	component = pktString.substr(startIndex + 6, endIndex - startIndex - 6);
	if (component.length() > 0)
		pkt.contents.post = component;

	startIndex = pktString.find(",wallOwner:", endIndex);
	endIndex = pktString.find(",rcvd_cnts:", startIndex);
	if(startIndex == -1 || endIndex == -1) {
		fprintf(stderr, "Packet Format Wrong9\n");
		return -1;
	}
	component = pktString.substr(startIndex + 11, endIndex - startIndex - 11);
	if (component.length() > 0)
		pkt.contents.wallOwner = component;

	startIndex = pktString.find(",rcvd_cnts:", endIndex);
	if(startIndex == -1) {
		fprintf(stderr, "Packet Format Wrong10\n");
		return -1;
	}
	component = pktString.substr(startIndex + 11, packetLength - startIndex - 11);
	if (component.length() > 0)
		pkt.contents.rcvd_cnts = component;

	return totalRead;
}

void deepCopyPkt(struct packet &destination, struct packet &source) {
	destination.content_len = source.content_len;
	destination.cmd_code = source.cmd_code;
	destination.req_num = source.req_num;
    destination.sessionId = source.sessionId;
    if(source.contents.username.length() > 0)
    	destination.contents.username = source.contents.username;
	else
		destination.contents.username = string();
    if(source.contents.password.length() > 0)
    	destination.contents.password = source.contents.password;
	else
		destination.contents.password = string();
    if(source.contents.postee.length() > 0)
    	destination.contents.postee = source.contents.postee;
	else
		destination.contents.postee = string();
    if(source.contents.post.length() > 0)
    	destination.contents.post = source.contents.post;
	else
		destination.contents.post = string();
    if(source.contents.wallOwner.length() > 0)
    	destination.contents.wallOwner = source.contents.wallOwner;
	else
		destination.contents.wallOwner = string();
    if(source.contents.rcvd_cnts.length() > 0)
    	destination.contents.rcvd_cnts = source.contents.rcvd_cnts;
	else
		destination.contents.rcvd_cnts = string();
}

int write_socket(int socketfd, struct packet &pkt) {
	int readError = 0;
	bool doTCPRead = true;

	if((isServer && pkt.cmd_code == NOTIFY) || (!isServer && pkt.cmd_code != ACK && pkt.cmd_code != NOTIFY)) {	//if the packet is a new request, assign a req-num to it
		pthread_mutex_lock(&seqNumlock);
		pkt.req_num = packetSeqNum;
		packetSeqNum++;
		pthread_mutex_unlock(&seqNumlock);
	}

	//calculate the correct contentLength
	int contentLength = to_string(pkt.cmd_code).length() + to_string(pkt.req_num).length() + to_string(pkt.sessionId).length() + pkt.contents.username.length() + pkt.contents.password.length() + pkt.contents.postee.length() + pkt.contents.post.length() + pkt.contents.wallOwner.length() + pkt.contents.rcvd_cnts.length();
	pkt.content_len = (unsigned int) contentLength;

	int writeError = write_socket_helper(socketfd, pkt);
	if(writeError < 0)
		return writeError;

	auto sendTime = chrono::high_resolution_clock::now();

	struct packet ackPkt;


	Retry:
	pthread_mutex_lock(&bufferPktlock);
	//if there are buffer pkts, check each, see if the wanted ACK is in them
	if(bufferOccupied > 0) {
		for(int i = 0; i < bufferOccupied; i++) {
			deepCopyPkt(ackPkt, bufferPkts[i]);
			if(ackPkt.content_len != pkt.content_len || ackPkt.cmd_code != ACK || ackPkt.req_num != pkt.req_num || ackPkt.sessionId != pkt.sessionId) {
				continue;	//not the wanted one
			} else {	//got the wanted one
				bufferOccupied--;
				for(int j = i; j < bufferOccupied; j++) {
					deepCopyPkt(bufferPkts[j], bufferPkts[j+1]);
				}
				doTCPRead = false;
				string contentLengthString = to_string(pkt.content_len);
				int packetLength = 98 + contentLengthString.length() + stoi(contentLengthString);
				readError = packetLength;	//if get packet from buffer, change readError to packet length
			}
		}
	}
	pthread_mutex_unlock(&bufferPktlock);

	if(doTCPRead) {
		//set timeout, the read for ACK needs to timeout
		struct timeval tv;
		tv.tv_sec = TIMEOUT_SEC;
		tv.tv_usec = 0;
		if(setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv) < 0) {
			char errorMessage[ERR_LEN];
			fprintf(stderr, "setsockopt(TIMEOUT) failed; Error Message: %s\n", strerror_r(errno, errorMessage, ERR_LEN));
			return -5;
		}
		readError = read_socket_helper(socketfd, ackPkt);
		if(readError <= 0) {
			doTCPRead = false;
			goto Retry;
		}
	}
	if(readError <= 0) {
		fprintf(stderr, "Failed to Read ACK Packet\n");
		return -2;
	}

	pthread_mutex_lock(&bufferPktlock);
	if(ackPkt.cmd_code != ACK || ackPkt.req_num != pkt.req_num) {	//get unwanted packet, put it into buffer
		if(bufferOccupied > 9) {
			fprintf(stderr, "Buffer Queue Full\n");
			pthread_mutex_unlock(&bufferPktlock);
			return -4;
		} else {
			deepCopyPkt(bufferPkts[bufferOccupied], ackPkt);
			bufferOccupied++;
			pthread_mutex_unlock(&bufferPktlock);
			goto Retry;
		}
	}

	pthread_mutex_unlock(&bufferPktlock);

	if(ackPkt.sessionId != pkt.sessionId) {
		fprintf(stderr, "ACK Packet belong to other session\n");
		return -3;
	}
	if(ackPkt.content_len != pkt.content_len) {
		fprintf(stderr, "ACK Packet does not match\n");
		return -5;
	}

	Skip:
	pthread_mutex_lock(&logFilelock);
	FILE * logFile;
	logFile = fopen("log.txt","a");
	auto endTime = chrono::high_resolution_clock::now();
	std::chrono::duration<double> response_time = endTime - sendTime;
	std::time_t endTimeStamp = std::chrono::system_clock::to_time_t(endTime);
	std::time_t sendTimeStamp = std::chrono::system_clock::to_time_t(sendTime);
	fprintf(logFile, "Write %d byte at %s\t[len: %u | cmd: %s | num: %u | sid: %u]\n", writeError, std::ctime(&sendTimeStamp), pkt.content_len, getCommand(pkt.cmd_code), pkt.req_num, pkt.sessionId);
	fprintf(logFile, "Received ACK packet at %sResponse time: %f ms\n\n", std::ctime(&endTimeStamp), response_time.count()*1000);
	fclose(logFile);
	pthread_mutex_unlock(&logFilelock);
	return 0;
}

int read_socket(int socketfd, struct packet &pkt) {
	//turn off timeout if any
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	if(setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv) < 0) {
		char errorMessage[ERR_LEN];
		fprintf(stderr, "setsockopt(TIMEOUT) failed; Error Message: %s\n", strerror_r(errno, errorMessage, ERR_LEN));
		return -5;
	}

	pthread_mutex_lock(&bufferPktlock);
	if(bufferOccupied > 0) {
		for(int i = 0; i < bufferOccupied; i++) {
			deepCopyPkt(pkt, bufferPkts[i]);
			if(pkt.cmd_code != ACK) {
				bufferOccupied--;
				for(int j = i; j < bufferOccupied; j++) {
					deepCopyPkt(bufferPkts[j], bufferPkts[j+1]);
				}
				string contentLengthString = to_string(pkt.content_len);
				int packetLength = 98 + contentLengthString.length() + stoi(contentLengthString);
				pthread_mutex_unlock(&bufferPktlock);
				return packetLength;
			}
		}
	}
	pthread_mutex_unlock(&bufferPktlock);

	Retry:
	int readError = read_socket_helper(socketfd, pkt);
	if(readError <= 0)	//error in reading
		return readError;

	if(pkt.cmd_code == ACK) {
		pthread_mutex_lock(&bufferPktlock);
		if(bufferOccupied < 10) {
				deepCopyPkt(bufferPkts[bufferOccupied], pkt);
				bufferOccupied++;
				pthread_mutex_unlock(&bufferPktlock);
				goto Retry;
		} else {
			fprintf(stderr, "Recieved a ACK Packet, buffer Packet Queue Full\n");
		}
		pthread_mutex_unlock(&bufferPktlock);
		return -4;
	}

	struct packet ackPkt;
	deepCopyPkt(ackPkt, pkt);
	ackPkt.cmd_code = ACK;

	int writeError = write_socket_helper(socketfd, ackPkt);
	if(writeError > 0) {
		pthread_mutex_lock(&logFilelock);
		FILE * logFile;
		logFile = fopen("log.txt","a");
    		time_t timeStamp;
    		timeStamp = time(NULL);
    		fprintf(logFile, "Read %d byte at %s\t[len: %u | cmd: %s | num: %u | sid: %u]\n\n", readError, asctime(localtime(&timeStamp)),  pkt.content_len, getCommand(pkt.cmd_code), pkt.req_num, pkt.sessionId);
		fclose(logFile);
		pthread_mutex_unlock(&logFilelock);
		return readError;
	} else {
		fprintf(stderr, "Failed to Send ACK Packet\n");
		return -3;
	}

	return 0;
}
