/* SERIAL_WIN32 */
#include <stdio.h>
#include <windows.h>
#include "serial_win32.h"
int readtimeout=1000;

HANDLE open_port(char* port)
{
	HANDLE fd;

	fd = CreateFile(port, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED | FILE_ATTRIBUTE_NORMAL, 0);
	if (fd == INVALID_HANDLE_VALUE)
		printf("error opening port; abort\n");	
	return fd;
}

int close_port(HANDLE hComm)
{
	CloseHandle(hComm);
	return 0;
}

int writeport(HANDLE hComm, char * lpBuf)
{
	DWORD dwToWrite = strlen(lpBuf);
	OVERLAPPED osWrite = {0};
	DWORD dwWritten;
	int fRes;
	
	// Create this writes OVERLAPPED structure hEvent.
	osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (osWrite.hEvent == NULL) {
		printf("Error creating overlapped event handle.\n");
		err(hComm);
		return 1;
	}

	// Issue write.
	if (!WriteFile(hComm, lpBuf, dwToWrite, &dwWritten, &osWrite)) {
		if (GetLastError() != ERROR_IO_PENDING) {
			printf("WriteFile failed, but isn't delayed.\n");
			err(hComm);
			fRes = 1;
		}
		else {
		// Write is pending.
		if (!GetOverlappedResult(hComm, &osWrite, &dwWritten, TRUE)) {
			printf("GetOverlappedResult failed.\n");
			err(hComm);
            fRes = 1;
		}
        else
            fRes = 0;            // Write operation completed successfully.
		}
	}
	else
		// WriteFile completed immediately.
		fRes = 0;

   CloseHandle(osWrite.hEvent);
   return fRes;
}

int set(HANDLE hComm, int baud)
{
	DCB dcb;

	FillMemory(&dcb, sizeof(dcb), 0);
	if (!GetCommState(hComm, &dcb))	// get current DCB  
		return 1; // Error in GetCommState

	/* Set communications settings to match ELC-2/3 */
	dcb.ByteSize = 7;
	dcb.StopBits = ONESTOPBIT;
	dcb.Parity = EVENPARITY;

	/* TODO: Set Flow control = None */
	dcb.fOutxCtsFlow = FALSE;
	dcb.fOutxDsrFlow = FALSE;
	
	switch(baud) {
		case 0:
			dcb.BaudRate = CBR_1200;
			break;
		case 1:
			dcb.BaudRate = CBR_2400;
			break;
		case 4:
			dcb.BaudRate = CBR_4800;
			break;
		case 5:
			dcb.BaudRate = CBR_9600;
			break;
		case 6:
			dcb.BaudRate = CBR_19200;
			break;
		case 7:
			dcb.BaudRate = CBR_38400;
			break;
		default:
			dcb.BaudRate = CBR_1200;
	}
	
	printf("switching speed %d\n", baud);
	
	if (!SetCommState(hComm, &dcb))	// Set new state.
	/* Possibly a problem with the communications 
	   port handle or a problem with the DCB structure itself. */
		return 1;

	/* Move it here to prevent compilation errors */
	int timeout=1;
	readtimeout = 100;

	/* Notice the diffrence between Connection Timeouts and Read Timeout
	   Maybe we should save and restore the communication settings
	   upon start and exit */
	COMMTIMEOUTS timeouts;
	if(timeout == 1) {
		timeouts.ReadIntervalTimeout = 20; 
		timeouts.ReadTotalTimeoutMultiplier = 10;
		timeouts.ReadTotalTimeoutConstant = 100;
		timeouts.WriteTotalTimeoutMultiplier = 10;
		timeouts.WriteTotalTimeoutConstant = 100;
	}
	else {
		timeouts.ReadIntervalTimeout = MAXDWORD; 
		timeouts.ReadTotalTimeoutMultiplier = 0;
		timeouts.ReadTotalTimeoutConstant = 0;
		timeouts.WriteTotalTimeoutMultiplier = 0;
		timeouts.WriteTotalTimeoutConstant = 0;
	}
	if (!SetCommTimeouts(hComm, &timeouts))
		// Error setting time-outs.
		return 1;
    return 0;
}

