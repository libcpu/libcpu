#ifndef __bsd_arc4random_h
#define __bsd_arc4random_h

uint32_t bsd_arc4random(void);
void bsd_arc4random_addrandom(uint8_t const *, size_t);
void bsd_arc4random_stir(void);
void bsd_arc4random_bytes(uint8_t *, size_t);

#endif  /* !__bsd_arc4random_h */
