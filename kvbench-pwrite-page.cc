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

// STL
#include <memory>

// po6
#include <po6/errno.h>
#include <po6/threads/mutex.h>

// kvbench
#include "database.h"

class database_pwrite_page : public database
{
    public:
        database_pwrite_page();
        ~database_pwrite_page() throw ();

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
        po6::threads::mutex m_mtx;
        off_t m_off;
        int m_fd;

        database_pwrite_page(const database_pwrite_page&);
        database_pwrite_page& operator = (const database_pwrite_page&);
};

database_pwrite_page :: database_pwrite_page()
    : m_ap()
    , m_fsync(false)
    , m_mtx()
    , m_off(0)
    , m_fd(-1)
{
    m_ap.arg().long_name("fsync")
              .description("perform an fsync after each write (default: no)")
              .set_true(&m_fsync);
}

database_pwrite_page :: ~database_pwrite_page() throw ()
{
}

const e::argparser&
database_pwrite_page :: parser()
{
    return m_ap;
}

bool
database_pwrite_page :: setup(const char* prefix)
{
    po6::threads::mutex::hold hold(&m_mtx);
    std::string path = prefix;
    path += "/file.dat";
    m_fd = open(path.c_str(), O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);

    if (m_fd < 0)
    {
        perror("pwrite benchmark failed");
        return false;
    }

    return true;
}

struct pwrite_page
{
    pwrite_page(size_t ps) : page_sz(ps), buf(NULL), buf_sz(0) {}
    ~pwrite_page() throw () { if (buf) free(buf); }

    const size_t page_sz;
    char* buf;
    size_t buf_sz;

    private:
        pwrite_page(const pwrite_page&);
        pwrite_page& operator = (const pwrite_page&);
};

bool
database_pwrite_page :: setup_thread(unsigned, void** ptr)
{
    *ptr = NULL;
    const size_t page_sz = sysconf(_SC_PAGESIZE);
    std::auto_ptr<pwrite_page> pp(new pwrite_page(page_sz));
    pp->buf = (char*)malloc(page_sz);

    if (!pp->buf)
    {
        return false;
    }

    pp->buf_sz = page_sz;
    *ptr = static_cast<void*>(pp.release());
    return true;
}

bool
database_pwrite_page :: teardown_thread(void* ptr)
{
    pwrite_page* pp = static_cast<pwrite_page*>(ptr);

    if (pp)
    {
        delete pp;
    }

    return true;
}

bool
database_pwrite_page :: teardown()
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
database_pwrite_page :: get(void* ptr, const char* key, size_t key_sz)
{
    abort();
    (void) ptr;
    (void) key;
    (void) key_sz;
}

bool
database_pwrite_page :: put(void* ptr,
                            const char* key, size_t key_sz,
                            const char* val, size_t val_sz)
{
    pwrite_page* pp = static_cast<pwrite_page*>(ptr);
    const size_t write_sz = (key_sz + val_sz - 1 + pp->page_sz) & ~(pp->page_sz - 1);

    if (write_sz > pp->buf_sz)
    {
        char* buf = (char*)realloc(pp->buf, write_sz);

        if (!buf)
        {
            perror("pwrite benchmark failed");
            return false;
        }

        pp->buf = buf;
        pp->buf_sz = write_sz;
    }

    memmove(pp->buf, key, key_sz);
    memmove(pp->buf + key_sz, val, val_sz);
    m_mtx.lock();
    off_t off = m_off;
    m_off += write_sz;
    m_mtx.unlock();

    if (pwrite(m_fd, pp->buf, write_sz, off) != (ssize_t)write_sz)
    {
        perror("pwrite benchmark failed");
        return false;
    }

    if (m_fsync && fsync(m_fd) < 0)
    {
        perror("pwrite benchmark failed");
        return false;
    }

    return true;
    (void) ptr;
}

bool
database_pwrite_page :: del(void* ptr, const char* key, size_t key_sz)
{
    abort();
    (void) ptr;
    (void) key;
    (void) key_sz;
}

bool
database_pwrite_page :: scan(void* ptr, const char* key, size_t key_sz, size_t num)
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
    return new database_pwrite_page();
}