int readchar(HANDLE hComm) {
#define	READ_BUF_SIZE	1
	OVERLAPPED osReader = {0};
	DWORD dwRes;
	DWORD dwRead;
	char lpBuf[255];
	int fRes;

	osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (osReader.hEvent == NULL) {
	   printf("Error creating overlapped event; abort.\n");
	   err(hComm);
	   return -1;
	}

	int iWait = 0;

	if (!ReadFile(hComm, lpBuf, READ_BUF_SIZE, &dwRead, &osReader)) {
		if (GetLastError() != ERROR_IO_PENDING) {   // read not delayed?
			printf("ReadFile failed, but isn't delayed.\n");
			err(hComm);
			fRes = -1;
		}
		else {
			iWait=1;
			while(iWait == 1) {

				dwRes = WaitForSingleObject(osReader.hEvent, readtimeout);
				switch(dwRes) {
			    case WAIT_OBJECT_0:
					if (!GetOverlappedResult(hComm, &osReader, &dwRead, FALSE)) {
						printf("Error in GetOverlappedResult.\n");
						err(hComm);
						fRes = -1;
					}
			        else {/* Success */
						iWait = 0;
						/* Check if we've actually read anything */
						if(dwRead == 1)
							fRes = lpBuf[0];
						else
							fRes = 0;
					}
			        break;
			    case WAIT_TIMEOUT: /* Timeout */
					fRes = 0;
					break;                       
				default: /* Error */
					printf("Error in read\n");
					err(hComm);
					fRes = -1;
					break;
				}
			}
		}
	}
	else {
		/* ReadFile completed immediately */

		/* Check if we've actually read anything */
		if(dwRead == 1)
			fRes = lpBuf[0];
		else
			fRes = 0;
	}
	CloseHandle(osReader.hEvent);
	return fRes;
}

void err(HANDLE hComm) {
	COMSTAT comStat;
    DWORD   dwErrors;
    BOOL    fOOP, fOVERRUN, fPTO, fRXOVER, fRXPARITY, fTXFULL;
    BOOL    fBREAK, fDNS, fFRAME, fIOE, fMODE;

    // Get and clear current errors on the port.
    if (!ClearCommError(hComm, &dwErrors, &comStat))
        // Report error in ClearCommError.
        return;

    // Get error flags.
    fDNS = dwErrors & CE_DNS;
    fIOE = dwErrors & CE_IOE;
    fOOP = dwErrors & CE_OOP;
    fPTO = dwErrors & CE_PTO;
    fMODE = dwErrors & CE_MODE;
    fBREAK = dwErrors & CE_BREAK;
    fFRAME = dwErrors & CE_FRAME;
    fRXOVER = dwErrors & CE_RXOVER;
    fTXFULL = dwErrors & CE_TXFULL;
    fOVERRUN = dwErrors & CE_OVERRUN;
    fRXPARITY = dwErrors & CE_RXPARITY;

    // COMSTAT structure contains information regarding
    // communications status.
    if (comStat.fCtsHold)
        printf("Tx waiting for CTS signal\n");
    if (comStat.fDsrHold);
        printf("Tx waiting for DSR signal\n");
    if (comStat.fRlsdHold);
        printf("Tx waiting for RLSD signal\n");
    if (comStat.fXoffHold);
        printf("Tx waiting, XOFF char rec'd\n");
    if (comStat.fXoffSent);
        printf("Tx waiting, XOFF char sent\n");
    if (comStat.fEof);
        printf("EOF character received\n");
    if (comStat.fTxim);
        printf("Character waiting for Tx; char queued with TransmitCommChar\n");
    if (comStat.cbInQue);
        printf("comStat.cbInQue bytes have been received, but not read\n");
    if (comStat.cbOutQue);
        printf("comStat.cbOutQue bytes are awaiting transfer\n");
}
