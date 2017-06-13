/* NAME: SHAAN MATHUR
 * EMAIL: SHAANKARANMATHUR@GMAIL.COM
 * UID: 904606576
 */

#include <stdio.h>
#include "mraa/aio.h"
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <mraa.h>
#include <openssl/ssl.h>
#include <getopt.h>
#include <stdlib.h>
#include <poll.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>

const int B = 4275;	// B value of the thermistor
const int R0 = 100000; // R0 = 100k

#define FAHR 0
#define CELS 1

double read_temperature(mraa_aio_context *temp, int mode)
{
	if (*temp == NULL)
	{
		fprintf(stderr, "MRAA structure NULL.\n");
		exit(1);
	}

	int a = mraa_aio_read(*temp);
	double R = 1023.0 / a - 1.0;
	R = R0 * R;

	double temperature = 1.0 / (log(R / R0) / B + 1 / 298.15) - 273.15; // convert to temperature via datasheet

	if (mode == FAHR)
	{
		temperature = temperature * 1.8 + 32;
	}
	//float R = 1023.0/read - 1.0;
	//R *= R0;

	//float temperature = 1.0/(log(R/R0)/B + 1/298.15)-273.15;

	return temperature;
}

void init(mraa_aio_context *t_sensor, mraa_gpio_context *button)
{
	*t_sensor = mraa_aio_init(0);
	if (*t_sensor == NULL)
	{
		fprintf(stderr, "Failed to initialize sensor.\n");
		exit(1);
	}

	*button = mraa_gpio_init(3);
	if (*button == NULL)
	{
		fprintf(stderr, "Failed to initialize button.\n");
		exit(1);
	}

	mraa_gpio_dir(*button, MRAA_GPIO_IN);
}

void print_usage()
{
	fprintf(stderr, "Usage: [--id=9DIGIT] [--host=HOSTNAME] [--log=FILENAME]\n");
}

void shut(int logfd)
{
	time_t rawtime;
	struct tm *timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);

	printf("%02d:%02d:%02d SHUTDOWN\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	if (logfd != -1)
		dprintf(logfd, "%02d:%02d:%02d SHUTDOWN\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

	exit(0);
}

void bad_input(int logfd)
{
	fprintf(stderr, "Bad input.\n");
	if (logfd != -1)
		dprintf(logfd, "Bad input.\n");
	exit(1);
}

void close_sensors(mraa_aio_context *a, mraa_gpio_context *b)
{
	mraa_aio_close(*a);
	mraa_gpio_close(*b);
}

int remote_connect(const char * hostname, int port) {
	// Get information on target server host/port
	int sockfd;
	struct sockaddr_in server_address;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "Error#: %d. Error Message: \"%s\"\r\n", errno, strerror(errno));
		exit(1);
	}
	struct hostent *server_name = gethostbyname(hostname);

	bzero((char *)&server_address, sizeof(server_address));												// Zero it out for padding purposes
	bcopy((char *)server_name->h_addr, (char *)&server_address.sin_addr.s_addr, server_name->h_length); // Copy over relevant data from server_name

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);

	if (connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
	{
		fprintf(stderr, "Error #: %d. Error Message: \"%s\"\r\n", errno, strerror(errno));
		exit(1);
	}

	return sockfd;
}

