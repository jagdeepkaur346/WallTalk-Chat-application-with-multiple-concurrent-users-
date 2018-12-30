#include <pthread.h>
#include "func_lib.h"
#include  "mysql_lib.h"

extern pthread_cond_t notify_cond;
extern pthread_mutex_t notify_mutex;
extern MySQLDatabaseDriver databaseDriver;

#define SERVER_URL "tcp://127.0.0.1:3306"
#define SERVER_USERNAME "root"
#define SERVER_PASSWORD "socialnetworkpswd"
#define SERVER_DATABASE "SocialNetwork"
int notify_variable;

void processNotification()
{
	int sock_fd, read, sock_write;
	int ret = 0;
	DatabaseNotificationInterface notify(databaseDriver, SERVER_URL, SERVER_USERNAME,
			SERVER_PASSWORD, SERVER_DATABASE);

	pthread_mutex_lock(&notify_mutex);

	while (1)
	{
		while (!notify_variable)
		{
			pthread_cond_wait(&notify_cond, &notify_mutex);
		}
		notify_variable = 0;
		ret = notify.getNotifications();
		if (ret < 0)
		{
			printf("Error (getNotifications): get Notification failed\n");
			break;
		}
		while ((ret > 0) && (notify.next() > 0))
		{
			struct packet notifyPkt;
			sock_fd = notify.sendNotification(notifyPkt);
			if (sock_fd < 0)
			{
				printf("Error (sendNotification): Notification sending failed\n");
				break;
			}
			sock_write = write_socket(sock_fd, notifyPkt);
			if (sock_write < 0)
			{
				printf("Error (write_socket): Notification sending failed (skipping to next notification)\n");
				continue;
			}
			read = notify.markRead();
			if (read < 0)
			{
				printf("Error (markRead): Notification sending failed\n");
				break;
			}
		}
	}
	pthread_mutex_unlock(&notify_mutex);
	return;
}
