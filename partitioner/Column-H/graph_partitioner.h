/*
 * Copyright 2020, Jigsaw authors (see AUTHORS file).
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __GRAPH_PARTITIONER__
#define __GRAPH_PARTITIONER__

#include <stdlib.h>
#include <stdio.h>
#include <metis.h>
#include <mutex>
#include "tuples.h"

using namespace std;

template<typename T>
class GPartitioner{
public:
    GPartitioner(Tuples<T>* tuples, int pnum):
        tuples(tuples), pnum(pnum){}

    void partition(vector<uint64_t>& parts);
private:
    void metis(uint64_t sample_size, vector<bitset<QNUM>> &part_access);
    void assign(vector<bitset<QNUM>> &part_access
            , vector<uint64_t> &parts);

    void* assign_t(void* args);

    struct assign_params{
        vector<bitset<QNUM>>* part_access;
        vector<uint64_t>* parts; //the partition ID of each tuple
        vector<uint64_t>* part_size; //the #tuples in a partition
        vector<mutex>* part_lock; //locks of the part_size 

        uint64_t start_tid; //inclusive
        uint64_t end_tid; //exclusive
    };

    Tuples<T>* tuples;
    int pnum;

};

#endif
