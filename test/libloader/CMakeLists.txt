PROJECT(libloader)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${LIBCPU_RUNTIME_OUTPUT_DIRECTORY})

INCLUDE(CheckIncludeFiles)

CHECK_INCLUDE_FILES(endian.h HAVE_ENDIAN_H)
CHECK_INCLUDE_FILES(sys/endian.h HAVE_SYS_ENDIAN_H)

INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})
CONFIGURE_FILE(loader_config.h.cmake ${PROJECT_BINARY_DIR}/loader_config.h)

# Generate elf32.c and elf64.c
FILE(READ "@PROJECT_SOURCE_DIR@/elf/elfXX.c" elfXX)

STRING(REPLACE "XX" "32" elf32 "${elfXX}")
STRING(REPLACE "XX" "64" elf64 "${elfXX}")

FILE(WRITE "@PROJECT_BINARY_DIR@/elf32.c" "${elf32}")
FILE(WRITE "@PROJECT_BINARY_DIR@/elf64.c" "${elf64}")

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/elf)

#
ADD_LIBRARY(libloader SHARED
            loader_image.c
            loader_init.c
            loader_iterator.c
            loader_registry.c
            elf/elf_common.c
            ${PROJECT_BINARY_DIR}/elf32.c
            ${PROJECT_BINARY_DIR}/elf64.c)
