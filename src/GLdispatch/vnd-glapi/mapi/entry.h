/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2010 LunarG Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Chia-I Wu <olv@lunarg.com>
 */

#ifndef _ENTRY_H_
#define _ENTRY_H_

#include "u_compiler.h"

typedef void (*mapi_func)(void);

enum {
    ENTRY_X86_TLS,
    ENTRY_X86_64_TLS,
    ENTRY_X86_TSD,
    ENTRY_PURE_C,
    ENTRY_X86_64_TSD,
    ENTRY_ARMV7_THUMB_TSD,
    ENTRY_NUM_TYPES
};

extern const int entry_type;
extern const int entry_stub_size;

void
entry_init_public(void);

mapi_func
entry_get_public(int slot);

mapi_func
entry_generate(int slot);

void
entry_generate_default_code(char *entry, int slot);

void
entry_patch(mapi_func entry, int slot);

#endif /* _ENTRY_H_ */
