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

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "u_current.h"
#include "entry.h"
#include "stub.h"
#include "table.h"

#if !defined(STATIC_DISPATCH_ONLY)
#include "u_thread.h"
#endif

#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#define MAPI_LAST_SLOT (MAPI_TABLE_NUM_STATIC + MAPI_TABLE_NUM_DYNAMIC - 1)

struct mapi_stub {
   /*!
    * The name of a static stub function. This isn't really a pointer, it's an
    * offset into public_string_pool.
    */
   const void *nameOffset;

   int slot;
   mapi_func addr;

   /**
    * The name of the function. This is only used for dynamic stubs. For static
    * stubs, nameOffset is used instead.
    */
   char *nameBuffer;
};

/* define public_string_pool and public_stubs */
#define MAPI_TMP_PUBLIC_STUBS
#include "mapi_tmp.h"

static int
stub_compare(const void *key, const void *elem)
{
   const char *name = (const char *) key;
   const struct mapi_stub *stub = (const struct mapi_stub *) elem;
   const char *stub_name;

   stub_name = &public_string_pool[(unsigned long) stub->nameOffset];

   return strcmp(name, stub_name);
}

/**
 * Return the public stub with the given name.
 */
const struct mapi_stub *
stub_find_public(const char *name)
{
   /* Public entry points are stored without their 'gl' prefix */
   if (name[0] == 'g' && name[1] == 'l') {
       name += 2;
   }

   return (const struct mapi_stub *) bsearch(name, public_stubs,
         ARRAY_SIZE(public_stubs), sizeof(public_stubs[0]), stub_compare);
}

#if !defined(STATIC_DISPATCH_ONLY)
static struct mapi_stub dynamic_stubs[MAPI_TABLE_NUM_DYNAMIC];
static int num_dynamic_stubs;
static int next_dynamic_slot = MAPI_TABLE_NUM_STATIC;

void stub_cleanup_dynamic(void)
{
    int i;

    // TODO: Free the memory for the generated stub functions.

    // Free the copies of the stub names.
    for (i=0; i<num_dynamic_stubs; i++) {
        struct mapi_stub *stub = &dynamic_stubs[i];
        free(stub->nameBuffer);
        stub->nameBuffer = NULL;
    }

    num_dynamic_stubs = 0;
    next_dynamic_slot = MAPI_TABLE_NUM_STATIC;
}

/**
 * Add a dynamic stub.
 */
static struct mapi_stub *
stub_add_dynamic(const char *name)
{
   struct mapi_stub *stub;
   int idx;

   idx = num_dynamic_stubs;
   /* minus 1 to make sure we can never reach the last slot */
   if (idx >= MAPI_TABLE_NUM_DYNAMIC - 1)
      return NULL;

   stub = &dynamic_stubs[idx];

   /*
    * name is the pointer passed to glXGetProcAddress, so the caller may free
    * or modify it later. Allocate a copy of the name to store.
    */
   stub->nameBuffer = strdup(name);
   if (stub->nameBuffer == NULL) {
       return NULL;
   }

   /* dispatch to the last slot, which is reserved for no-op */
   stub->addr = entry_generate(MAPI_LAST_SLOT);
   if (!stub->addr) {
      free(stub->nameBuffer);
      stub->nameBuffer = NULL;
      return NULL;
   }

   stub->nameOffset = NULL;
   /* to be fixed later */
   stub->slot = -1;

   num_dynamic_stubs = idx + 1;

   return stub;
}

/**
 * Return the dynamic stub with the given name.  If no such stub exists and
 * generate is true, a new stub is generated.
 */
struct mapi_stub *
stub_find_dynamic(const char *name, int generate)
{
   u_mutex_declare_static(dynamic_mutex);
   struct mapi_stub *stub = NULL;
   int count, i;
   
   u_mutex_lock(dynamic_mutex);

   if (generate)
      assert(!stub_find_public(name));

   count = num_dynamic_stubs;
   for (i = 0; i < count; i++) {
      if (strcmp(name, (const char *) dynamic_stubs[i].nameBuffer) == 0) {
         stub = &dynamic_stubs[i];
         break;
      }
   }

   /* generate a dynamic stub */
   if (generate && !stub)
         stub = stub_add_dynamic(name);

   u_mutex_unlock(dynamic_mutex);

   return stub;
}

