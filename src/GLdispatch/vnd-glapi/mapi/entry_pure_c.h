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

#include <assert.h>
#include <stdlib.h>

static INLINE const struct mapi_table *
entry_current_get(void)
{
   return u_current_get();
}

/* C version of the public entries */
#define MAPI_TMP_DEFINES
#define MAPI_TMP_PUBLIC_DECLARES
#define MAPI_TMP_PUBLIC_ENTRIES
#include "mapi_tmp.h"

const int entry_type = ENTRY_PURE_C;
const int entry_stub_size = 0;

void
entry_init_public(void)
{
}

void
entry_generate_default_code(char *entry, int slot)
{
    assert(0);
}

mapi_func
entry_get_public(int slot)
{
   /* pubic_entries are defined by MAPI_TMP_PUBLIC_ENTRIES */
   return public_entries[slot];
}

#if !defined(STATIC_DISPATCH_ONLY)
mapi_func
entry_generate(int slot)
{
   return NULL;
}

void
entry_patch(mapi_func entry, int slot)
{
}
#endif // !defined(STATIC_DISPATCH_ONLY)
