#ifndef __TOYOTA_H 
#define __TOYOTA_H

/*
 * Split minors in two parts
 */
#define TYPE(dev)   (MINOR(dev) >> 4)  /* high nibble */
#define NUM(dev)    (MINOR(dev) & 0xf) /* low nibble */

#endif /* __TOYOTA_H */
