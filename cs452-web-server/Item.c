
#include "Item.h"

struct item *createItem(int fd, int producer) 
{
	struct item *item = (struct item *) malloc(sizeof(struct item));
	item->fd = fd; // this gets filled in by the pool
	item->producer = producer;
	return item;
}

int compareToItem(const void *obj1, const void *obj2)
{
	return ((struct item *)obj1)->fd - ((struct item *)obj2)->fd;
}

#define ITEM_STRING_SIZE 128

char *toStringItem(const void *obj)
{
	struct item * item = (struct item *) obj;
	char *buffer = (char *) malloc(sizeof(char)*ITEM_STRING_SIZE);
	sprintf(buffer, "%d from producer %d ", item->fd, item->producer);
	return buffer;
}

void freeItem(void *obj) 
{
	free((struct item *) obj);
}
