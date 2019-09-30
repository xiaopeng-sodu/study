#include <sys/epoll.h>
#include <fcntl.h>

static int 
sp_create(){
	return epoll_create(1024);
}

static int
sp_invalid(int sock){
	return sock <= 0 ? -1 : 0;
}

static int 
sp_add(int epfd, int sock, void *ud){
	struct epoll_event event; 
	event.events = EPOLLIN; 
	event.data.ptr = ud; 
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &event) == -1){
		return -1; 
	}
	return  0; 
}

static int 
sp_write(int epfd, int sock, void * ud){
	struct epoll_event event; 
	event.events = EPOLLOUT; 
	event.data.ptr = ud; 
	if (epoll_ctl(epfd, EPOLL_CTL_MOD, sock, &event) == -1){
		return -1; 
	}
	return 0;
}

static int 
sp_del(int epfd, int sock, void *ud){
	if (epoll_ctl(epfd, EPOLL_CTL_DEL, sock, NULL) == -1){
		return -1; 
	}
	return 0;
}

static int 
sp_wait(int epfd, struct event * events, int max){
	int n; 
	struct epoll_event evs[max];
	n = epoll_wait(epfd, evs, max, -1);
	if (n > 0){
		int i; 
		for (i = 0; i<n; i++){
			events[i] = evs[i].ptr; 
		}
	}
	return n; 
}

static int 
sp_nonblocking(int sock){
	int flag = fcntl(sock, F_GETFL, 0); 
	flag |= O_NONBLOCK; 
	return fcntl(sock, F_SETFL, flag);
}