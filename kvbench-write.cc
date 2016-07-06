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

// C
#include <stdio.h>

// POSIX
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

// po6
#include <po6/errno.h>
#include <po6/threads/mutex.h>

// kvbench
#include "database.h"

class database_write : public database
{
    public:
        database_write();
        ~database_write() throw ();

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
        int m_fd;

        database_write(const database_write&);
        database_write& operator = (const database_write&);
};

database_write :: database_write()
    : m_ap()
    , m_fsync(false)
    , m_mtx()
    , m_fd(-1)
{
    m_ap.arg().long_name("fsync")
              .description("perform an fsync after each write (default: no)")
              .set_true(&m_fsync);
}

database_write :: ~database_write() throw ()
{
}

const e::argparser&
database_write :: parser()
{
    return m_ap;
}

bool
database_write :: setup(const char* prefix)
{
    po6::threads::mutex::hold hold(&m_mtx);
    std::string path = prefix;
    path += "/file.dat";
    m_fd = open(path.c_str(), O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);

    if (m_fd < 0)
    {
        perror("write benchmark failed");
        return false;
    }

    return true;
}

bool
database_write :: teardown()
{
    po6::threads::mutex::hold hold(&m_mtx);

    if (m_fd >= 0)
    {
        close(m_fd);
        m_fd = -1;
    }

    return true;
}

bool
database_write :: get(void* ptr, const char* key, size_t key_sz)
{
    abort();
    (void) ptr;
    (void) key;
    (void) key_sz;
}

bool
database_write :: put(void* ptr,
                      const char* key, size_t key_sz,
                      const char* val, size_t val_sz)
{
    {
        po6::threads::mutex::hold hold(&m_mtx);

        if (write(m_fd, key, key_sz) != (ssize_t)key_sz ||
            write(m_fd, val, val_sz) != (ssize_t)val_sz)
        {
            perror("write benchmark failed");
            return false;
        }
    }

    if (m_fsync && fsync(m_fd) < 0)
    {
        perror("write benchmark failed");
        return false;
    }

    return true;
    (void) ptr;
}

bool
database_write :: del(void* ptr, const char* key, size_t key_sz)
{
    abort();
    (void) ptr;
    (void) key;
    (void) key_sz;
}

bool
database_write :: scan(void* ptr, const char* key, size_t key_sz, size_t num)
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
    return new database_write();
}
