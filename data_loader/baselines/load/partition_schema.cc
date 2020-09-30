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
#include "partition_schema.h"
#include <fstream>
#include <bitset>
#include <string>
#include <bits/stdc++.h>
#include <math.h>
#include <vector>

PSchema::PSchema(PType pt, const char* filename){
    //construct the col_off
    this->psize = NULL;
    this->rpnum = NULL;
    if(pt == PType::Row){
        for(int i = 0; i < ANUM; i++)
            col_off[i] = 0;
        this->cpnum = 1;
        this->psize = new uint64_t[1];
        this->psize[0] = PSIZE / sizeof(VTYPE) / ANUM; 
        this->rpnum = new uint64_t[1];
        this->rpnum[0] = 0;
    }else if(pt == PType::Column || pt == PType::Column_H){
        for(int i = 0; i < ANUM; i++)
            col_off[i] = i;
        this->cpnum = ANUM;
        if(pt == PType::Column){
            this->psize = new uint64_t[ANUM];
            this->rpnum = new uint64_t[ANUM];
            for(int i = 0; i < ANUM; i++){
                this->psize[i] = PSIZE / sizeof(VTYPE);
                this->rpnum[i] = ceil(TNUM / (double) this->psize[i]);
            }
            this->rpnum[0] = 0;
            for(int j = 1; j < ANUM; j++)
                this->rpnum[j] += this->rpnum[j-1];
        }
    }else if(pt == PType::Row_V){
        this->psize = new uint64_t[ANUM];
        this->rpnum = new uint64_t[ANUM];
        string str;
        ifstream ifile(filename);
        int voff = 0;
        uint64_t tt[ANUM];
        while(getline(ifile, str)){
            reverse(str.begin(), str.end());
            bitset<ANUM> b(str);
            for(int i = 0; i < ANUM; i++)
                if(b[i] == 1)
                    col_off[i] = voff;
            this->psize[voff] = PSIZE / sizeof(VTYPE) / b.count();
            tt[voff] = ceil(TNUM / (double)this->psize[voff]);
            voff++;
        }
        this->cpnum = voff;
        this->rpnum[0] = 0;
        for(int j = 1; j < voff; j++)
            this->rpnum[j] = this->rpnum[j-1] + tt[j-1];
        ifile.close();
    }
    
    row_off = NULL;
    if(pt == PType::Column_H){
        row_off = new uint64_t[TNUM];
        this->rpnum = new uint64_t[ANUM];
        this->rpnum[1] = 0;
        ifstream ifile(filename);
        for(uint64_t i = 0; i < TNUM; i++){
            uint64_t tid, pid;
            ifile >> pid >> tid;
            row_off[tid] = pid;
            this->rpnum[1] = this->rpnum[1] > pid? this->rpnum[1] : pid;
        }
        
        ifile.close();
        this->rpnum[1]++;
        for(int i = 0; i < ANUM; i++)
            this->rpnum[i] = this->rpnum[1] * i;
    }
    this->type = pt;
}

PSchema::~PSchema(){
    if(row_off != NULL)
        delete[] row_off;
    if(psize != NULL)
        delete[] psize;
    if(rpnum != NULL)
        delete[] rpnum;
}

void PSchema::getPids(uint64_t tid, uint64_t pids[]){
   
    if(type != PType::Column_H){
        for(int i = 0; i < cpnum; i++)
            pids[i] = this->rpnum[i] + tid / this->psize[i];
    }else{
        for(int i = 0; i < ANUM; i++)
            pids[i] = this->rpnum[i] + this->row_off[tid];
    }

}

bitset<ANUM> PSchema::getVPartition(uint64_t pid){
    uint64_t voff = 0;
    for(; voff < cpnum; voff++)
        if(rpnum[voff] > pid)
            break;
    voff--;
    bitset<ANUM> b;
    for(int i = 0; i < ANUM; i++)
        if(col_off[i] == voff)
            b[i] = 1;
    return b;
}


void PSchema::getVSchema(int sv[]){
    for(int i = 0; i < ANUM; i++)
        sv[i] = col_off[i];
}

uint64_t PSchema::getPNum(){
    uint64_t num = 0;
    if(type == PType::Row){
        num = ceil(TNUM / (double)this->psize[0]);
    }else if(type == PType::Column || type == PType::Column_H){
        num = ANUM * this->rpnum[1];
    }else{
        for(int i = 0; i < cpnum; i++)
            num += ceil(TNUM / (double)psize[i]);
    }
    return num;
}

Index* PSchema::createIndex(){

    vector<bitset<ANUM>> cols;
    vector<vector<uint64_t>> pids;
    vector<vector<uint64_t>> off;

    uint64_t hsize[rpnum[1]];
    if(type == PType::Column_H){
        for(uint64_t i = 0; i < rpnum[1]; i++)  
            hsize[i] = 0;
        for(uint64_t i = 0; i < TNUM; i++)
            hsize[row_off[i]] ++;      
    }

    for(uint64_t i = 0; i < cpnum; i++){
        bitset<ANUM> b;
        for(int j = 0; j < ANUM; j++)
            if(col_off[j] == i)
                b[j] = 1;
        cols.push_back(b);

        vector<uint64_t> p;
        vector<uint64_t> o;
        uint64_t base = rpnum[i];
        if(type != PType::Column_H){
            for(uint64_t tid = 0; tid < TNUM; tid+=psize[i]){
                uint64_t a = tid / psize[i] + base;
                p.push_back(a);
                o.push_back(tid);
            }
        }else{
            uint64_t hnum = rpnum[1]; // the number h partitions
            uint64_t k = 0;
            for(uint64_t tid = 0; tid < TNUM; ){
                p.push_back(base);
                base++;
                o.push_back(tid);
                tid += hsize[k++];
            }
        }
        o.push_back(TNUM);
        pids.push_back(p);
        off.push_back(o);
    }
    return new Index(cols, pids, off);
}
