// Copyright (c) 2016, Robert Escriva
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
#include <po6/time.h>

// e
#include <e/atomic.h>

// kvbench
#include "workload-ycsb-core.h"

struct workload_ycsb_core::thread_state
{
    thread_state();
    ~thread_state() throw ();

    armnod_config* opconf;
    armnod_generator* opgen;
    armnod_generator* keygen;
    armnod_generator* valgen;

    private:
        thread_state(const thread_state&);
        thread_state& operator = (const thread_state&);
};

workload_ycsb_core :: thread_state :: thread_state()
    : opconf(NULL)
    , opgen(NULL)
    , keygen(NULL)
    , valgen(NULL)
{
}

workload_ycsb_core :: thread_state :: ~thread_state() throw ()
{
    if (opgen) armnod_generator_destroy(opgen);
    if (keygen) armnod_generator_destroy(keygen);
    if (valgen) armnod_generator_destroy(valgen);
    if (opconf) armnod_config_destroy(opconf);
}

workload_ycsb_core :: workload_ycsb_core()
    : m_ap()
    , m_mtx()
    , m_key_parser(armnod_argparser::create("key-", true))
    , m_val_parser(armnod_argparser::create("value-"))
    , m_max_ops(10000)
    , m_weight_read(5)
    , m_weight_write(95)
    , m_weight_modify(0)
    , m_weight_delete(0)
    , m_weight_scan(0)
    , m_ops_done(0)
    , m_series_read()
    , m_series_write()
    , m_series_modify()
    , m_series_delete()
    , m_series_scan()
{
    m_ap.add("Key Generation:", m_key_parser->parser());
    m_ap.add("Value Generation:", m_val_parser->parser());
    m_ap.arg().name('O', "max-ops")
              .description("max number of operations to perform (default: 10000)")
              .metavar("#")
              .as_long(&m_max_ops);
    m_ap.arg().name('R', "read")
              .description("weight assigned to read operations (default: 5)")
              .metavar("#")
              .as_long(&m_weight_read);
    m_ap.arg().name('W', "write")
              .description("weight assigned to write operations (default: 95)")
              .metavar("#")
              .as_long(&m_weight_write);
    m_ap.arg().name('M', "modify")
              .description("weight assigned to read-modify-write operations (default: 0)")
              .metavar("#")
              .as_long(&m_weight_modify);
    m_ap.arg().name('D', "delete")
              .description("weight assigned to delete operations (default: 0)")
              .metavar("#")
              .as_long(&m_weight_delete);
    m_ap.arg().name('S', "scan")
              .description("weight assigned to scan operations (default: 0)")
              .metavar("#")
              .as_long(&m_weight_scan);

    m_series_read.name = "read";
    m_series_read.indep_units = YGOR_UNIT_MS;
    m_series_read.indep_precision = YGOR_PRECISE_INTEGER;
    m_series_read.dep_units = YGOR_UNIT_MS;
    m_series_read.dep_precision = YGOR_HALF_PRECISION;

    m_series_write.name = "write";
    m_series_write.indep_units = YGOR_UNIT_MS;
    m_series_write.indep_precision = YGOR_PRECISE_INTEGER;
    m_series_write.dep_units = YGOR_UNIT_MS;
    m_series_write.dep_precision = YGOR_HALF_PRECISION;

    m_series_modify.name = "modify";
    m_series_modify.indep_units = YGOR_UNIT_MS;
    m_series_modify.indep_precision = YGOR_PRECISE_INTEGER;
    m_series_modify.dep_units = YGOR_UNIT_MS;
    m_series_modify.dep_precision = YGOR_HALF_PRECISION;

    m_series_delete.name = "delete";
    m_series_delete.indep_units = YGOR_UNIT_MS;
    m_series_delete.indep_precision = YGOR_PRECISE_INTEGER;
    m_series_delete.dep_units = YGOR_UNIT_MS;
    m_series_delete.dep_precision = YGOR_HALF_PRECISION;

    m_series_scan.name = "scan";
    m_series_scan.indep_units = YGOR_UNIT_MS;
    m_series_scan.indep_precision = YGOR_PRECISE_INTEGER;
    m_series_scan.dep_units = YGOR_UNIT_MS;
    m_series_scan.dep_precision = YGOR_HALF_PRECISION;

    m_series[0] = &m_series_read;
    m_series[1] = &m_series_write;
    m_series[2] = &m_series_modify;
    m_series[3] = &m_series_delete;
    m_series[4] = &m_series_scan;
}

