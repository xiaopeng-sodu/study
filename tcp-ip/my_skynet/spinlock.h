#ifndef spin_lock_t
#define spin_lock_t

#define SPIN_INIT(q)  spinlock_init(&((q)->lock))
#define SPIN_LOCK(q)  spinlock_lock(&((q)->lock))
#define SPIN_UNLOCK(q)  spinlock_unlock(&((q)->lock))
#define SPIN_TRYLOCK(q)  spinlock_trylock(&((q)->lock))
#define SPIN_DESTROY(q)  spinlock_destroy((&(q)->lock))

struct spinlock_t {
	int lock; 
};

static void
spinlock_init(struct spinlock_t * lock){
	lock->lock = 0;
}

static void
spinlock_lock(struct spinlock_t * lock){
	while(__sync_lock_test_and_set(&lock->lock, 1)){}
}

static void
spinlock_unlock(struct spinlock_t * lock){
	__sync_lock_release(&lock->lock);
}

static void
spinlock_destroy(struct spinlock_t * lock){
	(void)(&lock->lock);
}

static void
spinlock_trylock(struct spinlock_t * lock){
	if(__sync_lock_test_and_set(&lock->lock, 1)){
		return 0;
	}
	return 1; 
}

#endif