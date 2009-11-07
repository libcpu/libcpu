PROJECT(xec-compat)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

INCLUDE(CheckIncludeFiles)

CHECK_INCLUDE_FILES(sys/queue.h __XEC_HAVE_SYS_QUEUE_H)

IF(WIN32)
    SET(MMAP_SOURCE lib/xec-mmap-win32.c)
    SET(XCPT_SOURCE lib/xec-xcpt-win32.c)
ELSE()
    SET(MMAP_SOURCE lib/xec-mmap-unix.c)
    SET(XCPT_SOURCE lib/xec-xcpt-unix.c)
ENDIF()

INCLUDE_DIRECTORIES(lib engine arch ${PROJECT_BINARY_DIR})
CONFIGURE_FILE(xec-base.h.cmake ${PROJECT_BINARY_DIR}/xec-base.h)

ADD_LIBRARY(xec-compat SHARED lib/xec-log.c lib/xec-mem.c ${MMAP_SOURCE} ${XCPT_SOURCE}
    engine/xec-us-syscall.c engine/xec-monitor.c)
