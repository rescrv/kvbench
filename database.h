// Copyright (c) 2014, Robert Escriva
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright notice,
//       this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of this project nor the names of its contributors may
//       be used to endorse or promote products derived from this software
//       without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef kvbench_database_h_
#define kvbench_database_h_

// C
#include <stdlib.h>

// e
#include <e/popt.h>

// ygor
#include <ygor/data.h>

class database
{
    public:
        static database* create();

    public:
        database();
        virtual ~database() throw ();

    // configuration/setup/teardown
    public:
        virtual const e::argparser& parser() = 0;
        virtual bool setup(const char* prefix) = 0;
        virtual bool setup_thread(unsigned idx, void** ptr);
        virtual bool teardown_thread(void* ptr);
        virtual bool teardown() = 0;

    // benchmarked calls
    public:
        virtual bool get(void* ptr, const char* key, size_t key_sz) = 0;
        virtual bool put(void* ptr,
                         const char* key, size_t key_sz,
                         const char* val, size_t val_sz) = 0;
        virtual bool del(void* ptr, const char* key, size_t key_sz) = 0;
        virtual bool scan(void* ptr, const char* key, size_t key_sz, size_t num) = 0;
};

#endif // kvbench_database_h_
