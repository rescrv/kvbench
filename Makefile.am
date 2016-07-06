# Copyright (c) 2014-2016, Robert Escriva
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#     * Redistributions of source code must retain the above copyright notice,
#       this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of this project nor the names of its contributors may
#       be used to endorse or promote products derived from this software
#       without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

AM_CPPFLAGS  = $(YGOR_CPPFLAGS)
AM_CFLAGS    = -D_FILE_OFFSET_BITS=64 $(YGOR_CFLAGS) $(WANAL_CFLAGS)
AM_CXXFLAGS  = -D_FILE_OFFSET_BITS=64 $(YGOR_CFLAGS) $(WANAL_CXXFLAGS)
if MAKE_NO_PRINT_DIRECTORY
AM_MAKEFLAGS = --no-print-directory
endif

EXTRA_DIST  =
EXTRA_DIST += LICENSE

noinst_HEADERS =
bin_PROGRAMS  =

# Common Pieces
noinst_LTLIBRARIES = libkvbench.la

libkvbench_la_SOURCES =
libkvbench_la_SOURCES += database.cc
libkvbench_la_SOURCES += workload.cc
libkvbench_la_SOURCES += workload-ycsb-core.cc
libkvbench_la_SOURCES += kvbench.cc

# Unix write benchmark
bin_PROGRAMS += kvbench-write
kvbench_write_SOURCES = kvbench-write.cc
kvbench_write_LDADD = libkvbench.la ${POPT_LIBS} ${YGOR_LIBS}

# Unix pwrite benchmark
bin_PROGRAMS += kvbench-pwrite
kvbench_pwrite_SOURCES = kvbench-pwrite.cc
kvbench_pwrite_LDADD = libkvbench.la ${POPT_LIBS} ${YGOR_LIBS}

# Unix pwrite benchmark (page-aligned)
bin_PROGRAMS += kvbench-pwrite-page
kvbench_pwrite_page_SOURCES = kvbench-pwrite-page.cc
kvbench_pwrite_page_LDADD = libkvbench.la ${POPT_LIBS} ${YGOR_LIBS}

# libc write benchmark
bin_PROGRAMS += kvbench-fwrite
kvbench_fwrite_SOURCES = kvbench-fwrite.cc
kvbench_fwrite_LDADD = libkvbench.la ${POPT_LIBS} ${YGOR_LIBS}

# Sharded Unix write benchmark
bin_PROGRAMS += kvbench-write-sharded
kvbench_write_sharded_SOURCES = kvbench-write-sharded.cc
kvbench_write_sharded_LDADD = libkvbench.la ${POPT_LIBS} ${YGOR_LIBS}

if ENABLE_LEVELDB
bin_PROGRAMS += kvbench-leveldb
kvbench_leveldb_SOURCES  = kvbench-leveldb.cc
kvbench_leveldb_LDADD    = libkvbench.la -lleveldb ${POPT_LIBS} ${YGOR_LIBS}
kvbench_leveldb_CPPFLAGS = $(AM_CPPFLAGS) -I"${LEVELDB_REPO}/include" $(CPPFLAGS)
kvbench_leveldb_LDFLAGS  = -L"${LEVELDB_REPO}/out-shared" -Wl,-rpath -Wl,"${LEVELDB_REPO}/out-shared"
endif
