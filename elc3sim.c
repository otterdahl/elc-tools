/* ELC3SIM */
#include <stdio.h>	/* Standard input/output definitions */
#ifdef _WIN32
#include <windows.h>
#else
#define	HANDLE	int
#define	INVALID_HANDLE_VALUE	-1
#include <string.h>	/* String function definitions */
#include <unistd.h>	/* UNIX standard function definitions */
#include <fcntl.h>	/* File control definitions */
#include <errno.h>	/* Error number definitions */
#include <termios.h>	/* POSIX terminal control definitions */
#include <ctype.h>	/* Character Class Tests */
#include "serial_posix.h"
#endif

/* By David Otterdahl <david.otterdahl@gmail.com>
 * Emulation of ELC-3 serial communications
 * Helps to debug the parcopy/parascii programs
 *
 * elc3sim is in a fairly decent state, but so far POSIX only.
 *
 * PARDUMPOUT works
 * PARDUMPIN works somewhat, saving to pardumpin.txt
 *
 * Race conditions when switching between PARDUMP-modes and command modes
 * The remaining content of the buffer is ignored and lost
 */

int elcf(char*,int,FILE*);
int readport(int fd, FILE *fp);

int lf = 1;		/* Linefeed */
int main(int argc, char* argv[])
{
	int fd;
	FILE *hex;
	if(argc < 3) {
		fputs("Usage: elc3sim [port (eg. /dev/ttyS0 or /dev/cuad0)] [hex-file]\n", stderr);
		return -1;
	}
	if((hex=fopen(argv[2],"r")) == NULL) {
		printf("elc3sim: can't open %s\n", argv[2]);
		return -1;
	}
	if((fd = open_port(argv[1])) == INVALID_HANDLE_VALUE)
		return -1;
	set(fd,0,0);	/* No timeout */
	readport(fd, hex);
	close_port(fd);
	fclose(hex);
	return 0;
}

int readport(int fd, FILE *fp)
{
	/* Currently in blocking mode */
	fcntl(fd, F_SETFL, 0 /* FNDELAY */);
	int size;
	char buffer[255];
	char *bufptr;

	char command[255];
	int ic=0;
	
	int i;
	int ai=0;	/* Additional input, certain commands */
	bufptr = buffer;
	char echo[] = {0,0};
	while((size=read(fd, bufptr, buffer + sizeof(buffer) - bufptr - 1)) > 0) {

		if(size>0)
			//printf("%d chars detected...", size);
		for(i=0; i < size; i++) {
			//printf("'%c' (%d) ", bufptr[i], bufptr[i]);

			// ECHO, don't echo enter
			if(bufptr[i] != 10) {
				bufptr[i] = echo[0] = toupper(bufptr[i]);
				writeport(fd,echo);
			}

			if(bufptr[i] == '\n' || bufptr[i] == '\r') {
				command[ic] = '\0';

				if(ai==2) { /* additional input */
					printf(" input: %s\n", command /* buffer */);
					writeport(fd,"\r");
					if(lf == 1)
						writeport(fd, "\n");
					writeport(fd, "\r> ");
					ai=0;
				}
			       	else {
					printf("command detected: %s\n", command /*buffer*/);
					ai = elcf(command, fd, fp);
				}
				ic=0;
			}
			else if(bufptr[i] == 27 || bufptr[i] == 0) {/* ESC */
				writeport(fd, "\r> ");
			}
			else {
				command[ic++] = bufptr[i];
			}
		}
		bufptr = buffer;
	}
	return 0;
}

/* elcf: elc functions
 * BAUDx: change baud rate
 * BAUD0: 1200 baud
 * BAUD1: 2400 baud
 * BAUD2: 300 baud
 * BAUD3: 600 baud
 * BAUD4: 4800 baud
 * BAUD5: 9600 baud
 * LOCK: Locks all lock levels
 * LINEFEED: sends a linefeed '\n' at the end of each line
 * CRTxx/TTYxx: Selects between printer type terminal and crt type terminal.
 *              The line length is given as parameter.
 *              eg. a 80 char line: CRT80
 *              With TTY, status display commands xxINT (e.g. GRINT) displays
 *              line/second
 * STAT: Prints status info about the intersection, etc.
 */
