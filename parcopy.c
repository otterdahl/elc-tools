/* PARCOPY */
#include <stdio.h>	/* Standard input/output definitions */

#ifdef _WIN32 /* Windows */
#include <windows.h>
#include "serial_win32.h"
#define SECOND		2000

#else /* Posix */
#define	HANDLE	int
#define INVALID_HANDLE_VALUE -1
#define	SECOND		1
#include <string.h>	/* String function definitions */
#include <unistd.h>	/* UNIX standard function definitions */
#include <fcntl.h>	/* File control definitions */
#include <errno.h>	/* Error number definitions */
#include <termios.h>	/* POSIX terminal control definitions */
#include "serial_posix.h"
#endif

int init(int,HANDLE);
int response(HANDLE);
int get(HANDLE);
int put(HANDLE);
int terminal(HANDLE);
int cleanup(HANDLE);

/* By David Otterdahl <david.otterdahl@gmail.com>
 *
 * Open issues:
 * - terminal mode
 * - read/write checksum or somehow detection of communication loss?
 * - get() works with some hardware but fails with other, why? reading
 *   to slow?
 */

int get(HANDLE fd) {
	/* Sequence of operations during Read
	   <init>, PARDUMPOUT, <cleanup> */
	writeport(fd, "pardumpout\n");
	
	FILE *dump;
	if((dump=fopen("dump.hex","w")) == NULL) {
		printf("parcopy: can't open dump.hex for output\n");
		return -1;
	}

	printf("receving...\n");
	char line[10240];
	int il=0;
	int c;
	while((c=readchar(fd)) > 0) {
		if(c == '\n') {
			line[il] = '\0';
			if(il>0) {
				/* line has to start with ':' */
				if(line[0] == ':') {
					/* Print the line */
					if(il>=6) {
						printf("%c%c%c%c",line[3],line[4],line[5],line[6]);
						printf("\b\b\b\b");

						/* Remove carrige return.
						   Needed if linefeed option is set */
						//line[il-1] = '\0';  
					}
					fputs(line,dump);
					putc('\n',dump);
				}
			}
			il=0;

			/* Save line */
			/* Perhaps we should try looking for ESC or NULL? */
			/* if line is :0000001FF, break */
			if(strcmp(line,":00000001FF") == 0) {
				fclose(dump);
				return 0;
			}
		}
		else {
			line[il++] = c;
			if(il>10240) {
				printf("ran out of buffer space. this shouldn't happen\n");
				return 1;
			}
		}
	}
	fclose(dump);
	printf("read failed!\n");
	return 1;
}

int terminal(HANDLE fd) {
	 /*
	 * Sequence of operation during Terminal
	 * <init>
	 * <whatever the user types>
	 * <cleanup>
	 */
	/*
	printf("Entering Terminal mode. Use the EOF signal to exit. (Usually <ctrl>-Z)\n");
	char c[] = {0,0};
	int size;
	char buffer[1024];
	char *bufptr;
	int i;
	while(c[0] != EOF) {
		while ((c[0] = getchar()) != '\n')
			writeport(fd,c);
		sleep(SECOND);
		while((size=read(fd, bufptr, buffer + sizeof(buffer) - bufptr - 1)) > 0) {
			printf("%d new characters to be printed\n",size);
			for(i=0; i < size; i++) {
				printf("%c",bufptr[i]);
			}
		}
	}
	printf("Exiting Terminal mode\n");
	*/
	printf("Terminal mode is not finished.\n");
	return 0;
}


