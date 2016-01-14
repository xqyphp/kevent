//
//  types.h
//  klib
//
//  Created by LiKai on 16/1/8.
//  Copyright © 2016年 LiKai. All rights reserved.
//

#ifndef ktypes_h
#define ktypes_h

#include <stddef.h>

typedef unsigned       bool_t;
typedef int            status_t;

#define K_SUCCESS      0
#define K_ERROR       -1
#define K_TRUE         1
#define K_FALSE        0
#define K_NULL         ((void*)0)

typedef int            (*compare_t)(void* left,void* right);
typedef void*          (*getkey_t)(void* rbnode);

#ifndef errno_t
typedef int            errno_t;
#endif

#endif /* ktypes_h */
