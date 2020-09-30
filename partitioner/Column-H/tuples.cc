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
#include "tuples.h"

template<typename T>
Tuples<T>::Tuples(vector<Query<T>*> queries){
    this->queries = queries;
    tnum = 0;
    access.resize(TNUM);
}

template<typename T>
bool Tuples<T>::push(T* buf, uint64_t tnum){
    if(tnum + this->tnum > TNUM)
        return false;
    
    //generate the access pattern
    for(uint64_t t = 0; t < tnum; t++){
        T* bptr = buf + t * ANUM;
        for(int j = 0; j < queries.size(); j++)
            if(queries[j]->evaluate(bptr))
                this->access[this->tnum + t][j] = 1;
    }
    
    this->tnum += tnum;
    return true;
}


template<typename T>
void Tuples<T>::sample(uint64_t tnum, set<uint64_t>& tids){
    tids.clear();
    for(uint64_t i = 0; i < tnum; i++){
        uint64_t tid = rand() % this->tnum;
        if(tids.find(tid) == tids.end())
            tids.insert(tid);
        else
            i--;
    }
}

template class Tuples<VTYPE>;
