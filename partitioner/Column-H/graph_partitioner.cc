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
#include "graph_partitioner.h"
#include <assert.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <thread>
#include <iostream>

template<typename T>
void GPartitioner<T>::metis(uint64_t sample_size
        , vector<bitset<QNUM>> &part_access){
    set<uint64_t> stids;
    tuples->sample(sample_size, stids);
    vector<uint64_t> tids(stids.begin(), stids.end());
    
    idx_t nvtxs = tids.size();
    idx_t ncon = 1;
    idx_t* xadj = new idx_t[nvtxs + 1];
    vector<idx_t> adjncy;
    vector<idx_t> adjwgt;
    idx_t options[METIS_NOPTIONS];
    idx_t* part = new idx_t[nvtxs];
    idx_t objval;
    idx_t nparts = this->pnum;

    xadj[0] = 0;
    for(idx_t i = 0; i < nvtxs; i++){
        for(idx_t j = 0; j < nvtxs; j++){
            if(i == j)
                continue;
            bitset<QNUM> *acc_i, *acc_j;
            acc_i = tuples->getAccess(tids[i]);
            acc_j = tuples->getAccess(tids[j]);

            idx_t dist = ((*acc_i) & (*acc_j)).count();
            if(dist != 0){
                adjncy.push_back(j);
                adjwgt.push_back(dist);
            }
        }
        xadj[i+1] = adjncy.size();
    }
    
    METIS_SetDefaultOptions(options);
    options[METIS_OPTION_NUMBERING] = 0;

    int ret = METIS_PartGraphKway(&nvtxs, &ncon, xadj, adjncy.data()
            , NULL, NULL
            , adjwgt.data(), &nparts, NULL, NULL, options, &objval
            , part);
    assert(ret == METIS_OK);
    delete[] xadj;

    part_access.clear();
    part_access.resize(this->pnum);
    for(uint64_t i = 0; i < tids.size(); i++){
        bitset<QNUM>* access = tuples->getAccess(tids[i]);
        idx_t p = part[i];
        part_access[p] |= *access;
    }
    delete[] part;
}

template<typename T>
void* GPartitioner<T>::assign_t(void* args){
    assign_params* input = (assign_params*)args;

    for(uint64_t tid = input->start_tid; tid < input->end_tid; tid++){
        uint64_t dist = QNUM;
        uint64_t min_pid = -1;
        bitset<QNUM>* t_access = tuples->getAccess(tid);

        for(uint64_t i = 0; i < input->part_access->size() && dist != 0
                ; i++){
            if(input->part_size->at(i) >= MAX_PSIZE)
               continue;
            uint64_t d = ((*t_access)^input->part_access->at(i)).count();
            if(d <= dist){
                min_pid = i;
                dist = d;
            }
        }
        
        assert(min_pid != -1);
        input->parts->at(tid) = min_pid;
        
        input->part_lock->at(min_pid).lock();
        input->part_size->at(min_pid) ++;
        input->part_lock->at(min_pid).unlock();
    }

    return NULL;
}

template<typename T>
void GPartitioner<T>::assign(vector<bitset<QNUM>> &part_access
        , vector<uint64_t> &parts){
    parts.clear();
    parts.resize(tuples->getTNum()); 
    vector<uint64_t> part_size(part_access.size(), 0);
    vector<mutex> part_lock(part_access.size());
    
    //int ncpu = get_nprocs();
    int ncpu = 1;
    printf("USE %d cores\n", ncpu);
    uint64_t step = tuples->getTNum() / ncpu;
    assign_params params[ncpu];
    std::thread threads[ncpu]; 

    for(int i = 0; i < ncpu; i++){
        params[i].part_access = &part_access;
        params[i].parts = &parts;
        params[i].part_size = &part_size;
        params[i].part_lock = &part_lock;
        params[i].start_tid = step * i;
        params[i].end_tid = step*(i+1);
        if(i == ncpu-1)
            params[i].end_tid = tuples->getTNum();
        threads[i] = std::thread(&GPartitioner<T>::assign_t, this
                , &params[i]);
    }

    for(auto& th: threads)
        th.join();
}

template<typename T>
void GPartitioner<T>::partition(vector<uint64_t>& parts){
    vector<bitset<QNUM>> part_access;
    
    struct timeval start, start_assign, end;
    gettimeofday(&start, NULL); 
   
    this->metis(MAX_TUPLES, part_access);

    gettimeofday(&start_assign, NULL); 

    this->assign(part_access, parts);
    
    gettimeofday(&end, NULL); 
    unsigned long long t1 = 1000000*( start_assign.tv_sec - start.tv_sec )
            + start_assign.tv_usec -start.tv_usec;
    unsigned long long t2 = 1000000*(end.tv_sec - start_assign.tv_sec )
            + end.tv_usec - start_assign.tv_usec;
    cout << "METIS time: " << t1 << " micorseconds" << endl;
    cout << "Assign time: " << t2 << " micorseconds" << endl;
    cout << "Total time: " << t1+t2 << " micorseconds" << endl;
    cout << "Number of partitions: " << part_access.size() << endl;
}

template class GPartitioner<VTYPE>;
