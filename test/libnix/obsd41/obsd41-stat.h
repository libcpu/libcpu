#ifndef __obsd41_stat_h
#define __obsd41_stat_h

#define OBSD41_S_IFMT   0170000
#define OBSD41_S_IFIFO  0010000
#define OBSD41_S_IFCHR  0020000
#define OBSD41_S_IFDIR  0040000
#define OBSD41_S_IFBLK  0060000
#define OBSD41_S_IFREG  0100000
#define OBSD41_S_IFLNK  0120000
#define OBSD41_S_IFSOCK 0140000

#define OBSD41_S_ISUID  0004000
#define OBSD41_S_ISGID  0002000
#define OBSD41_S_ISVTX  0001000

#endif  /* !__obsd41_stat_h */
