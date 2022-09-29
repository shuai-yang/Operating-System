
int init_infiniti(void);


void * infiniti_malloc(size_t size);
void infiniti_free(void * addr);
void infiniti_dump();
void infiniti_invlpg(void * addr);
