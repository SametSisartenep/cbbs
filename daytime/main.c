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
#include "../dat.h"
#include "../fns.h"
#include "../args.h"

enum {
	LPORT = 13
};

int lfd, ncpu;
pthread_t *threads;
pthread_mutex_t attendlock;

int debug;
char *argv0;

void *
tmain(void *a)
{
	char caddr[128], buf[1024];
	int cfd;
	time_t t;

	for(;;){
		pthread_mutex_lock(&attendlock);
		if((cfd = acceptcall(lfd, caddr, sizeof caddr)) < 0)
			sysfatal("acceptcall: %r");
		pthread_mutex_unlock(&attendlock);
		if(debug)
			fprint(2, "thr#%lu accepted call from %s\n", pthread_self(), caddr);
		t = time(nil);
		snprint(buf, sizeof buf, "%.24s\r\n", ctime(&t));
		if(write(cfd, buf, strlen(buf)) != strlen(buf))
			sysfatal("write: %r");
		close(cfd);
		if(debug)
			fprint(2, "thr#%lu ended call with %s\n", pthread_self(), caddr);
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
	threads = emalloc(sizeof(pthread_t)*ncpu);
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
