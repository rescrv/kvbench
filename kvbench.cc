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

// STL
#include <memory>

// po6
#include <po6/errno.h>
#include <po6/threads/thread.h>
#include <po6/time.h>

// e
#include <e/atomic.h>
#include <e/popt.h>

// ygor
#include <ygor/data.h>

// kvbench
#include "database.h"
#include "workload.h"

int
main(int argc, const char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <workload> [OPTION...]" << std::endl;
        return EXIT_FAILURE;
    }

    const std::auto_ptr<workload> work(workload::create(argv[1]));
    const std::auto_ptr<database> db(database::create());
    assert(db.get());

    if (!work.get())
    {
        std::cerr << "Unknown workload " << argv[1] << std::endl;
        std::cerr << "Usage: " << argv[0] << " <workload> [OPTION...]" << std::endl;
        return EXIT_FAILURE;
    }

    const char* output = "benchmark.dat";
    const char* dir = "tmp";
    long num_threads = 1;
    bool stats = true;

    e::argparser ap;
    ap.autohelp();
    ap.arg().name('o', "output")
            .description("output benchmark results (default: benchmark.dat.bz2)")
            .as_string(&output);
    ap.arg().name('d', "dir")
            .description("directory under which to store the data (default: tmp)")
            .as_string(&dir);
    ap.arg().name('t', "threads")
            .description("run the benchmark with T concurrent threads (default: 1)")
            .metavar("T")
            .as_long(&num_threads);
    ap.arg().long_name("no-stats")
            .description("don't collect local system stats")
            .set_false(&stats);
    ap.add("Database Specific Options:", db->parser());
    ap.add("Workload Specific Options:", work->parser());

    if (!ap.parse(argc - 1, argv + 1))
    {
        return EXIT_FAILURE;
    }

    ygor_data_logger* dl = ygor_data_logger_create(output, work->series(), work->series_sz());

    if (!dl)
    {
        std::cerr << "could not open output: " << po6::strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }

    if (!db->setup(dir))
    {
        return EXIT_FAILURE;
    }

    int rc = EXIT_SUCCESS;

    if (!work->run(db.get(), dl, num_threads))
    {
        rc = EXIT_FAILURE;
    }

    if (!db->teardown())
    {
        rc = EXIT_FAILURE;
    }

    if (ygor_data_logger_flush_and_destroy(dl) < 0)
    {
        std::cerr << "could not close output: " << po6::strerror(errno) << std::endl;
        rc = EXIT_FAILURE;
    }

    return rc;
}
