#ifndef MYSQL_LIB_H_
#define MYSQL_LIB_H_

#include <stdlib.h>
#include <iostream>
#include <string>
#include <climits>
#include <cstdlib>
#include <math.h>

#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include "structures.h"
using namespace std;

string wall_entry_format(string timestamp, string poster, string postee,
		string content);
//formats wall entry consistently across classes

class MySQLDatabaseDriver {
	/*
	 * Call this once in the global space to initialize the MySQLDriver
	 */
public:
	sql::Driver *driver;

	MySQLDatabaseDriver();
	~MySQLDatabaseDriver();
};

class DatabaseCommandInterface {
	/*
	 * Call this in each client handler thread that needs to connect to the database.
	 * It handles database connections only within a thread.
	 */
public:
	unsigned int session_timeout = 15; // in minutes between 0 and 59. Should be set the same across all threads
	unsigned int session_id_max = UINT_MAX;

	DatabaseCommandInterface(MySQLDatabaseDriver databaseDriver,
			std::string server_url, std::string server_username,
			std::string server_password, std::string server_database);
	~DatabaseCommandInterface();

	void getResults(std::string query);
	/*
	 * Input query is a valid SQL statement that returns rows
	 * Output is SQL results printed to standard out
	 * Used for testing
	 */

	/*
	 * the following functions take the request packet input, perform SQL queries and then overwrite the request
	 * packet with the corresponding response packet. They return 0 if successful and
	 * -1 if unsuccessful. If unsuccessful, rcvd_cnts will also contain an error message.
	 */

	int hasValidSession(struct packet& pkt, unsigned int* user_id = NULL,
			unsigned int* socket_descriptor = NULL);
	/*
	 * This function checks if the session in the packet is valid based on session_timeout
	 * and logout status. If the session is valid and variables are passed in,
	 * the function can return the user_id and socket_descriptor associated with the session.
	 *
	 * returns:
	 * 0 if valid
	 * -1 for invalid session and modifies packet to have error message
	 * -2 for server error and modifies packet to have error message
	 */

	int login(struct packet& pkt, unsigned int socket_descriptor);
	/*
	 * Checks if username and password exist in the table. If so, generates
	 * a valid sessionID and writes that ID to the packet and returns 0.
	 * If not, writes an error message to received contents and returns -1.
	 * If server error, writes an error message to received contents and returns -2.
	 */

	int listUsers(struct packet& pkt);
	/*
	 * Queries database for list of all users. Writes numbered list of users
	 * with newlines between users to rcvd_cnts.
	 * Ex:
	 * 1. alice
	 * 2. bob
	 *
	 * Returns 0 if successful, or -2 if server error and writes error message to rcvd_cnts
	 */

	int showWall(struct packet& pkt);
	/*
	 * Queries database for list of posts on a user's wall. Writes a formatted
	 * string of posts to rcvd_cnts.
	 * Ex:
	 * timestamp - Alice posted on Bob's wall
	 * Oh my god! Politics!
	 *
	 * timestamp - Claire posted on Bob's wall
	 * I know, right!
	 *
	 * Returns 0 if successful,
	 * or -1 if unsuccessful and writes error message to rcvd_cnts,
	 * or -2 if server error and writes error message to rcvd_cnts
	 */

	int postOnWall(struct packet& pkt);
	/*
	 * Creates a post on the specified user's wall.
	 *
	 * If successful, returns 0 and rcvd_cnts should be ignored
	 * Otherwise:
	 * returns -1 if unsuccessful and writes error message to rcvd_cnts
	 * returns -2 if server error and writes error message to rcvd_cnts
	 */

	int logout(struct packet& pkt);
	/*
	 * Marks the user as logged out. This invalidates the session id
	 *
	 * If successful, returns 0
	 * Otherwise:
	 * returns -2 if server error and writes error message to rcvd_cnts
	 */

private:
	sql::Driver* driver;
	sql::Connection* con;
	sql::Statement* stmt;
	sql::PreparedStatement* pstmt;
	sql::ResultSet* res;
	sql::ResultSetMetaData* result_set_meta_data;

	void printResults();
	/*
	 * Used for testing
	 * Prints existing resultset in a table format. Can only be called once a
	 * result set has been generated. Restores the result set state before returning
	 */

	int insertInteractionLog(unsigned int session_id, bool logout,
			std::string command, unsigned int user_id = 0,
			unsigned int socket_descriptor = 0);
	/*
	 * Used for updating InteractionLog
	 *
	 * Should only be called after existing statements, prepared statements,
	 * or result sets have been deleted as this modifies the private prepared statement
	 * and result set variables. Creates own prepared statements for updating database.
	 *
	 * Returns 0 if successful, returns -2 if unintended SQL behavior/server error
	 */

	int getUserID(std::string user_name, unsigned int* user_id = NULL);
	/*
	 * Used for checking if a user exists in the database. Passes userID back if supplied with pointer
	 *
	 * Should only be called after existing statements, prepared statements,
	 * or result sets have been deleted as this modifies the private prepared statement
	 * and result set variables. Creates own prepared statements for querying database.
	 *
	 * Returns 0 if user exists, returns -1 if user doesn't exist, returns -2 if unintended SQL behavior/server error
	 */
};

class DatabaseNotificationInterface {
	/*
	 * The Notification object handles querying the database for notifications,
	 * iterating through the notifications, generating a packet to send,
	 * and updating the database if a notification is successfully sent to the client.
	 *
	 * It requires a MySQLDatabaseDriver to have been initialized and passed to it.
	 *
	 * This object should only be used within the notifications thread
	 *
	 * Thread safety: Should only be used in one thread at a time
	 */
public:
	int session_timeout = 15; // in minutes between 0 and 59. Should be set the same across all threads

	DatabaseNotificationInterface(MySQLDatabaseDriver databaseDriver,
			std::string server_url, std::string server_username,
			std::string server_password, std::string server_database);
	~DatabaseNotificationInterface();

	int getNotifications(void);
	/*
	 * queries the database for notifications to process. Returns the number of notifications
	 * to process.
	 *
	 * Returns:
	 * 0 or positive int if successful
	 * -2 if server error
	 */

	int next(void);
	/*
	 * Iterates the Notification object to the next entry. Returns the row number if the entry exists.
	 * Starts on an empty "0th" entry so much be called once to get to the first entry.
	 *
	 * Returns:
	 * row number if entry exists
	 * -1 if no more entries
	 * -2 if server error
	 */

	int sendNotification(struct packet& pkt);
	/*
	 * Generates the notification packet and returns the socket descriptor to send it to.
	 *
	 * Returns:
	 * socket descriptor if successful
	 * -2 if server error
	 */
	int markRead(void);
	/*
	 * Marks the current notification as read (acknowledged by client code).
	 *
	 * Returns:
	 * row number of notification if successful.
	 * -2 if server error
	 */

private:
	sql::Driver* driver;
	sql::Connection* con;
	sql::PreparedStatement* pstmt_get_notifications;
	sql::ResultSet* res_get_notifications;
	sql::PreparedStatement* pstmt_mark_read;
	bool notifications_generated = false;
	/* used for garbage collection of statements and result sets and to
	 * ensure that functions aren't run on uncreated statements
	 */
};

#endif /* MYSQL_LIB_H_ */
