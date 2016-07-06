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

#ifndef kvbench_workload_ycsb_core_h_
#define kvbench_workload_ycsb_core_h_

// STL
#include <memory>

// ygor
#include <ygor/armnod.h>

// kvbench
#include "workload.h"

class workload_ycsb_core : public workload
{
    public:
        workload_ycsb_core();
        virtual ~workload_ycsb_core() throw ();

    public:
        virtual const e::argparser& parser();
        virtual const ygor_series** series();
        virtual size_t series_sz();

    protected:
        virtual bool setup(unsigned num_threads);
        virtual bool setup_thread(unsigned idx, void** ptr);
        virtual bool run(void* db_state, void* work_state, unsigned idx);
        virtual bool teardown_thread(void* ptr);

    private:
        struct thread_state;

    private:
        e::argparser m_ap;
        po6::threads::mutex m_mtx;
        const std::auto_ptr<armnod_argparser> m_key_parser;
        const std::auto_ptr<armnod_argparser> m_val_parser;
        long m_max_ops;
        long m_weight_read;
        long m_weight_write;
        long m_weight_modify;
        long m_weight_delete;
        long m_weight_scan;
        char m_ops[256];
        uint64_t m_ops_done;
        ygor_series m_series_read;
        ygor_series m_series_write;
        ygor_series m_series_modify;
        ygor_series m_series_delete;
        ygor_series m_series_scan;
        const ygor_series* m_series[5];

    private:
        workload_ycsb_core(const workload_ycsb_core&);
        workload_ycsb_core& operator = (const workload_ycsb_core&);
};

#endif // kvbench_workload_ycsb_core_h_
