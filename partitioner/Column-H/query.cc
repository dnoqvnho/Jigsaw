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
Query<T>::Query(uint64_t qid
                , const vector<int>& selection
                , const vector<T>& ranges
                , const bitset<2*ANUM>& closed){

    this->qid = qid;
    this->sel_vec = selection;
    for(auto& i : selection)
        this->selection[i] = 1;
    this->ranges = ranges;
    this->closed = closed;
}

template<typename T>
Query<T>::Query(uint64_t qid
        , const bitset<ANUM>& selection
        , const vector<T>& ranges
        , const bitset<2*ANUM>& closed){
    this->qid = qid;
    this->selection = selection;
    for(int i = 0; i < ANUM; i++)
        if(selection[i] == 1)
            this->sel_vec.push_back(i);
    this->ranges = ranges;
    this->closed = closed;
}

template<typename T>
Query<T>* Query<T>::parse_string(string str){
    stringstream ss(str);
    string token;

    //projection, don't care
    ss >> token;

    //selection
    ss >> token;
    reverse(token.begin(), token.end());
    bitset<ANUM> selection(token);

    //ranges
   vector<T> ranges;
   ranges.reserve(2*ANUM);
   for(int i = 0; i < 2*ANUM; i++){
       ranges.push_back(0);
       ss >> ranges[i];
   }

   //closed
   ss >> token;
   reverse(token.begin(), token.end());
   bitset<2*ANUM> closed(token);

   Query<T>* q = new Query<T>(next_qid++, selection, ranges, closed);
   return q;
}

template<typename T>
bool Query<T>::evaluate(T* data){
    for(auto i : sel_vec){
        T v = data[i];
        T l(ranges[2*i]), r(ranges[2*i + 1]);
        if(v < l || (v == l && closed[2*i] == 0))
            return false;
        if(v > r || (v == r && closed[2*i + 1] == 0))
            return false;
    }
    return true;
}

template<typename T>
void Query<T>::evaluate(T* data, bool ret[], uint64_t tnum){
    for(uint64_t i = 0; i < tnum; i++)
        ret[i] = true;
    for(uint64_t i = 0 ;  i < tnum; i++){
        for(auto j : sel_vec){
            T v = data[i*ANUM + j];
            T l(ranges[2*j]), r(ranges[2*j + 1]);
            if((v < l || (v == l && closed[2*j] == 0))
                    || (v > r || (v == r && closed[2*j + 1] == 0))){
                ret[i] = false;
                break;
            }
        }
    }
}

template class Query<VTYPE>;
