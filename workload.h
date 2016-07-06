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

#ifndef kvbench_workload_h_
#define kvbench_workload_h_

// C
#include <stdlib.h>

// STL
#include <string>
#include <vector>

// po6
#include <po6/threads/barrier.h>

// ygor
#include <ygor/data.h>

// kvbench
#include "database.h"

class workload
{
    public:
        static workload* create(const char* load);

    public:
        workload();
        virtual ~workload() throw ();

    public:
        virtual const e::argparser& parser() = 0;
        virtual const ygor_series** series() = 0;
        virtual size_t series_sz() = 0;
        bool run(database* db, ygor_data_logger* dl, unsigned num_threads);

    protected:
        virtual bool setup(unsigned num_threads);
        virtual bool setup_thread(unsigned idx, void** ptr);
        virtual bool run(void* db_state, void* work_state, unsigned idx) = 0;
        virtual bool teardown_thread(void* ptr);
        virtual bool teardown();

    protected:
        database* m_db;
        ygor_data_logger* m_dl;

    private:
        void run_worker(unsigned thread, po6::threads::barrier* b);

    private:
        uint32_t m_error;

    private:
        workload(const workload&);
        workload& operator = (const workload&);
};

#endif // kvbench_workload_h_
