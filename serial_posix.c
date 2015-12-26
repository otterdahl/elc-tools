/* SERIAL_POSIX */

#include <stdio.h>	/* Standard input/output definitions */
#include <string.h>	/* String function definitions */
#include <unistd.h>	/* UNIX standard function definitions */
#include <fcntl.h>	/* File control definitions */
#include <errno.h>	/* Error number definitions */
#include <termios.h>	/* POSIX terminal control definitions */
#include <ctype.h>	/* Character Class Tests*/

#define	HANDLE	int

/* 'open_port()' - Open serial port.
 * Return the file descriptor on success or -1 on error.
 */
int open_port(char* port)
{
	int fd; /* File descriptor for the port */

	fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1) {
		/* Could not open the port.  */
		perror("open_port: Unable to open /dev/ttyd0 - ");
	}
	else
		fcntl(fd, F_SETFL, 0);
	
	return (fd);
}

int close_port(HANDLE fd)
{
	close(fd);
	return 0;
}

int writeport(HANDLE fd, char t[])
{
	int n = write(fd, t, strlen(t));
	if (n < 0)
		fputs("write() failed!\n", stderr);
	return 0;
}

/* 1 second timeout enable 1/0 */
int set(HANDLE fd, int baud, int timeout)
{
	/* We should use the following ELC-settings by default:
	 * Baudrate: 1200
	 * Parity: Even
	 * Databits: 7
	 * Stopbits: 1
	 * 7-E-1
	 * No hardware flow control
	 */

	printf("Setting baud rate to %d\n",baud);
	struct termios options;

	/* Get the current options for the port...  */
	tcgetattr(fd, &options);

    /* Set the baud rates */
    switch(baud) {
        case 0:
            /* 1200 baud */
            cfsetispeed(&options, B1200);
            cfsetospeed(&options, B1200);
            break;
        case 1:
            /* 2400 baud */
            cfsetispeed(&options, B2400);
            cfsetospeed(&options, B2400);
            break;
        case 4:
            /* 4800 baud */
            cfsetispeed(&options, B4800);
            cfsetospeed(&options, B4800);
            break;
        case 5:
            /* 9600 baud */
            cfsetispeed(&options, B9600);
            cfsetospeed(&options, B9600);
            break;
        case 6:
            /* 19200 baud */
            cfsetispeed(&options, B19200);
            cfsetospeed(&options, B19200);
            break;
        case 7:
            /* 38400 baud */
            cfsetispeed(&options, B38400);
            cfsetospeed(&options, B38400);
            break;
        case 8:
            /* 115200 baud */
		    cfsetispeed(&options, B115200);
		    cfsetospeed(&options, B115200);
            break;
	}

	/* Enable the receiver and set local mode... */
	options.c_cflag |= (CLOCAL | CREAD);

	/* Standard settings: 8-O-1 .... 7-E-1*/
    options.c_cflag |= PARENB;	/* Enable parity bit */
    /*options.c_cflag |= PARODD; */ /* Use odd parity instead of even */
    options.c_cflag &= ~PARODD;     /* Use even parity instead of odd */
    options.c_cflag &= ~CSTOPB;	/* 1 stop bit */
    options.c_cflag &= ~CSIZE;	/* Mask the character size bits */
    /*options.c_cflag |= CS8;*/		/* Select 8 data bits */
    options.c_cflag |= CS7;		/* Select 7 data bits */

	/* Enable Hardware flow control */
	options.c_cflag |= CRTSCTS;	/* Also called CNEW_RTSCTS */

	/* Local options */
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	options.c_oflag &= ~OPOST;

	if(timeout == 1) {
		/* Sets 1 second timeout */
    		options.c_cc[VMIN]  = 0;
    		options.c_cc[VTIME] = 10;
	}
	else {
		options.c_cc[VMIN] = 1;
		options.c_cc[VTIME] = 0;
	}

	/* Set the new options for the port... */
	tcsetattr(fd, TCSANOW, &options);
	
	return 0;
}

//int readline(HANDLE fd, char* buf)
//{
//	int size;
//	char buffer[256];
//	char *bufptr;
//	bufptr = buffer;
//	size = read(fd, bufptr, buffer + sizeof(buffer) - bufptr -1);
//	buf = buffer;
//	return size;
//}

int readchar(HANDLE fd) {
	char b;
	int i = read(fd, &b, 1);
	if(i > 0)
		return b;
	else
		return 0;
}
