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
#ifndef PARTITIONER
#define PARTITIONER

#include <stdlib.h>
#include <stdio.h>
#include <vector>

#include "segment.h"

using namespace std;

template<typename T>
class Partitioner{

public:
    /*
     * Partition a table
     * Return the estimated cost to evaluate queries on this partitioning
     * The cost is in microseconds.
     */
    static double partition(Segment<T>* table
            , vector<vector<Segment<T>*>> &partitions);

private:

    struct snode{
        Segment<T>* self;
        vector<Segment<T>*> children;

        snode(Segment<T>* self
                , vector<Segment<T>*> &child):self(self)
        {
            this->children = child;
        }

        bool operator< (const snode& p) const{
            return self->benefit < p.self->benefit;
        }
    };

    static void split(Segment<T>* table, vector<Segment<T>*> &segments);


    //merge the segments accessed by same queries to a partition
    //only merge partitions that are smaller than the MIN_PSIZE
    static void merge(const vector<Segment<T>*> &segments
            , vector<vector<Segment<T>*>> &partitions);

};

#endif
