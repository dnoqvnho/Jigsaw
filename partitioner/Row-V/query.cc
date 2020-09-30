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
#include "query.h"
#include <sstream>
#include <bits/stdc++.h>

template<typename T>
uint64_t Query<T>::next_qid = 0;

template<typename T>
Query<T>::Query(uint64_t qid, const vector<int>& projection
                , const vector<int>& selection
                , const vector<T>& ranges
                , const bitset<2*ANUM>& closed){

    this->qid = qid;
    for(auto& i : projection)
        this->projection[i] = 1;
    for(auto& i : selection)
        this->selection[i] = 1;
    this->ranges = ranges;
    this->closed = closed;
}

template<typename T>
Query<T>::Query(uint64_t qid, const bitset<ANUM>& projection
        , const bitset<ANUM>& selection
        , const vector<T>& ranges
        , const bitset<2*ANUM>& closed){

    this->qid = qid;
    this->projection = projection;
    this->selection = selection;
    this->ranges = ranges;
    this->closed = closed;

}

template<typename T>
Query<T>* Query<T>::parse_string(string str){
    stringstream ss(str);
    string token;

    ss >> token;
    reverse(token.begin(), token.end());
    bitset<ANUM> projection(token);

    ss >> token;
    reverse(token.begin(), token.end());
    bitset<ANUM> selection(token);


   projection = projection & (projection ^ selection);

   vector<T> ranges;
   ranges.reserve(2*ANUM);
   for(int i = 0; i < 2*ANUM; i++){
       ranges.push_back(0);
       ss >> ranges[i];
   }

   ss >> token;
   reverse(token.begin(), token.end());
   bitset<2*ANUM> closed(token);

   Query<T>* q = new Query<T>(next_qid++, projection, selection, ranges, closed);
   return q;
}

template class Query<VTYPE>;