int elcf(char* f, int fd, FILE *fp)
{
	if(strcmp(f, "STAT") == 0) {	/* Status */
		writeport(fd, "\rStatus");
		writeport(fd, "\rELC-3");
	}
	else if( strcmp(f, "BAUD0") == 0)
		set(fd, 0, 0);
	else if( strcmp(f, "LINEFEED") == 0)
		lf = 1;
	else if( strcmp(f, "NOLINEFEED") == 0)
		lf = 0;
	else if( strcmp(f, "BAUD4") == 0)
		set(fd, 4, 0);
	else if( strcmp(f, "BAUD5") == 0)
		set(fd, 5, 0);
	else if( strcmp(f, "LOCK") == 0)
		;
	else if( strcmp(f, "CRT20") == 0)
		;
	else if( strcmp(f, "CRT40") == 0)
		;
	else if( strcmp(f, "LA") == 0) {
		writeport(fd,"\r");
		return 2; /* Wait for additional input */ 
	}
	else if( strcmp(f, "LB") == 0) {
		writeport(fd,"\r");
		return 2; /* Wait for additional input */
	}
	else if( strcmp(f, "PARDUMPOUT") == 0) {
		printf("dumping...");
		writeport(fd,"\n");
		char c[] = {0,0};
		while ((c[0] = getc(fp)) != EOF) {
			writeport(fd,c); 
			//printf("%c",c[0]);
			//TODO: listen for ESC
		}
		rewind(fp);
		printf("done\n");
	}
	else if( strcmp(f, "PARDUMPIN") == 0) {
		printf("Entering PARDUMPIN mode\n");
		FILE *parin;
		if((parin=fopen("pardumpin.txt","w")) == NULL) {
			printf("can't open file for output, skipping...\n");
		}
		else {
			printf("Saving input in pardumpin.txt\n");
		}

		int size;
		char buffer[1024];
		char *bufptr;
		int i;

		char command[255];
		int ic=0;

		bufptr = buffer;
		writeport(fd, "\r");
		if(lf == 1)
			writeport(fd, "\n");
		writeport(fd, "\r> ");
		
		while((size=read(fd, bufptr, buffer + sizeof(buffer) - bufptr - 1)) > 0)
		{

			for(i=0; i < size; i++) {

				if(bufptr[i] == '\n') {
					command[ic] = '\0';
					/* WRITE LINE TO FILE */
					if(parin != NULL && ic > 0) {
						fputs(command,parin);
						putc('\r',parin);
						putc('\n',parin);
					}
					//printf("input: %s\n", buffer);
					//bufptr = buffer;
					//*buffer = '\0';
					if(lf == 1)
						writeport(fd, "\n");
					writeport(fd, "\r> ");
					ic=0;
				}
				else if(bufptr[i] == 27 || bufptr[i] == 0) {/* ESC */
					printf("ESC detected \n");
					writeport(fd, "\r> ");
					bufptr = buffer;
					goto exit;
				}
				else if(bufptr[i] != '\r')
					command[ic++] = bufptr[i];
			}

			bufptr = buffer;
		}
exit:
		/* TODO: There might still be data in buffer that we leave behind
		 * Current workaround is to simply add a sleep(1) in the parcopy implementation
		 */
		printf("Exiting PARDUMPIN mode\n");
		if(parin != NULL)
			fclose(parin);
	}
	else if( (strcmp(f, "\n") == 0) || 
		(strcmp(f, "\r") == 0)) {	/* Enter */
			;
	}
	else {
		printf("Unknown command - %s\n", f);
		writeport(fd, "    Syntax error");
	}
	if(lf == 1)
		writeport(fd, "\n");  // Linefeed
	writeport(fd, "\r> ");
	return 0;
} 


