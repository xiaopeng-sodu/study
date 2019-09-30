#ifndef skynet_module_h
#define skynet_module_h

struct skynet_context; 

typedef void* (*skynet_dl_create)(void);
typedef int (*skynet_dl_init)(void *instance, struct skynet_context * ctx, void * param);
typedef void (*skynet_dl_release)(void* instance);
typedef void (*skynet_dl_signal)(void * instance, void *param);

struct skynet_module {
	const char * name; 
	void *module; 
	skynet_dl_create create; 
	skynet_dl_init init; 
	skynet_dl_signal signal;
	skynet_dl_release release;  
};

void skynet_module_init(const char *path);
struct skynet_module* skynet_module_query(const char *name);


#endif