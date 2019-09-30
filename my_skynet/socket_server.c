#include "socket_server.h"

#include <unistd.h>

struct socket_server {
	int epfd; 
	int writectrl_fd; 
	int readctrl_fd; 
	int checkctrl; 
	int event_index; 
	int event_n; 
	struct socket *slot; 
	struct send_obj_interface  soi; 
	struct fd_set * rdfs;
};


struct socket_server * 
socket_server_create(){
	int epfd; 
	int fd[2];
	if (pipe(fd)){
		fprintf(stderr,  "create pipe failed.\n");
		return NULL;
	}
	epfd = epoll_create();
}