static const struct mapi_stub *
search_table_by_slot(const struct mapi_stub *table, size_t num_entries,
                     int slot)
{
   size_t i;
   for (i = 0; i < num_entries; ++i) {
      if (table[i].slot == slot)
         return &table[i];
   }
   return NULL;
}

const struct mapi_stub *
stub_find_by_slot(int slot)
{
   const struct mapi_stub *stub =
      search_table_by_slot(public_stubs, ARRAY_SIZE(public_stubs), slot);
   if (stub)
      return stub;
   return search_table_by_slot(dynamic_stubs, num_dynamic_stubs, slot);
}

void
stub_fix_dynamic(struct mapi_stub *stub, const struct mapi_stub *alias)
{
   int slot;

   if (stub->slot >= 0)
      return;

   if (alias)
      slot = alias->slot;
   else
      slot = next_dynamic_slot++;

   entry_patch(stub->addr, slot);
   stub->slot = slot;
}
#endif // !defined(STATIC_DISPATCH_ONLY)

/**
 * Return the name of a stub.
 */
const char *
stub_get_name(const struct mapi_stub *stub)
{
   const char *name;

   if (stub >= public_stubs &&
       stub < public_stubs + ARRAY_SIZE(public_stubs)) {
      name = &public_string_pool[(unsigned long) stub->nameOffset];
   } else {
      name = stub->nameBuffer;
   }

   return name;
}

/**
 * Return the slot of a stub.
 */
int
stub_get_slot(const struct mapi_stub *stub)
{
   return stub->slot;
}

/**
 * Return the address of a stub.
 */
mapi_func
stub_get_addr(const struct mapi_stub *stub)
{
   assert(stub->addr || (unsigned int) stub->slot < MAPI_TABLE_NUM_STATIC);
   return (stub->addr) ? stub->addr : entry_get_public(stub->slot);
}

static int stub_allow_override(void)
{
    return !!entry_stub_size;
}

static GLboolean stubStartPatch(void)
{
    if (!stub_allow_override())
    {
        return GL_FALSE;
    }

    // Nothing else to do yet.
    return GL_TRUE;
}

static void stubFinishPatch(void)
{
    // Nothing else to do yet.
}

static void stubRestoreFuncs(void)
{
    int i, slot;
    const struct mapi_stub *stub;

    assert(stub_allow_override());

    for (stub = public_stubs, i = 0;
         i < MAPI_TABLE_NUM_STATIC;
         stub++, i++) {
        slot = (stub->slot == -1) ? MAPI_LAST_SLOT : stub->slot;
        entry_generate_default_code((char *)stub_get_addr(stub), slot);
    }

#if !defined(STATIC_DISPATCH_ONLY)
    for (stub = dynamic_stubs, i = 0;
         i < num_dynamic_stubs;
         stub++, i++) {
        slot = (stub->slot == -1) ? MAPI_LAST_SLOT : stub->slot;
        entry_generate_default_code((char *)stub_get_addr(stub), slot);
    }
#endif // !defined(STATIC_DISPATCH_ONLY)
}

static void stubAbortPatch(void)
{
    stubRestoreFuncs();
}

static GLboolean stubGetPatchOffset(const char *name, void **writePtr, const void **execPtr)
{
    const struct mapi_stub *stub;
    void *addr = NULL;

    stub = stub_find_public(name);

#if !defined(STATIC_DISPATCH_ONLY)
    if (!stub) {
        stub = stub_find_dynamic(name, 0);
    }
#endif // !defined(STATIC_DISPATCH_ONLY)

    if (stub) {
        addr = stub_get_addr(stub);
    }
    if (writePtr != NULL) {
        *writePtr = addr;
    }
    if (execPtr != NULL) {
        *execPtr = addr;
    }

    return (addr != NULL ? GL_TRUE : GL_FALSE);
}

static int stubGetStubType(void)
{
    return entry_type;
}

static int stubGetStubSize(void)
{
    return entry_stub_size;
}

static const __GLdispatchStubPatchCallbacks stubPatchCallbacks =
{
    stubStartPatch,     // startPatch
    stubFinishPatch,    // finishPatch
    stubAbortPatch,     // abortPatch
    stubRestoreFuncs,   // restoreFuncs
    stubGetPatchOffset, // getPatchOffset
    stubGetStubType,    // getStubType
    stubGetStubSize,    // getStubSize
};

const __GLdispatchStubPatchCallbacks *stub_get_patch_callbacks(void)
{
    if (stub_allow_override())
    {
        return &stubPatchCallbacks;
    }
    else
    {
        return NULL;
    }
}

