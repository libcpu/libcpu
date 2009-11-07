PROJECT(obsd41)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

INCLUDE(CheckIncludeFiles)

CHECK_INCLUDE_FILES(sys/sysctl.h HAVE_SYS_SYSCTL_H)
CHECK_INCLUDE_FILES(linux/sysctl.h HAVE_LINUX_SYSCTL_H)

# For the time being
SET(GUEST m88k)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR})
INCLUDE_DIRECTORIES(arch/${GUEST})

CONFIGURE_FILE(nix-obsd41-config.h.cmake ${PROJECT_BINARY_DIR}/nix-obsd41-config.h)

ADD_LIBRARY(obsd41
			SHARED
      obsd41-callbacks.c
      obsd41-init.c
      obsd41-ioctl.c
      obsd41-structs.c
      obsd41-syscalls.c
      arc4random.c
      # For the time being
      arch/${GUEST}/obsd41-guest.c)

TARGET_LINK_LIBRARIES(obsd41 nix xec-compat)

