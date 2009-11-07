#ifndef __obsd41_sysctl_h
#define __obsd41_sysctl_h

#define OBSD41_CTL_KERN 1
#  define OBSD41_KERN_OSTYPE     1
#  define OBSD41_KERN_OSRELEASE  2
#  define OBSD41_KERN_ARGMAX     8
#  define OBSD41_KERN_HOSTNAME   10
#  define OBSD41_KERN_CLOCKRATE  12
#  define OBSD41_KERN_DOMAINNAME 22
#  define OBSD41_KERN_OSVERSION  27
#  define OBSD41_KERN_ARND       37

#define OBSD41_CTL_HW 6
#  define OBSD41_HW_MACHINE  1
#  define OBSD41_HW_NCPUS    3
#  define OBSD41_HW_PAGESIZE 7

#endif  /* !__obsd41_sysctl_h */