SSL* ssl_init(int sockfd) {
	SSL_CTX *ssl_obj = SSL_CTX_new(TLS_method());
	if(!SSL_set_fd(ssl_obj, sockfd)) {
		fprintf("SSL Setup Failure.\n");
		exit(1);
	}

	SSL_connect(ssl_obj);

	return ssl_obj;
}
int main(int argc, char **argv)
{
	unsigned int seconds = 1; // COMMAND LINE ARGUMENT
	int mode = FAHR;		  // COMMAND LINE ARGUMENT
	int logfd = -1;
	static struct option l_options[] = {
		{"id", required_argument, NULL, 'i'},
		{"host", required_argument, NULL, 'h'},
		{"log", required_argument, NULL, 'l'},
		{0, 0, 0, 0}};

	char c;

	char *id = NULL;		  // COMMAND LINE ARGUMENT
	char *hostname = NULL; 	  // COMMAND LINE ARGUMENT
	long port = -1;			  // COMMAND LINE ARGUMENT
	int x;
	for (x = 1; x < argc; x++) {
		if(argv[x][0] == '-')
			continue;
		port = atoi(argv[x]);
	}

	const int ID_LENGTH = 9;
	int size = 0;
	while ((c = getopt_long(argc, argv, "i:h:l:", l_options, NULL)) != -1)
	{
		switch (c)
		{
		case 'i':
			if (strlen(optarg) != ID_LENGTH)
			{
				fprintf(stderr, "Error: ID must be %d digits long", ID_LENGTH);
				print_usage();
				exit(1);
			}
			id = (char *)malloc(sizeof(char) * (ID_LENGTH + 1));
			strcpy(id, optarg);
			break;
		case 'h':
			size = strlen(optarg);
			hostname = (char *)malloc(sizeof(char) * (size + 1));
			strcpy(hostname, optarg);
			hostname[size] = 0; // Null terminated

			break;
		case 'l':
			logfd = open(optarg, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
			break;
		default:
			print_usage();
			exit(1);
		}
	}

	if (port == -1)	{
		fprintf(stderr, "Invalid port number.\n");
		print_usage();
		exit(1);
	}

	


	sockfd = remote_connect(hostname, port);
	SSL* ssl = ssl_init(sockfd);
	dprintf(sockfd, "ID=%s\n", id);

	mraa_aio_context t_sensor;
	mraa_gpio_context button;
	init(&t_sensor, &button);

	time_t rawtime;
	struct tm *timeinfo;

	int running = 1;
	struct pollfd fd;
	fd.fd = sockfd;
	fd.events = POLLIN | POLLHUP | POLLERR;

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	time_t prev;
	float tmp = read_temperature(&t_sensor, mode);

	// Run first initially to make sure at least one message is printed.
	if (tmp >= 10.0)
		dprintf(sockfd, "%02d:%02d:%02d %.1f\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, tmp);
	else
		dprintf(sockfd, "%02d:%02d:%02d 0%.1f\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, tmp);
	if (logfd != -1)
	{
		if (tmp >= 10.0)
			dprintf(logfd, "%02d:%02d:%02d %.1f\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, tmp);
		else
			dprintf(logfd, "%02d:%02d:%02d 0%.1f\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, tmp);
	}
	prev = rawtime;

	while (1)
	{
		if (mraa_gpio_read(button))
		{
			close_sensors(&t_sensor, &button);
			shut(logfd);
		}

		if (poll(&fd, 1, 0) < 0)
		{
			fprintf(stderr, "Error #:%d Error Message:%s\n", errno, strerror(errno));
			close_sensors(&t_sensor, &button);
			exit(1);
		}

		if (fd.revents & POLLIN)
		{
			// Put input into buffer
			int size = 0;
			int capacity = 1024;
			char *buff = (char *)malloc(sizeof(char) * capacity);
			do
			{
				int rc = read(fd.fd, (void *)buff, capacity - size);
				if (rc <= 0)
					break;
				size += rc;

				if (size >= capacity)
					buff = (char *)realloc(buff, (capacity *= 2));

				if (poll(&fd, 1, 0) < 0)
				{
					fprintf(stderr, "Error #:%d Error Message:%s\n", errno, strerror(errno));
					close_sensors(&t_sensor, &button);
					exit(1);
				}
			} while (fd.revents & POLLIN);

			buff = (char *)realloc(buff, size + 1);
			buff[size] = 0;

			// Break input into token seperated by newline
			const char nl[2] = "\n";
			char *token = strtok(buff, nl);
			while (token != NULL)
			{
				if (strcmp(token, "OFF") == 0)
				{
					if (logfd != -1)
						dprintf(logfd, "OFF\n");
					close_sensors(&t_sensor, &button);
					shut(logfd);
				}
				else if (strcmp(token, "STOP") == 0)
				{
					running = 0;
				}
				else if (strcmp(token, "START") == 0)
				{
					running = 1;
				}
				else if (strncmp(token, "SCALE=", 6) == 0 && strlen(token) == 7)
				{
					if (token[6] == 'F')
						mode = FAHR;
					else if (token[6] == 'C')
						mode = CELS;
					else
					{
						close_sensors(&t_sensor, &button);
						bad_input(logfd);
					}
				}
				else if (strncmp(token, "PERIOD=", 7) == 0 && strlen(token) > 7)
				{
					seconds = atoi(token + 7);
					if (seconds <= 0)
					{
						close_sensors(&t_sensor, &button);
						bad_input(logfd);
					}
				}
				else
				{
					close_sensors(&t_sensor, &button);
					bad_input(logfd);
				}

				if (logfd != -1)
					dprintf(logfd, "%s\n", token);
				token = strtok(NULL, nl);
			}
		}

		if (fd.revents & (POLLHUP | POLLERR))
		{
			close_sensors(&t_sensor, &button);
			fprintf(stderr, "Received POLLHUP or POLLERR from STDIN");
			exit(1);
		}

		// Update time if needed
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		tmp = read_temperature(&t_sensor, mode);

		if (running && rawtime - prev >= seconds)
		{
			tmp = floor(tmp * 10.0) / 10.0;
			if (tmp >= 10.0)
				dprintf(sockfd, "%02d:%02d:%02d %.1f\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, tmp);
			else
				dprintf(sockfd, "%02d:%02d:%02d 0%.1f\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, tmp);
			if (logfd != -1)
			{
				if (tmp >= 10.0)
					dprintf(logfd, "%02d:%02d:%02d %.1f\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, tmp);
				else
					dprintf(logfd, "%02d:%02d:%02d 0%.1f\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, tmp);
			}
			prev = rawtime;
		}
	}

	close_sensors(&t_sensor, &button);
	return 0;
}
