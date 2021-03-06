/*
 * Copyright (C) 2008 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if __CRYSTAX__
#define __STDC_LIMIT_MACROS 1
#include <stdint.h> // for UINTPTR_MAX
#endif

#include <errno.h>
#include <unistd.h>

#if __LP64__
static void* __bionic_brk;
#else
void* __bionic_brk; // Accidentally exported by the NDK.
#endif

int brk(void* end_data) {
  __bionic_brk = __brk(end_data);
  if (__bionic_brk < end_data) {
    errno = ENOMEM;
    return -1;
  }
  return 0;
}

void* sbrk(ptrdiff_t increment) {
  // Initialize __bionic_brk if necessary.
  if (__bionic_brk == NULL) {
    __bionic_brk = __brk(NULL);
  }

  // Don't ask the kernel if we already know the answer.
  if (increment == 0) {
    return __bionic_brk;
  }

  // Avoid overflow.
  uintptr_t old_brk = reinterpret_cast<uintptr_t>(__bionic_brk);
  if ((increment > 0 && static_cast<uintptr_t>(increment) > (UINTPTR_MAX - old_brk)) ||
      (increment < 0 && static_cast<uintptr_t>(-increment) > old_brk)) {
    errno = ENOMEM;
    return reinterpret_cast<void*>(-1);
  }

  void* desired_brk = reinterpret_cast<void*>(old_brk + increment);
  __bionic_brk = __brk(desired_brk);
  if (__bionic_brk < desired_brk) {
    errno = ENOMEM;
    return reinterpret_cast<void*>(-1);
  }

  return reinterpret_cast<void*>(old_brk);
}
