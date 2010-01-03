/* Compatibility header.
 * Emulate unistd.h on MSVC
 */
typedef int ssize_t;
#include <io.h>
#include <direct.h>
