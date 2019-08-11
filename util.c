#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <utf.h>
#include <fmt.h>
#include "dat.h"
#include "fns.h"

extern int debug;

void
sysfatal(char *fmt, ...)
{
	va_list arg;
	char buf[2048];

	va_start(arg, fmt);
	vseprint(buf, buf+sizeof(buf), fmt, arg);
	va_end(arg);
	fprint(2, "%s\n", buf);
	exit(1);
}

void *
emalloc(ulong n)
{
	void *p;

	p = malloc(n);
	if(p == nil)
		sysfatal("malloc: %r");
	memset(p, 0, n);
	return p;
}

SockaddrINET *
mkinetsa(char *ipaddr, int port)
{
	SockaddrINET *sa;

	sa = emalloc(sizeof(SockaddrINET));
	sa->sin_family = AF_INET;
	if(inet_pton(AF_INET, ipaddr, &sa->sin_addr.s_addr) <= 0)
		sysfatal("inet_pton: %r");
	sa->sin_port = htons(port);
	return sa;
}

int
listentcp(int port)
{
	SockaddrINET *addr;
	int fd;

	addr = mkinetsa("0.0.0.0", port);
	if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -1;
	if(bind(fd, (Sockaddr *)addr, sizeof(SockaddrINET)) < 0){
		close(fd);
		return -1;
	}
	if(listen(fd, 128) < 0){
		close(fd);
		return -1;
	}
	if(debug)
		fprint(2, "listening on %s!%d\n", inet_ntoa(addr->sin_addr), port);
	free(addr);
	return fd;
}

int
acceptcall(int lfd, char *caddr, int caddrlen)
{
	SockaddrINET addr;
	uint addrlen;
	int fd, port;
	char *cs;

	memset(&addr, 0, sizeof(SockaddrINET));
	addrlen = sizeof(SockaddrINET);
	if((fd = accept(lfd, (Sockaddr *)&addr, &addrlen)) < 0)
		return -1;
	cs = inet_ntoa(addr.sin_addr);
	port = ntohs(addr.sin_port);
	snprint(caddr, caddrlen, "tcp!%s!%d", cs, port);
	return fd;
}
