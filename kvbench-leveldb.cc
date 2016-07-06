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
#include <unistd.h>

// STL
#include <memory>

// LevelDB
#include <leveldb/comparator.h>
#include <leveldb/db.h>
#include <leveldb/filter_policy.h>

// kvbench
#include "database.h"

class database_leveldb : public database
{
    public:
        database_leveldb();
        ~database_leveldb() throw ();

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
        const leveldb::FilterPolicy* m_bf;
        leveldb::DB* m_db;

        database_leveldb(const database_leveldb&);
        database_leveldb& operator = (const database_leveldb&);
};

database_leveldb :: database_leveldb()
    : m_ap()
    , m_bf(NULL)
    , m_db(NULL)
{
}

database_leveldb :: ~database_leveldb() throw ()
{
}

const e::argparser&
database_leveldb :: parser()
{
    return m_ap;
}

bool
database_leveldb :: setup(const char* prefix)
{
    leveldb::Options opts;
    opts.create_if_missing = true;
    opts.filter_policy = m_bf = leveldb::NewBloomFilterPolicy(10);
    opts.max_open_files = std::max(sysconf(_SC_OPEN_MAX) >> 1, 1024L);
    leveldb::Status st = leveldb::DB::Open(opts, prefix, &m_db);

    if (!st.ok())
    {
        std::cerr << "could not open LevelDB: " << st.ToString() << std::endl;
        return false;
    }

    return true;
}

bool
database_leveldb :: teardown()
{
    if (m_db)
    {
        delete m_db;
    }

    return true;
}

bool
database_leveldb :: get(void*, const char* key, size_t key_sz)
{
    std::string value;
    leveldb::Status st = m_db->Get(leveldb::ReadOptions(), leveldb::Slice(key, key_sz), &value);

    if (!st.ok() && !st.IsNotFound())
    {
        std::cerr << "leveldb error: " << st.ToString() << std::endl;
        return false;
    }

    return true;
}

bool
database_leveldb :: put(void*,
                       const char* key, size_t key_sz,
                       const char* val, size_t val_sz)
{
    leveldb::WriteOptions opts;
    opts.sync = false;
    leveldb::Status st = m_db->Put(opts, leveldb::Slice(key, key_sz), leveldb::Slice(val, val_sz));

    if (!st.ok())
    {
        std::cerr << "leveldb error: " << st.ToString() << std::endl;
        return false;
    }

    return true;
}

bool
database_leveldb :: del(void*, const char* key, size_t key_sz)
{
    leveldb::WriteOptions opts;
    opts.sync = false;
    leveldb::Status st = m_db->Delete(opts, leveldb::Slice(key, key_sz));

    if (!st.ok() && !st.IsNotFound())
    {
        std::cerr << "leveldb error: " << st.ToString() << std::endl;
        return false;
    }

    return true;
}

bool
database_leveldb :: scan(void*, const char* key, size_t key_sz, size_t num)
{
    std::auto_ptr<leveldb::Iterator> it(m_db->NewIterator(leveldb::ReadOptions()));
    it->Seek(leveldb::Slice(key, key_sz));

    for (size_t i = 0; i < num && it->Valid(); ++i)
    {
        it->Next();
    }

    if (!it->status().ok())
    {
        std::cerr << "leveldb error: " << it->status().ToString() << std::endl;
        return false;
    }

    return true;
}

database*
database::create()
{
    return new database_leveldb();
}
