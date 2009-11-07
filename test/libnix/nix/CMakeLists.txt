PROJECT(nix)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

INCLUDE(CheckIncludeFiles)
INCLUDE(CheckLibraryExists)
INCLUDE(CheckFunctionExists)

CHECK_INCLUDE_FILES(sys/event.h HAVE_SYS_EVENT_H)
CHECK_INCLUDE_FILES(bitstring.h HAVE_BITSTRING_H)
CHECK_INCLUDE_FILES(pthread.h HAVE_PTHREAD_H)
CHECK_INCLUDE_FILES(sched.h HAVE_SCHED_H)

# XXX check librt for clock_XXX.

CHECK_FUNCTION_EXISTS(adjtime HAVE_ADJTIME)
CHECK_FUNCTION_EXISTS(adjfreq HAVE_ADJFREQ)
CHECK_FUNCTION_EXISTS(chflags HAVE_CHFLAGS)
CHECK_FUNCTION_EXISTS(fchflags HAVE_FCHFLAGS)
CHECK_FUNCTION_EXISTS(fdatasync HAVE_FDATASYNC)
CHECK_FUNCTION_EXISTS(clock_gettime HAVE_CLOCK_GETTIME)
CHECK_FUNCTION_EXISTS(clock_getres HAVE_CLOCK_GETRES)
CHECK_FUNCTION_EXISTS(clock_settime HAVE_CLOCK_SETTIME)
CHECK_FUNCTION_EXISTS(revoke HAVE_REVOKE)
CHECK_FUNCTION_EXISTS(acct HAVE_ACCT)
CHECK_FUNCTION_EXISTS(gethostid HAVE_GETHOSTID)
CHECK_FUNCTION_EXISTS(sethostid HAVE_SETHOSTID)
CHECK_FUNCTION_EXISTS(issetugid HAVE_ISSETUGID)
CHECK_FUNCTION_EXISTS(getreuid HAVE_GETREUID)
CHECK_FUNCTION_EXISTS(getregid HAVE_GETREGID)
CHECK_FUNCTION_EXISTS(getresuid HAVE_GETRESUID)
CHECK_FUNCTION_EXISTS(getresgid HAVE_GETRESGID)
CHECK_FUNCTION_EXISTS(getfsuid HAVE_GETFSUID)
CHECK_FUNCTION_EXISTS(getfsgid HAVE_GETFSGID)
CHECK_FUNCTION_EXISTS(nice HAVE_NICE)
CHECK_FUNCTION_EXISTS(sched_yield HAVE_SCHED_YIELD)
CHECK_FUNCTION_EXISTS(pthread_yield HAVE_PTHREAD_YIELD)
IF(HAVE_SYS_EVENT_H)
	CHECK_FUNCTION_EXISTS(kqueue HAVE_KQUEUE)
ENDIF()

INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})

CONFIGURE_FILE(nix-config.h.cmake ${PROJECT_BINARY_DIR}/nix-config.h)

ADD_LIBRARY(nix
			SHARED
			nix-bsd-cred.c
			nix-bsd-dir.c
			nix-bsd-file.c
			nix-bsd-fs.c
			nix-bsd-io.c
			nix-bsd-mem.c
			nix-bsd-nfs.c
			nix-bsd-obsolete.c
			nix-bsd-process.c
			nix-bsd-signal.c
			nix-bsd-system.c
			nix-bsd-time.c
			nix-common.c
			nix-cred.c
			nix-dir.c
			nix-env.c
			nix-fd.c
			nix-file.c
			nix-hostinfo.c
			nix-hpux-cred.c
			nix-init.c
			nix-io.c
			nix-linux-cred.c
			nix-linux-file.c
			nix-linux-fs.c
			nix-linux-i386.c
			nix-linux-mem.c
			nix-linux-nfs.c
			nix-linux-process.c
			nix-linux-signal.c
			nix-linux-system.c
			nix-linux-time.c
			nix-mem.c
			nix-process.c
			nix-rt-process.c
			nix-rt-time.c
			nix-s5-msg.c
			nix-s5-sem.c
			nix-s5-shm.c
			nix-signal.c
			nix-socket.c
			nix-structs.c
			nix-time.c
			nix-xcpt.c)
TARGET_LINK_LIBRARIES(nix xec-compat)

