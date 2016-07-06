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

// po6
#include <po6/threads/thread.h>

// e
#include <e/atomic.h>
#include <e/compat.h>

// kvbench
#include "database.h"
#include "workload.h"
#include "workload-ycsb-core.h"

#define WORKLOAD(N, F) \
    do { if (load == N) return new F(); } while (0)

workload*
workload :: create(const char* _load)
{
    std::string load(_load);
    WORKLOAD("ycsb-core", workload_ycsb_core);
    return NULL;
}

workload :: workload()
    : m_db(NULL)
    , m_dl(NULL)
    , m_error()
{
    e::atomic::store_32_nobarrier(&m_error, 0);
}

workload :: ~workload() throw ()
{
}

bool
workload :: run(database* db, ygor_data_logger* dl, unsigned num_threads)
{
    m_db = db;
    m_dl = dl;

    if (!setup(num_threads))
    {
        return false;
    }

    po6::threads::barrier barrier(num_threads);
    typedef e::compat::shared_ptr<po6::threads::thread> thread_ptr;
    std::vector<thread_ptr> threads;

    for (unsigned i = 0; i < num_threads; ++i)
    {
        using namespace po6::threads;
        thread_ptr t(new thread(make_obj_func(&workload::run_worker, this, i, &barrier)));
        threads.push_back(t);
    }

    for (size_t i = 0; i < threads.size(); ++i)
    {
        threads[i]->start();
    }

    for (size_t i = 0; i < threads.size(); ++i)
    {
        threads[i]->join();
    }

    if (!teardown())
    {
        return false;
    }

    return e::atomic::load_32_nobarrier(&m_error) == 0 ? true : false;
}

void
workload :: run_worker(unsigned thread, po6::threads::barrier* b)
{
    void* db_state = NULL;
    void* work_state = NULL;
    bool fail = false;

    if (!m_db->setup_thread(thread, &db_state))
    {
        std::cerr << "database thread state setup failed\n" << std::flush;
        fail = true;
    }

    if (!this->setup_thread(thread, &work_state))
    {
        std::cerr << "workload thread state setup failed\n" << std::flush;
        fail = true;
    }

    b->wait();

    if (!fail && !this->run(db_state, work_state, thread))
    {
        fail = true;
    }

    if (!this->teardown_thread(work_state))
    {
        std::cerr << "workload thread state teardown failed\n" << std::flush;
        fail = true;
    }

    if (!m_db->teardown_thread(db_state))
    {
        std::cerr << "database thread state teardown failed\n" << std::flush;
        fail = true;
    }

    if (fail)
    {
        e::atomic::store_32_nobarrier(&m_error, 1);
    }
}

bool
workload :: setup(unsigned num_threads)
{
    (void) num_threads;
    return true;
}

bool
workload :: setup_thread(unsigned, void** ptr)
{
    *ptr = NULL;
    return true;
}

bool
workload :: teardown_thread(void* ptr)
{
    (void) ptr;
    return true;
}

bool
workload :: teardown()
{
    return true;
}
