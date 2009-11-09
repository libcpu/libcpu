#ifndef __lists_h
#define __lists_h

#include "xec-param.h"

typedef struct _param
  {
    TAILQ_ENTRY (_param) link;
    bool                 ellipsis;
    xec_param_type_t     type;
  } param_t;

typedef TAILQ_HEAD (_param_list, _param) param_list_t;

typedef struct _call
  {
    TAILQ_ENTRY (_call)  link;
    int                  scno;
    char                *name;
    param_t             *rettype;
    param_list_t        *params;
  } call_t;

typedef TAILQ_HEAD (_call_list, _call) call_list_t;

param_list_t *
param_list_new (param_t *param);

param_list_t *
param_list_link (param_list_t *list,
                 param_t      *param);

size_t
param_list_count (param_list_t *pl);

param_t *
param_new (xec_param_type_t type);

param_t *
param_new_ellipsis (void);

call_list_t *
call_list_new (call_t *call);

call_list_t *
call_list_link (call_list_t *list,
                call_t      *call);

call_t *
call_new (int           scno,
          char         *name,
          param_t      *rettype,
          param_list_t *params);

#endif  /* !__lists_h */
