void sysfatal(char *s, ...);
void *emalloc(ulong);
SockaddrINET *mkinetsa(char *s, int);
int listentcp(int);
int acceptcall(int, char *, int);
