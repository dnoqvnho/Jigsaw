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
#ifndef __PSCHEMA__
#define __PSCHEMA__

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <bitset>
#include "../constants.h"
#include "../storage/index.h"

using namespace std;

class PSchema{
public:

    PSchema(PType pt, const char* filename = NULL);
    
    /*
     * Get the number of vertical partitions 
     */
    uint64_t getVNum(){return cpnum;}

    /*
     * Get the partition IDs of a given tuple ID
     * The length of pids is same as the number of vertical partitions
     */
    void getPids(uint64_t tid, uint64_t pids[]);
   
    /*
     * Get the number of partitions of the table
     */ 
    uint64_t getPNum();

    /*
     * Get the vertical partitioning
     * The length of sv is same as the number of attributes
     */
    void getVSchema(int sv[]); 

    /*
     * Given a pid, get the attributes stored in the partition
     */
    bitset<ANUM> getVPartition(uint64_t pid);

    /*
     * Create the Index object
     * The tuples are given new IDs 
     */
    Index* createIndex();


    ~PSchema();
private:
    int col_off[ANUM]; //vertical partitioning
    uint64_t* row_off; //tupleID to horizontal partition ID (Col-H)
    uint64_t cpnum; //the number of vertical partitions
    PType type; //the partitioning type
    
    //the number of tuples in a partition
    //The length is same as cpnum
    //Except for Col-H
    uint64_t* psize; 
    
    //the number of partitions in previous vertical partitions
    //the lenght is same as cpnum
    uint64_t* rpnum;
};

#endif
