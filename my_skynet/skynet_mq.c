#include "skynet_mq.h"
#include "spinlock.h"

#include <malloc.h>
#include <string.h>
#include <assert.h>

#define MQ_BUF_SIZE 64
#define MQ_IN_GLOBAL 1
#define MQ_OVERLOAD_THREASHOLD  64

struct skynet_message {
	int source; 
	int session; 
	void *data; 
	int size; 
};

struct message_queue {
	struct spinlock_t lock; 
	int head;
	int tail; 
	int cap;
	int handle; 
	int in_global; 
	int release; 
	int overload; 
	int overload_threashold;
	struct skynet_message *queue; 
	struct message_queue * next; 
};

struct global_queue{
	struct spinlock_t lock; 
	struct message_queue *head; 
	struct message_queue *tail; 
};

struct global_queue *Q = NULL;

void 
skynet_gloablmq_push(struct message_queue *mq){
	assert(mq);
	struct global_queue *q = Q; 
	SPIN_LOCK(q)
	if (q->head){
		q->tail->next = mq;
		q->tail = q->tail->next;
	}else{
		assert(q->tail == NULL);
		q->head = q->tail = mq; 
	}
	q->tail->next = NULL;
	SPIN_UNLOCK(q)
}

struct message_queue * 
skynet_globalmq_pop(){
	struct global_queue *q = Q; 
	SPIN_LOCK(q)
	struct message_queue *mq = q->head; 
	if (mq){
		q->head = q->head->next;
		mq->next = NULL; 
		if (q->head == NULL){
			assert(q->tail == NULL);
		}
	}
	SPIN_UNLOCK(q)
	return mq; 
}

struct message_queue* 
skynet_mq_create(int handle){
	struct message_queue *mq = malloc(sizeof(*mq));
	memset(mq, 0, sizeof(*mq));
	mq->handle = handle; 
	SPIN_INIT(mq)
	mq->head = 0; 
	mq->tail = 0; 
	mq->cap = MQ_BUF_SIZE;
	mq->in_global = 0;
	mq->release = 0;
	mq->overload = 0; 
	mq->overload_threashold = MQ_OVERLOAD_THREASHOLD;
	mq->queue = malloc(sizeof(struct skynet_message*) * MQ_BUF_SIZE);
	mq->next = NULL;

	return mq; 
}

void
expand_queue(struct message_queue* mq){
	struct skynet_message *new_queue = malloc(sizeof(struct skynet_message*) * mq->cap * 2);
	memset(new_queue, 0, sizeof(struct skynet_message*) * mq->cap * 2);
	int i; 
	for(i=0; i<mq->cap;i++){
		mq->new_queue[i] = mq->queue[(mq->head+i)%mq->cap];
	}
	mq->head = 0;
	mq->tail = mq->cap; 
	mq->cap = mq->cap * 2; 
	free(mq->queue);
	mq->queue = new_queue;
}

void 
skynet_mq_push(struct message_queue*mq, struct skynet_message* message){
	SPIN_LOCK(mq)
	mq->queue[mq->tail++] = *message; 
	if (mq->tail >= mq->cap){
		mq->tail = 0;
	}
	if (mq->tail == mq->head){
		expand_queue(mq);
	}
	SPIN_UNLOCK(mq)
}

struct skynet_message* 
skynet_mq_pop(struct message_queue * mq){
	SPIN_LOCK(mq)
	struct skynt_message * message = NULL; 
	if (mq->head != mq->tail){
		*message = mq->queue[mq->head++];
		if (mq->head >= mq->cap){
			mq->head = 0; 
		}

		int length = mq->tail - mq->head;
		if (length < 0){
			//length = mq->cap - (mq->head - mq->tail);
			length += mq->cap;
		} 

		while(length > mq->overload_threashold){
			mq->overload_threashold *= 2; 
			mq->overload = 1;
		}

	}else{
		mq->overload = 0; 
		mq->overload_threashold = MQ_OVERLOAD_THREASHOLD;
	}

	if (mq->in_global == 0){
		mq->in_global = MQ_IN_GLOBAL;
	}
	SPIN_UNLOCK(mq)
	return message;
}

int 
skynet_mq_length(struct message_queue* mq){
	SPIN_LOCK(mq)
	int length = mq->tail - mq->head;
	if (length < 0){
		length += mq->cap;
	}
	SPIN_UNLOCK(mq)
	return length;
}

int 
skynet_mq_handle(struct message_queue* mq){
	SPIN_LOCK(mq)
	int handle = mq->handle;
	SPIN_UNLOCK(mq)
	return handle; 
}

void 
skynet_mq_release(struct message_queue *mq){
	
}

void
skynet_globalmq_init(){
	struct global_queue * q = malloc(sizeof(*q));
	memset(q, 0, sizeof(*q));
	SPIN_INIT(q)
	q->head = NULL; 
	q->tail = NULL;
	Q = q;
}