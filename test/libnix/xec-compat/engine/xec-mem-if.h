/*
 * XEC - Optimizing Dynarec Engine
 *
 * Guest Memory COM Interface
 * Copyright (C) 2007 Orlando Bassotto. All rights reserved.
 * Copyright (C) 2007 Gianluca Guida. All rights reserved.
 * 
 * $Id: xec-code-block.h 125 2007-06-14 00:38:57Z orlando $
 */

#ifndef __xec_mem_if_h
#define __xec_mem_if_h

#include "xec-base.h"

typedef unsigned long xec_mem_flg_t;

typedef enum _xec_mem_err
  {
    XEC_MEM_PADDR = 0,
    XEC_MEM_VADDR = 1,

    XEC_MEM_INVALID = 0x10,
    XEC_MEM_LOAD = 0x20,
    XEC_MEM_FETCH = 0x40,
    XEC_MEM_STORE = 0x80,

    XEC_MEM_UNALIGNED = 0x100,
    XEC_MEM_NOT_PRESENT = 0x200,
    XEC_MEM_PRIVILEGE = 0x400,
    XEC_MEM_PERMISSION = 0x800,

    /* GLC: Aren't these *INPUT* flags? */
    XEC_MEM_RING0 = 0x1000000,
    XEC_MEM_RING1 = 0x2000000,
    XEC_MEM_RING2 = 0x3000000,
    XEC_MEM_RING3 = 0x4000000,
    XEC_MEM_RING_MASK = 0xf000000
  } xec_mem_err_t;

typedef struct _xec_mem_if xec_mem_if_t;

struct _xec_mem_if_vtbl
  {
    xec_gaddr_t   (*gmap)(xec_mem_if_t *self, xec_haddr_t addr, size_t len, unsigned flags);

    xec_gaddr_t   (*htog)(xec_mem_if_t *self, xec_haddr_t addr, xec_mem_flg_t *mf);
    xec_haddr_t   (*gtoh)(xec_mem_if_t *self, xec_gaddr_t addr, xec_mem_flg_t *mf);

    xec_mem_flg_t (*read)(xec_mem_if_t *self, xec_gaddr_t gaddr, uint8_t *buf, size_t sz);
    xec_mem_flg_t (*write)(xec_mem_if_t *self, xec_gaddr_t addr, uint8_t const *buf, size_t sz);
  };

struct _xec_mem_if
  {
    struct _xec_mem_if_vtbl const *vtbl;
  };

#define xec_mem_read(self, addr, buf, sz) \
  (self)->vtbl->read (self, addr, buf, sz)

#define xec_mem_write(self, addr, buf, sz) \
  (self)->vtbl->write (self, addr, buf, sz)

#define xec_mem_gtoh(self, addr, mf) \
  (self)->vtbl->gtoh (self, addr, mf)

#define xec_mem_htog(self, addr, mf) \
  (self)->vtbl->htog (self, addr, mf)

#define xec_mem_gmap(self, ...) \
  (self)->vtbl->gmap (self, __VA_ARGS__)

#endif /* !__xec_mem_if_h */
