#! /bin/sh

system=`uname -s`

case $system in
  Darwin | FreeBSD | NetBSD | OpenBSD | DragonFly)
    ncpu=`sysctl -n hw.ncpu 2>/dev/null`
    ;;
  SunOS)
    ncpu=`psrinfo -p`
    ;;
  Linux)
    ncpu=`cat /proc/cpuinfo | grep '^processor' | wc -l`
    ;;
  *)
    ;;
esac

if [ -z "$ncpu" ]; then 
  ncpu=1
fi

echo $ncpu
