#include "skynet_server.h"
#include "spinlock.h"

#include <malloc.h>
#include <string.h>
#include <assert.h>

#ifdef CHECK_CALLING 

#define CHECKCALLING_INIT(ctx)  spinlock_init(&ctx->calling);
#define CHECKCALLING_BEGIN(ctx) if(!spinlock_trylock(&ctx->calling)) {assert(0);}
#define CHECKCALLING_END(ctx) spinlock_unlock(&ctx->calling);
#define CHECKCALLING_DESTROY(ctx) spinlock_destroy(&ctx->calling);
#define CHECKCALLING_DECL  struct spinlock_t calling; 

else

#define CHECKCALLING_INIT(ctx)
#define CHECKCALLING_BEGIN(ctx)
#define CHECKCALLING_END(ctx)
#define CHECKCALLING_DESTROY(ctx)
#define CHECKCALLING_DECL

#endif

typedef int(*skynet_cb)(struct skynet_context * ctx, void* ud, int type, int session, int source, void *ud, int size);


struct skynet_context {
	void *instance; 
	struct skynet_module * mod; 
	skynet_cb cb; 
	void *cb_ud; 
	struct message_queue * queue; 
	int handle; 
	FILE *logfile;
	bool init; 
	bool endless; 
	int ref; 
	int session_id; 
	char result[32];

	CHECKCALLING_DECL
};

struct skynet_context * 
skynet_context_new(const char *name, void *param){
	struct skynet_module *mod = skynet_module_query(name);
	if(mod == NULL){
		return NULL
	}
	void * inst = skynet_module_instance_create(mod);
	if (inst == NULL){
		return NULL;
	}
	struct skynet_context * ctx = malloc(sizeof(struct skynet_context));
	memset(ctx, 0, sizeof(struct skynet_context));

	CHECKCALLING_INIT(ctx)
	ctx->cb = NULL;
	ctx->cb_ud = NULL;
	ctx->mod = mod;
	ctx->instance = inst;
	ctx->handle = 0; 
	ctx->session_id = 0;
	ctx->handle = skynet_handle_register(ctx);
	ctx->queue = skynet_mq_create(handle);
	ctx->logfile = NULL;
	ctx->init = false;
	ctx->ref = 2; 
	ctx->endless = false; 

	struct message_queue * queue = ctx->queue;
	CHECKCALLING_BEGIN(ctx)
	int r = skynet_module_instance_init(ctx);
	CHECKCALLING_END(ctx)

	if (r == 0){
		struct skynet_context * ret = skynet_context_release(ctx);
		if (ret){
			ctx->init = true;
		}
		skynet_globalmq_push(queue);
		if (ret){
			skynet_error(ret, "LAUNCH %s %s", name, param ? param : "");
		}
		return ret; 
	}else{
		skynet_error(ctx, "FAILED launch %s", name);
		int handle = ctx->handle; 
		skynet_context_release(ctx);
		skynet_handle_retire(handle);
		struct drop_t d = { handle };
		skynet_mq_release(queue, drop_message, &d);
		return NULL;
	}
}