workload_ycsb_core :: ~workload_ycsb_core() throw ()
{
}

const e::argparser&
workload_ycsb_core :: parser()
{
    return m_ap;
}

const ygor_series**
workload_ycsb_core :: series()
{
    return m_series;
}

size_t
workload_ycsb_core :: series_sz()
{
    return 5;
}

bool
workload_ycsb_core :: setup(unsigned)
{
    po6::threads::mutex::hold hold(&m_mtx);
    double sum = m_weight_read + m_weight_write + m_weight_modify + m_weight_delete;

    for (unsigned idx = 0; idx < 256; ++idx)
    {
        double x = idx / 256.;

        if (x < m_weight_read / sum)
        {
            m_ops[idx] = 'R';
        }
        else if (x < (m_weight_read + m_weight_write) / sum)
        {
            m_ops[idx] = 'W';
        }
        else if (x < (m_weight_read + m_weight_write + m_weight_modify) / sum)
        {
            m_ops[idx] = 'M';
        }
        else if (x < (m_weight_read + m_weight_write + m_weight_modify + m_weight_delete) / sum)
        {
            m_ops[idx] = 'D';
        }
        else if (x < (m_weight_read + m_weight_write + m_weight_modify + m_weight_delete + m_weight_scan) / sum)
        {
            m_ops[idx] = 'S';
        }
        else
        {
            abort();
        }
    }

    return true;
}

bool
workload_ycsb_core :: setup_thread(unsigned, void** ptr)
{
    po6::threads::mutex::hold hold(&m_mtx);
    std::auto_ptr<thread_state> ts(new thread_state());

    ts->opconf = armnod_config_create();
    armnod_config_choose_fixed(ts->opconf, 256);
    ts->opgen = armnod_generator_create(ts->opconf);

    ts->keygen = armnod_generator_create(m_key_parser->config());
    ts->valgen = armnod_generator_create(m_val_parser->config());

    uint64_t seed = (uintptr_t)pthread_self();
    armnod_seed(ts->opgen,  seed);
    armnod_seed(ts->keygen, seed ^ (0x55aaULL << 48));
    armnod_seed(ts->valgen, seed ^ (0xaa55ULL << 48));
    *ptr = ts.release();
    return true;
}

bool
workload_ycsb_core :: run(void* db_state, void* work_state, unsigned)
{
    thread_state* ts = static_cast<thread_state*>(work_state);

    while (e::atomic::increment_64_nobarrier(&m_ops_done, 1) <= (unsigned long)m_max_ops)
    {
        unsigned idx = armnod_generate_idx_only(ts->opgen);
        assert(idx < 256);
        size_t key_sz = 0;
        const char* key = armnod_generate_sz(ts->keygen, &key_sz);
        size_t val_sz = 0;
        const char* val = NULL;

        if (!key)
        {
            break;
        }

        ygor_data_point dp;
        const uint64_t start = po6::monotonic_time();
        dp.series = NULL;

        switch (m_ops[idx])
        {
            case 'R':
            case 'M':
                if (!m_db->get(db_state, key, key_sz))
                {
                    return false;
                }
                if (m_ops[idx] == 'R')
                {
                    dp.series = &m_series_read;
                    break;
                }
                dp.series = &m_series_modify;
                assert(m_ops[idx] == 'M');
            case 'W':
                val = armnod_generate_sz(ts->valgen, &val_sz);
                if (!m_db->put(db_state, key, key_sz, val, val_sz))
                {
                    return false;
                }
                dp.series = &m_series_write;
                break;
            case 'D':
                if (!m_db->del(db_state, key, key_sz))
                {
                    return false;
                }
                dp.series = &m_series_delete;
                break;
            case 'S':
                if (!m_db->scan(db_state, key, key_sz, 10))
                {
                    return false;
                }
                dp.series = &m_series_scan;
                break;
            default:
                std::cerr << "corrupt internal state\n";
                return false;
        }

        const uint64_t end = po6::monotonic_time();
        assert(dp.series);
        dp.indep.precise = end / PO6_MILLIS;
        dp.dep.approximate = (end - start) / (double)PO6_MILLIS;

        if (ygor_data_logger_record(m_dl, &dp) < 0)
        {
            return false;
        }
    }

    return true;
}

bool
workload_ycsb_core :: teardown_thread(void* ptr)
{
    if (ptr)
    {
        delete static_cast<thread_state*>(ptr);
    }

    return true;
}
