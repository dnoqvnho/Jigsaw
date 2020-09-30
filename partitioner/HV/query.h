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
#ifndef QUERY
#define QUERY

#include <stdlib.h>
#include <stdio.h>
#include <bitset>
#include <vector>
#include <stdint.h>

#include "constants.h"

using namespace std;

template <typename T>
class Query{
public:

    Query(uint64_t qid, const vector<int>& projection
            , const vector<int>& selection
            , const vector<T>& ranges
            , const bitset<2*ANUM>& closed);

    Query(uint64_t qid, const bitset<ANUM>& projection
            , const bitset<ANUM>& selection
            , const vector<T>& ranges
            , const bitset<2*ANUM>& closed);

    static Query<T>* parse_string(string str);
    
    bitset<ANUM> projection;
    bitset<ANUM> selection;
    vector<T> ranges;
    bitset<2*ANUM> closed; //the range is closed or not
    uint64_t qid;

private:
    static uint64_t next_qid;
};

#endif
