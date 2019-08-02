#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <utf.h>
#include <fmt.h>
#include "args.h"

#define nil NULL

typedef unsigned int uint;

enum {
	LPORT = 6666
};

int lfd, ncpu;
pthread_t *threads;
pthread_mutex_t attendlock;

int debug;
char *argv0;

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

int
listentcp(int port)
{
	struct sockaddr_in addr;
	int fd;

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
	if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -1;
	if(bind(fd, (struct sockaddr *)&addr, sizeof addr) < 0){
		close(fd);
		return -1;
	}
	if(listen(fd, 32) < 0){
		close(fd);
		return -1;
	}
	if(debug)
		fprint(2, "listening on %s!%d\n", inet_ntoa(addr.sin_addr), port);
	return fd;
}

int
acceptcall(int lfd, char *caddr, int caddrlen)
{
	struct sockaddr_in addr;
	uint addrlen;
	int fd, port;
	char *cs;

	memset(&addr, 0, sizeof addr);
	if((fd = accept(lfd, (struct sockaddr *)&addr, &addrlen)) < 0)
		return -1;
	cs = inet_ntoa(addr.sin_addr);
	port = ntohs(addr.sin_port);
	snprint(caddr, caddrlen, "tcp!%s!%d", cs, port);
	caddr[caddrlen-1] = 0;
	if(debug)
		fprint(2, "thr#%lu accepted call from %s\n", pthread_self(), caddr);
	return fd;
}

void *
tmain(void *a)
{
	char caddr[128], buf[1024];
	int cfd, n;

	for(;;){
		pthread_mutex_lock(&attendlock);
		if((cfd = acceptcall(lfd, caddr, sizeof caddr)) < 0)
			sysfatal("acceptcall: %r");
		pthread_mutex_unlock(&attendlock);
		while((n = read(cfd, buf, sizeof buf)) > 0)
			if(write(1, buf, n) != n)
				break;
		close(cfd);
	}
}

static void
usage(void)
{
	fprint(2, "usage: %s [-d]\n", argv0);
	exit(1);
}

int
main(int argc, char *argv[])
{
	int i;

	ARGBEGIN{
	case 'd': debug++; break;
	}ARGEND;
	if(argc != 0)
		usage();
	if((lfd = listentcp(LPORT)) < 0)
		sysfatal("listen: %r");
	ncpu = get_nprocs_conf();
	if(ncpu < 1)
		ncpu = 1;
	threads = emalloc(ncpu*sizeof(pthread_t));
	pthread_mutex_init(&attendlock, nil);
	for(i = 0; i < ncpu; i++){
		pthread_create(threads+i, nil, tmain, nil);
		if(debug)
			fprint(2, "created thr#%lu\n", *threads+i);
	}
	pause();
	free(threads);
	pthread_mutex_destroy(&attendlock);
	exit(0);
}