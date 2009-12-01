include(CheckIncludeFile)
include(CheckLibraryExists)
include(CheckSymbolExists)
include(CheckCXXSourceCompiles)

check_include_file(sys/resource.h HAVE_SYS_RESOURCE_H)
check_symbol_exists(getrusage sys/resource.h HAVE_GETRUSAGE)
check_library_exists(readline readline "" HAVE_LIBREADLINE)
check_include_file(netinet/in.h HAVE_NETINET_IN_H)

CHECK_CXX_SOURCE_COMPILES("
template <bool x> struct static_assert;
template <> struct static_assert<true> {};
struct s { char c; int i; } __attribute__((packed));
int main() {
	sizeof(static_assert< sizeof(struct s) == sizeof(char) + sizeof(int) >);
	return 0;
}
"
HAVE_ATTRIBUTE_PACKED)

CHECK_CXX_SOURCE_COMPILES("
template <bool x> struct static_assert;
template <> struct static_assert<true> {};
__pragma(pack(push, 1))
struct s { char c; int i; };
__pragma(pack(pop))
int main() {
	sizeof(static_assert< sizeof(struct s) == sizeof(char) + sizeof(int) >);
	return 0;
}
"
HAVE_PRAGMA_PACK)

CHECK_CXX_SOURCE_COMPILES("
int main() {
	struct { int i; } __attribute__((aligned(16))) x;
	return 0;
}
"
HAVE_ATTRIBUTE_ALIGNED)

CHECK_CXX_SOURCE_COMPILES("
int main() {
	struct { int i; } __declspec(align(16)) x;
	return 0;
}
"
HAVE_DECLSPEC_ALIGN)

CHECK_CXX_SOURCE_COMPILES("
__declspec(dllexport) void foo(void);
int main() {
	return 0;
}
"
HAVE_DECLSPEC_DLLEXPORT)

configure_file(libcpu/config.h.cmake libcpu/config.h)
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/libcpu)