int init(int speed, HANDLE fd) {
	/* Initiate communications
	 * speed: 0  Automatic
	 * speed: 1200  Force 1200 baud
	 * speed: 2400  Force 2400 baud
	 * speed: 4800  Force 4800 baud
	 */

	set(fd, 0, 1);
	writeport(fd, "\33");	/* ESC */
	writeport(fd, "baud0\n");

	/* Wait two seconds for response */
	sleep(SECOND);

	int i=0;
	while(response(fd) == 1) {
		if(i==0) {
			printf("Trying 2400 baud\n");
			set(fd, 1, 1);
		}
		else if(i==1) {
			printf("Trying 4800 baud\n");
			set(fd, 4, 1);
		}
		else if(i==2) {
			printf("Trying 9600 baud\n");
			set(fd, 5, 1);
		}
		else if(i==3) {
			printf("Trying 9600 baud again\n");
			set(fd, 5, 1);
		}
		else {
			printf("No response. Check your equipment\n");
			return 1;
		}
		i++;

		/* Write ESC */
		writeport(fd, "\27");
		writeport(fd, "baud0\n");
		sleep(SECOND);
		set(fd, 0, 1);
		sleep(SECOND);
	}

	switch(speed) {
	case 1200:
		writeport(fd, "baud0\n");
		sleep(SECOND);
		set(fd, 0, 1);
		sleep(SECOND);
		break;
	case 2400:
		writeport(fd, "baud1\n");
		sleep(SECOND);
		set(fd, 1, 1);
		sleep(SECOND);
		break;
	case 4800:
		writeport(fd, "baud4\n");
		sleep(SECOND);
		set(fd, 4, 1);
		sleep(SECOND);
		break;
	default:
		writeport(fd, "baud5\n");
		sleep(SECOND);
		set(fd, 5, 1);
		sleep(SECOND);
		break;
	}

	return 0;
}

/* Waits for '>' response */
int response(HANDLE fd) {
	char c;
	printf("response: ");
	while((c=readchar(fd)) > 0) {
		if(c > 48 && c < 122)
			printf("%c",c);
		else
			printf(".");

		if(c=='>') {
			printf("\n");
			return 0;
		}
	}
	printf("\n");
	return 1;
}

int put(HANDLE fd) {
	 /*
	 * Sequence of operations during Write
	 * <init>, <Display warning>, LA, LB, PARDUMPIN, <cleanup>
	 */
	FILE *elc;
	if((elc=fopen("elc.hex","r")) == NULL) {
		printf("parcopy: can't open elc.hex for input\n");
		return -1;
	}

	printf("Warning. This will replace the programming of the controller! Are you sure? (Y/N)\n");
	char a;
	while(a=getchar()) {
		if(a == 'y' || a == 'Y')
			break;
		else if(a == 'n' || a == 'N')
			return 1;
	}
	writeport(fd, "la\n");
	writeport(fd, "54321\n");
	writeport(fd, "lb\n");
	writeport(fd, "54321\n");
	writeport(fd, "pardumpin\n");
	sleep(SECOND); /* Workaround: Otherwise data will be lost in the elc3sim command subroutine */
	char c[] = {0,0};
	while((c[0] = getc(elc)) != EOF) {
		writeport(fd,c);
	}
	writeport(fd,"\33");	/* Write ESC to signal that we are done */ 
	fclose(elc);
	printf("done.\n");
	sleep(SECOND); /* Workaround: Otherwise commands might be lost in the elc3sim parcopy subroutine */
	return 0;
}

int cleanup(HANDLE fd) {
	printf("cleanup\n");
	writeport(fd,"lock\n");
	writeport(fd,"linefeed\n");
	writeport(fd,"baud0\n");
	sleep(SECOND);
	//set(fd, 0, 1);
	return 0;
}

/* The speed setting paramater differs from the original implementation */
int main(int argc, char* argv[]) {
	HANDLE fd;
	int mode;
	if(argc < 3) {
		fputs("Usage: parcopy [r|w|t] [port (eg. COM1 (win32), /dev/ttyS0 (Linux), /dev/cuad0 (FreeBSD))] (baud (1200/2400/4800))\n", stderr);
		return -1;
	}
	if((fd = open_port(argv[2])) == INVALID_HANDLE_VALUE)
		return -1;

	switch(argv[1][0]) {
		case 'r': mode=0; break;
		case 'w': mode=1; break;
		case 't': mode=2; break;
		default: fputs("Unknown mode\n", stderr);
			return 1;
	}
	
	printf("arguments: %d\n",argc);
	int speed=0;
	if(argc == 4) {
		printf("using custom speed\n");
		if(strcmp(argv[3], "1200") == 0)
			speed = 1200;
		else if(strcmp(argv[3], "2400") == 0)
			speed = 2400;
		else if(strcmp(argv[3], "4800") == 0)
			speed = 4800;
		else {
			printf("Unknown speed\n");
			return 1;
		}
	}

	if(init(speed, fd)) {
		fputs("Communication error\n", stderr);
		close_port(fd);
		return 1;
	}

	if(mode == 0)
		get(fd);
	else if(mode == 1)
		put(fd);
	else if(mode == 2)
		terminal(fd);

	cleanup(fd);
	close_port(fd);
	return 0;
}
