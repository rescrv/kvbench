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

class database_write_sharded : public database
{
    public:
        database_write_sharded();
        ~database_write_sharded() throw ();

    public:
        virtual const e::argparser& parser();
        virtual bool setup(const char* prefix);
        virtual bool setup_thread(unsigned idx, void** ptr);
        virtual bool teardown_thread(void* ptr);
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
        std::string m_prefix;

        database_write_sharded(const database_write_sharded&);
        database_write_sharded& operator = (const database_write_sharded&);
};

database_write_sharded :: database_write_sharded()
    : m_ap()
    , m_fsync(false)
    , m_prefix()
{
    m_ap.arg().long_name("fsync")
              .description("perform an fsync after each write (default: no)")
              .set_true(&m_fsync);
}

database_write_sharded :: ~database_write_sharded() throw ()
{
}

const e::argparser&
database_write_sharded :: parser()
{
    return m_ap;
}

bool
database_write_sharded :: setup(const char* prefix)
{
    m_prefix = prefix;
    return true;
}

bool
database_write_sharded :: setup_thread(unsigned idx, void** ptr)
{
    char buf[32];
    std::string path = m_prefix;
    sprintf(buf, "/file-%d.dat", idx);
    path += buf;
    int fd = open(path.c_str(), O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);

    if (fd < 0)
    {
        printf(__FILE__ ":%d %s\n", __LINE__, path.c_str());
        perror("write-sharded benchmark failed");
        return false;
    }

    *ptr = (void*)(intptr_t)fd;
    return true;
}

bool
database_write_sharded :: teardown_thread(void* ptr)
{
    int fd = (intptr_t)ptr;

    if (fd >= 0)
    {
        close(fd);
    }

    return true;
}

bool
database_write_sharded :: teardown()
{
    return true;
}

bool
database_write_sharded :: get(void* ptr, const char* key, size_t key_sz)
{
    abort();
    (void) ptr;
    (void) key;
    (void) key_sz;
}

bool
database_write_sharded :: put(void* ptr,
                      const char* key, size_t key_sz,
                      const char* val, size_t val_sz)
{
    int fd = (intptr_t)ptr;

    if (write(fd, key, key_sz) != (ssize_t)key_sz ||
        write(fd, val, val_sz) != (ssize_t)val_sz)
    {
        perror("write-sharded benchmark failed");
        return false;
    }

    if (m_fsync && fsync(fd) < 0)
    {
        perror("write-sharded benchmark failed");
        return false;
    }

    return true;
    (void) ptr;
}

bool
database_write_sharded :: del(void* ptr, const char* key, size_t key_sz)
{
    abort();
    (void) ptr;
    (void) key;
    (void) key_sz;
}

bool
database_write_sharded :: scan(void* ptr, const char* key, size_t key_sz, size_t num)
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
    return new database_write_sharded();
}
