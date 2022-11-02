/* This is the solution to leetcode problem No.1115, print FooBar alternately.
 * author: Jidong Xiao
 * */

#include <stdio.h> /* for printf */
#include <stdlib.h> /* for malloc */
#include <string.h> /* for strcat */
#include "cthreads.h"

char str[50];

typedef struct {
    int n;
    cthread_mutex_t mutex1;
    cthread_mutex_t mutex2;
} FooBar;

void printFoo(){
		strcat(str, "foo");
}

void printBar(){
		strcat(str, "bar");
}

FooBar* fooBarCreate(int n) {
    FooBar* obj = (FooBar*) malloc(sizeof(FooBar));
    cthread_mutex_init(&obj->mutex1);
    cthread_mutex_init(&obj->mutex2);
    cthread_mutex_lock(&obj->mutex2);
    obj->n = n;
    return obj;
}

void* foo(void* orig_obj) {

	int i;
	FooBar *obj = (FooBar *)orig_obj;
    for (i = 0; i < obj->n; i++) {
        cthread_mutex_lock(&obj->mutex1);
        // printFoo() outputs "foo". Do not change or remove this line.
        printFoo();
        cthread_mutex_unlock(&obj->mutex2);
    }
	cthread_exit(NULL);
	return NULL; // returning here just avoid the compiler warning saying "warning: control reaches end of non-void function"
}

void* bar(void* orig_obj) {

	int i;
	FooBar *obj = (FooBar *)orig_obj;
    for (i = 0; i < obj->n; i++) {
        cthread_mutex_lock(&obj->mutex2);
        // printBar() outputs "bar". Do not change or remove this line.
        printBar();
        cthread_mutex_unlock(&obj->mutex1);
    }
	cthread_exit(NULL);
	return NULL; // returning here just avoid the compiler warning saying "warning: control reaches end of non-void function"
}

void fooBarFree(FooBar* obj) {
	/* unliked the leetcode problem, here we do not need to call mutex destroy function. */
    free(obj);
}

int main(int argc, char *argv[]) {
    cthread_t tids[2];
	int n = 10;

	FooBar *obj = fooBarCreate(n);

	cthread_create(&tids[0], foo, (void *)obj);
	cthread_create(&tids[1], bar, (void *)obj);

	cthread_join(tids[0], NULL);
	cthread_join(tids[1], NULL);

	fooBarFree(obj);

    printf("main: output \"foobar\" %d times in a row:\n", n);
	printf("%s\n", str);
    printf("main: exiting\n");

    return 0;
}

/* vim: set ts=4: */
