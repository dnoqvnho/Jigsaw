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
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <assert.h>

#include <sys/time.h>

#include "constants.h"
#include "tuples.h"
#include "query.h"
#include "graph_partitioner.h"

using namespace std;

int main(const int argc, const char* argv[]){
   
    cout << "Column-H TNUM " << TNUM << " QNUM " << QNUM << endl;

    vector<Query<VTYPE>*> queries;
    string line;
    ifstream qfile(argv[1]);
    while(std::getline(qfile, line)){
        Query<VTYPE>* q = Query<VTYPE>::parse_string(line);
        queries.push_back(q);
    }
    qfile.close();

    Tuples<VTYPE> tuples(queries);

    ifstream dfile(argv[2], ios::binary);
    uint64_t buf_tnum = 1000*1000;
    VTYPE* buf = new VTYPE[buf_tnum * ANUM];
    uint64_t process = 0;
    do{
        cout << "Process " << process/1000000 << "M tuples" << endl;
        dfile.read((char*) buf, sizeof(VTYPE)*buf_tnum*ANUM);
        uint64_t rtnum = dfile.gcount() / sizeof(VTYPE) / ANUM;
        tuples.push(buf, rtnum);     
        
        process += rtnum;
        if(rtnum != buf_tnum)
            break;
        if(process == TNUM)
            break;
    }while(true);
    dfile.close();
    delete[] buf;
    assert(process == TNUM);

    GPartitioner<VTYPE> partitioner(&tuples, PNUM);
    vector<uint64_t> parts;
    partitioner.partition(parts);
    assert(parts.size() == TNUM);

    ofstream ofile(argv[3]);
    for(uint64_t i = 0; i < parts.size(); i++){
        ofile << parts[i] << " " << i << endl; 
    }
    ofile.close();

    
    return 0;
}
