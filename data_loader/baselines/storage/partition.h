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
#ifndef __PARTITION__
#define __PARTITION__

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <bitset>

#include "../constants.h"

using namespace std;

template<typename T>
class Partition{

private:
    uint64_t pid;
    //uint64_t rid; // the order in rows
    bitset<ANUM> cols;
    int col_off[ANUM];
    uint64_t ncol;
    uint64_t nrow;
    T* data;

    char* buf;

public:
    Partition(uint64_t pid, bitset<ANUM> cols, T* data
            , uint64_t data_num);
    Partition(char* buf, uint64_t len);

    ~Partition();

    /*
     * col_id is the attribute ID of the column
     * off is the order of the tuple in the partition
     */
    T getData(uint64_t col_id, uint64_t off);
    uint64_t getPID();
   
    int writeToFile(const char* filename); 
    static Partition<T>* readFromFile(const char* filename);

};

template<typename T>
inline T Partition<T>::getData(uint64_t col_id, uint64_t off){
    return data[off*ncol + col_off[col_id]];
}

template<typename T>
inline uint64_t Partition<T>::getPID(){
    return pid;
}

#endif
