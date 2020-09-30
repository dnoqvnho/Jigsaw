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
#include "partition.h"

#include <fstream>
#include <string>
#include <string.h>

template<typename T>
Partition<T>::Partition(uint64_t pid, bitset<ANUM> cols
        , T* data, uint64_t data_num){
    this->pid = pid;
//    this->rid = rid;
    this->cols = cols;
    this->data = data;
    this->ncol = this->cols.count();
    this->nrow = data_num / this->ncol;
    this->buf = NULL;


    //col_off
    int t = 0;
    for(int i = 0; i < ANUM; i++){
        col_off[i] = -1;
        if(this->cols[i] == 1)
            col_off[i] = t++;
    }
}

template<typename T>
Partition<T>::Partition(char* buf, uint64_t len){
    char* ptr = buf;

    this->pid = *((uint64_t*)ptr);
    ptr += sizeof(this->pid);

    //this->rid = *((uint64_t*)ptr);
    //ptr += sizeof(this->rid);

    this->nrow = *((uint64_t*)ptr);
    ptr += sizeof(this->nrow);

    string s(ptr, ANUM);
    bitset<ANUM> b(s);
    this->cols = b;
    this->ncol = this->cols.count();
    ptr += ANUM;

    this->data = (T*)ptr;
    this->buf = buf;

    //col_off
    int t = 0;
    for(int i = 0; i < ANUM; i++){
        col_off[i] = -1;
        if(this->cols[i] == 1)
            col_off[i] = t++;
    }

}

template<typename T>
Partition<T>::~Partition(){
    if(this->buf)
        delete[] this->buf;
}

template<typename T>
int Partition<T>::writeToFile(const char* filename){
    uint64_t buf_size = sizeof(uint64_t) * 2 //pid, nrow
        + ANUM + sizeof(T) * nrow * ncol;

    char* buf = new char[buf_size];

    uint64_t c = 0;
    memcpy(buf + c, &this->pid, sizeof(this->pid));
    c += sizeof(this->pid);

    //memcpy(buf + c, &this->rid, sizeof(this->rid));
    //c += sizeof(this->rid);

    memcpy(buf + c, &this->nrow, sizeof(this->nrow));
    c += sizeof(this->nrow);

    memcpy(buf + c, this->cols.to_string().c_str(), ANUM);
    c += ANUM;

    memcpy(buf + c, this->data, sizeof(T)*nrow*ncol);
    c += sizeof(T)*nrow*ncol;

   
    ofstream ofile(filename, ios::binary);
    ofile.write(buf, c);
    ofile.close();

    delete[] buf;
}


template<typename T>
Partition<T>* Partition<T>::readFromFile(const char* filename){
    
    ifstream ifile(filename, ios::binary);
    
    streampos begin, end;
    begin = ifile.tellg();
    ifile.seekg(0, ios::end);
    end = ifile.tellg();

    uint64_t size = end - begin;
    char* buf = new char[size];
    ifile.seekg(0, ios::beg);
    ifile.read(buf, size);
    
    if(ifile.gcount() != size){
        delete[] buf;
        ifile.close();
        return NULL;
    }

    ifile.close();

    Partition<T>* part = new Partition<T>(buf, size);
    return part;
}

template class Partition<VTYPE>;
