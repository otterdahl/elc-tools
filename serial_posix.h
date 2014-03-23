int open_port(char*);
int writeport(HANDLE, char t[]);
int set(HANDLE, int, int);
int close_port(HANDLE);

/* returns num of read chars? 0 timeout, else error */
//int readline(HANDLE, char*);
int readchar(HANDLE);
