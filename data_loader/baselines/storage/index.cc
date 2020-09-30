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
#include "index.h"
#include <fstream>
//#include <sstream>
#include <string>
using namespace std;

Index::Index(vector<bitset<ANUM>> cols, vector<vector<uint64_t>> pids
        , vector<vector<uint64_t>> off): cols(cols), pids(pids), off(off){
    this->ncol = cols.size();
}

void Index::writeToFile(const char* filename){
    ofstream ofile(filename);
    uint64_t ncol = cols.size();
    ofile << ncol << endl;

    //write the vertical partitioning
    for(auto& c : cols)
        ofile << c.to_string() << endl;

    //write the horizontal partitioning
    for(uint64_t i = 0; i < ncol; i++){
        ofile << pids[i].size() << endl; // the number of h partitions
        for(uint64_t j = 0; j < pids[i].size(); j++){
            ofile << pids[i][j] << " " << off[i][j + 1] << endl;
        }
    }
    
    ofile.close();
}

Index* Index::readFromFile(const char* filename){
    ifstream ifile(filename);

    uint64_t ncol;
    ifile >> ncol;
    
    vector<bitset<ANUM>> cols;
    vector<vector<uint64_t>> pids;
    vector<vector<uint64_t>> off;
    cols.reserve(ncol);
    pids.reserve(ncol);
    off.reserve(ncol);

    for(uint64_t i = 0; i < ncol; i++){
        string str;
        ifile >> str;
        bitset<ANUM> b(str);
        cols.push_back(b);
    }

    for(uint64_t i = 0; i < ncol; i++){
        uint64_t nrow;
        ifile >> nrow;

        vector<uint64_t> pv, po;
        pv.reserve(nrow);
        po.reserve(nrow + 1);
        po.push_back(0);

        for(uint64_t j = 0; j < nrow; j++){
            uint64_t p, of;
            ifile >> p >> of;
            pv.push_back(p);
            po.push_back(of);
        }
        pids.push_back(pv);
        off.push_back(po);
    }

    ifile.close();

    Index* idx = new Index(cols, pids, off);
    return idx;
}
