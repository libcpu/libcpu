#include "xec-debug.h"
#include "sc2int.h"

int
yywrap (void)
{
  return 1;
}

void
yyerror (char const *msg)
{
  XEC_LOG (NULL, XEC_LOG_FATAL, XEC_LOG_ERREXIT, "%s in file `%s' line %u.", msg, g_filename, g_line);
}
