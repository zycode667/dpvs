/*
 * Soft:        Keepalived is a failover program for the LVS project
 *              <www.linuxvirtualserver.org>. It monitor & manipulate
 *              a loadbalanced server pool using multi-layer checks.
 *
 * Part:        align.h include file.
 *
 * Author:      Quentin Armitage <quentin@armitage.org.uk>
 *
 *              This program is distributed in the hope that it will be useful,
 *              but WITHOUT ANY WARRANTY; without even the implied warranty of
 *              MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *              See the GNU General Public License for more details.
 *
 *              This program is free software; you can redistribute it and/or
 *              modify it under the terms of the GNU General Public License
 *              as published by the Free Software Foundation; either version
 *              2 of the License, or (at your option) any later version.
 *
 * Copyright (C) 2020-2020 Alexandre Cassen, <acassen@gmail.com>
 */

#ifndef _ALIGN_H
#define _ALIGN_H

#include "config.h"

#ifdef CHECK_CAST_ALIGN
#include "logger.h"
#endif

/* PTR_CAST and PTR_CAST_CONST should be used for all casts of pointers.
 *
 * PTR_CAST and PTR_CAST_CONST serve several purposes.
 *
 * 1) On 32 bit ARM systems which don't support unaligned memory access, configure
 *    will have defined CAST_VIA_VOID to avoid the compiler spewing out 1000s of
 *    "cast increases required alignment of target type" warnings which are caused
 *    due to the char * used in the cast possibly not being aligned for the pointer
 *    being cast to. CAST_VIA_VOID merely means that the char * is first cast to a
 *    void * which is then cast to the pointer of the type required. Casting via a
 *    void * should not alter the code produced by the compiler, since the initial
 *    pointer (char *) only has 1 byte alignment.
 *
 *    This still leaves the problem that, if the keepalived code is not correct, there
 *    may be an unaligned pointer being used. See 2) below for how this is dealt with.
 *
 *    On systems which do allow unaligned memory access, the warnings generated by
 *    not using a void * can be generated by using configure options:
 *      --enable-strict-cast-align --disable-cast-via-void
 *
 * 2) As identified in 1) above, there is a need to be able to ensure that there
 *    are no unaligned casts, both for performance reasons on sytems which do allow
 *    unaligned casts, and to ensure that there are not alignment traps, or worse
 *    still incorrect values returned (which happens with ARMv5) from unaligned reads.
 *
 *    For this reason there is a configure option --enable-cast-align-checks which
 *    defines CHECK_CAST_ALIGN. This causes PTR_CAST and PTR_CAST_CONST to generate
 *    run-time code to check that casts made via PTR_CAST and PRT_CAST_CONST are
 *    properly aligned, and logs a message if they are not. The checks work on any
 *    architecture, whether unaligned memory access works or not, and so can be
 *    performed on Intel x86_64, aarch64 etc.
 *
 *    Developers should periodically build with this option enabled and then run
 *    keepalived to check that there are no unaligned casts. 22 such instances of
 *    unaligned char arrays being cast to structure pointers with greater alignment
 *    were found when this check was first added.
 *
 * 3) Other cast checks can be added later by simply adding further definitions for
 *    PTR_CAST and PTR_CAST_CONST, probably just by adding a further definition of
 *    PTR_CAST_ALL.
 */

#ifdef CAST_VIA_VOID
#define __CAST_PTR(__const)		(__const void *)
#define PTR_CAST_ASSIGN			(void *)
#define PTR_CAST_ASSIGN_CONST		(const void *)
#else
#define __CAST_PTR(__const)
#define PTR_CAST_ASSIGN
#define PTR_CAST_ASSIGN_CONST
#endif

#ifdef CHECK_CAST_ALIGN
#define PTR_CAST_ALL(__type, __ptr, __const) ({				\
		__const void *sav_ptr = __ptr;			\
		if ((long)sav_ptr % __alignof__(__type))		\
			log_message(LOG_INFO, "Alignment error - (" #__type " *)(" #__ptr ") - alignment %zu, address %p", __alignof__(__type), sav_ptr); \
		(__const __type *) __CAST_PTR(__const) (sav_ptr);	\
		})

#define PTR_CAST2_ALL(__type, __type1, __ptr, __field, __const) ({	\
		__const void *sav_ptr1 = __ptr;			\
		if ((long)sav_ptr1 % __alignof__(__type1))		\
			printf("Alignment error - (" #__type1 " *)(" #__ptr ") - alignment %zu, address %p", __alignof__(__type1), sav_ptr1); \
		PTR_CAST_ALL(__type, &(((__const __type1 *) __CAST_PTR(__const) (sav_ptr1))->__field), __const);\
		})
#else
#define PTR_CAST_ALL(__type, __ptr, __const) \
		({ (__const __type *) __CAST_PTR(__const) (__ptr); })
#define PTR_CAST2_ALL(__type, __type1, __ptr, __field, __const) \
		({ (__const __type *) __CAST_PTR(__const) &((__const __type1 *) __CAST_PTR(__const) (__ptr))->__field; })
#endif

#define PTR_CAST(__type, __ptr)					PTR_CAST_ALL(__type, __ptr,)
#define PTR_CAST_CONST(__type, __ptr)				PTR_CAST_ALL(__type, __ptr, const)

#define PTR_CAST2(__type, __type1, __ptr, __field)		PTR_CAST2_ALL(__type, __type1, __ptr, __field,)
#define PTR_CAST2_CONST(__type, __type1, __ptr, __field)	PTR_CAST2_ALL(__type, __type1, __ptr, __field, const)

#endif