/*
 * XEC - Optimizing Dynarec Engine
 *
 * Exception and Unix Signal Handling
 * Copyright (C) 2007 Orlando Bassotto. All rights reserved.
 * 
 * $Id: xec-xcpt.h 310 2007-06-30 23:21:01Z orlando $
 */
#ifndef __xec_xcpt_h
#define __xec_xcpt_h

#include "xec-base.h"

typedef int (*xec_xcpt_handler_t)(int, void *);

xec_xcpt_handler_t
xec_xcpt_set_handler
  (
    xec_xcpt_handler_t handler
  );

#endif  /* !__xec_xcpt_h */
