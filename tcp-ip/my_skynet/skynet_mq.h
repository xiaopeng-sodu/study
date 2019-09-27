#ifndef skynet_mq_h
#define skynet_mq_h

void skynet_globalmq_init();
void skynet_gloablmq_push(struct message_queue *mq);
struct message_queue * skynet_globalmq_pop();
struct message_queue* skynet_mq_create(int handle);
void skynet_mq_push(struct message_queue * mq, struct skynet_message* message);
struct skynet_message* skynet_mq_pop(struct message_queue * mq);
int skynet_mq_length(struct message_queue* mq);
int skynet_mq_handle(struct message_queue* mq);
void skynet_mq_release(struct message_queue *mq);

#endif