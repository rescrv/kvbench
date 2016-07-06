// Copyright (c) 2014-2016, Robert Escriva
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

#define __STDC_LIMIT_MACROS

// C
#include <limits.h>
#include <stdio.h>
#include <unistd.h>

// po6
#include <po6/errno.h>
#include <po6/threads/mutex.h>

// kvbench
#include "database.h"

class database_fwrite : public database
{
    public:
        database_fwrite();
        ~database_fwrite() throw ();

    public:
        virtual const e::argparser& parser();
        virtual bool setup(const char* prefix);
        virtual bool teardown();

        virtual bool get(void* ptr, const char* key, size_t key_sz);
        virtual bool put(void* ptr,
                         const char* key, size_t key_sz,
                         const char* val, size_t val_sz);
        virtual bool del(void* ptr, const char* key, size_t key_sz);
        virtual bool scan(void* ptr, const char* key, size_t key_sz, size_t num);

    private:
        e::argparser m_ap;
        bool m_fsync;
        po6::threads::mutex m_mtx;
        FILE* m_file;

        database_fwrite(const database_fwrite&);
        database_fwrite& operator = (const database_fwrite&);
};

database_fwrite :: database_fwrite()
    : m_ap()
    , m_fsync(false)
    , m_mtx()
    , m_file(NULL)
{
    m_ap.arg().long_name("fsync")
              .description("perform an fsync after each write (default: no)")
              .set_true(&m_fsync);
}

database_fwrite :: ~database_fwrite() throw ()
{
}

const e::argparser&
database_fwrite :: parser()
{
    return m_ap;
}

bool
database_fwrite :: setup(const char* prefix)
{
    po6::threads::mutex::hold hold(&m_mtx);
    std::string path = prefix;
    path += "/file.dat";
    m_file = fopen(path.c_str(), "w+");

    if (!m_file || ferror(m_file))
    {
        perror("fwrite benchmark failed");
        return false;
    }

    return true;
}

bool
database_fwrite :: teardown()
{
    po6::threads::mutex::hold hold(&m_mtx);

    if (m_file)
    {
        fclose(m_file);
        m_file = NULL;
    }

    return true;
}

bool
database_fwrite :: get(void* ptr, const char* key, size_t key_sz)
{
    abort();
    (void) ptr;
    (void) key;
    (void) key_sz;
}

bool
database_fwrite :: put(void* ptr,
                       const char* key, size_t key_sz,
                       const char* val, size_t val_sz)
{
    int fd = -1;

    {
        po6::threads::mutex::hold hold(&m_mtx);

        if (key_sz > INT_MAX || val_sz > INT_MAX ||
            fwrite(key, 1, key_sz, m_file) != key_sz ||
            fwrite(val, 1, val_sz, m_file) != val_sz)
        {
            perror("fwrite benchmark failed");
            return false;
        }

        if (m_fsync && fflush(m_file))
        {
            perror("fwrite benchmark failed");
            return false;
        }

        fd = fileno(m_file);
    }

    if (m_fsync && fsync(fd) < 0)
    {
        perror("fwrite benchmark failed");
        return false;
    }

    return true;
    (void) ptr;
}

bool
database_fwrite :: del(void* ptr, const char* key, size_t key_sz)
{
    abort();
    (void) ptr;
    (void) key;
    (void) key_sz;
}

bool
database_fwrite :: scan(void* ptr, const char* key, size_t key_sz, size_t num)
{
    abort();
    (void) ptr;
    (void) key;
    (void) key_sz;
    (void) num;
}

database*
database::create()
{
    return new database_fwrite();
}
