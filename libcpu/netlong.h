/* Provide htonl and ntohl */
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#elif __LITTLE_ENDIAN__
static inline uint32_t htonl(uint32_t hostlong)
{
  return (hostlong >> 24 & 0x000000FFUL) |
         (hostlong >> 8  & 0x0000FF00UL) |
         (hostlong << 8  & 0x00FF0000UL) |
         (hostlong << 24 & 0xFF000000UL);
}

static inline uint32_t ntohl(uint32_t netlong)
{
  return (netlong >> 24 & 0x000000FFUL) |
         (netlong >> 8  & 0x0000FF00UL) |
         (netlong << 8  & 0x00FF0000UL) |
         (netlong << 24 & 0xFF000000UL);
}
#else
#define htonl(x) (x)
#define ntohl(x) (x)
#endif
