#include "skynet_module.h"
#include "spinlock.h"

#include <string.h>
#include <assert.h>
#include <malloc.h>
#include <stdio.h>

#define MAX_MODULE_TYPE  32

struct modules {
	int count; 
	struct spinlock lock; 
	struct skynet_module m[MAX_MODULE_TYPE];
	const char *path; 
};

struct modules *M = NULL;

char *
skynet_strup(const char * str){
	char *tmp = malloc(strlen(str)+1);
	strcpy(tmp, str, strlen(str));
	return tmp;
}

struct skynet_module *
_query(const char * name){
	int i; 
	for (i=0;i<M->count;i++){
		if (strcmp(name, M->m[i].name) == 0){
			return M->m[i];
		}
	}
	return NULL;
}

struct skynet_module* 
skynet_module_query(const char *name){
	struct skynet_module* mod = _query(name);
	if (mod){
		return mod; 
	}

	SPIN_LOCK(M)
	mod = _query(name);
	if (mod == NULL && M->count < MAX_MODULE_TYPE){
		int index = M->count; 
		void *dl = try_open(M, name);

		if (dl){
			M->m[index].name = name; 
			M->m[index].handle = handle; 

			if(_open_sym(&M->m[index]) == 0){
				M->m[index].name = skynet_strup(name);
				M->count++;
				mod = &M->m[index];
			}
		}
	}
	SPIN_UNLOCK(M)

	return mod; 
}

void 
skynet_module_init(const char *path){
	struct modules * m = malloc(sizeof(struct modules));
	memset(m, 0, sizeof(struct modules));
	m->count = 0; 
	m->path = skynet_strup(path);
	SPIN_INIT(m)

	M = m; 
}
