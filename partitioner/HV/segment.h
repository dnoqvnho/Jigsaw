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
#ifndef SEGMENT
#define SEGMENT

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <bitset>
#include <vector>
#include <string>

#include "constants.h"
#include "query.h"

using namespace std;

template <typename T>
class Segment{

public:

    Segment(const bitset<ANUM>& attributes
            , uint64_t tnum
            , const vector<T>& ranges
            , const bitset<2*ANUM>& closed
            , const vector<Query<T>*>& queries);


    void add_query(const vector<Query<T>*> &queries);
    
    double split(vector<Segment<T>*>& children);

    bool fit_max_psize(vector<Segment<T>*>& children);
    
    void query_bits(bitset<QNUM> &bits);

    uint64_t get_size();
    uint64_t get_data_size();
    uint64_t get_id_size();
    
    bool intersect(Query<T>* query);

    bitset<ANUM> attributes;
    uint64_t tnum;
    uint64_t anum;
    vector<T> ranges;
    bitset<2*ANUM> closed; //the range is colsed or not
    double benefit;
    vector<Query<T>*> queries;

    double cost; //the I/O cost to read the segment by the queries

    string to_string();
    static Segment<T> parse_string(string str);

private:
    /*
     * split the segment by a query
     * return the benefit by the spliting.
     * return -1 when the segment should not be splitted 
     */
    double split(const Query<T>& q, vector<Segment<T>*>& children);
    
    //horizontally split the segment.
    //When children is empty, the segment is not partitioned
    void horizontal_split(const Query<T>& q
            , Segment<T>* children[2]);

    //horziontally split the partition in the attribute aid with value v
    //the in_first is true when the v is included in the first children
    //false when v is in the second children
    //Children is NULL when v is not in the value range of aid
    void horizontal_split(int aid, T v, bool in_first
            , Segment<T>* children[2]);

    //return true if the partition is splitted
    //return false if the partition can not be splitted and it has been added to the children
    bool fit_max_psize(int aid, vector<Segment<T>*> &children);

    void filter_queries();

    void update_cost();

};
#endif
