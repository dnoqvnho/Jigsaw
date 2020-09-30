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
#include "data_load.h"
#include "partition_schema.h"
#include "../storage/partition.h"
#include "../storage/index.h"
#include <string>
#include <fstream>
#include <vector>
#include <bitset>
using namespace std;

bool load(const char* datafile, const char* outdir, PType type
        , const char* partition ){


    PSchema schema(type, partition);
    int sv[ANUM];
    schema.getVSchema(sv);
    uint64_t vnum = schema.getVNum();
    uint64_t pnum = schema.getPNum();

    vector<VTYPE>* data = new vector<VTYPE>[pnum];

    ifstream ifile(datafile, ios::binary);

    uint64_t btnum = 1024*1024;
    uint64_t btsize = btnum * ANUM;
    VTYPE* buffer = new VTYPE[btsize];

    uint64_t pids[vnum];
    uint64_t tnum = 0;
    do{
        //read tuples into a buffer
        ifile.read((char*) buffer, btsize * sizeof(VTYPE));
        uint64_t len = ifile.gcount();
        if(len % (sizeof(VTYPE) * ANUM) != 0){
            delete[] buffer;
            ifile.close();
            return false;
        }
        len = len / sizeof(VTYPE) / ANUM;

        //partition tuples in the buffer
        VTYPE* ptr;
        for(uint64_t i = 0; i < len; i++){
            ptr = buffer + i * ANUM;

            schema.getPids(tnum, pids);
            for(int j = 0; j < ANUM; j++){
                uint64_t p = pids[sv[j]];
                data[p].push_back(ptr[j]);
            }
            tnum ++;
        }
    }while(tnum < TNUM);


    for(uint64_t i = 0; i < pnum; i++){

        bitset<ANUM> b = schema.getVPartition(i);
        Partition<VTYPE> p(i, b, data[i].data(), data[i].size());
        string opath = string(outdir) + "/" + std::to_string(i);
        p.writeToFile(opath.c_str());
    }

    //write the index file
    Index* index = schema.createIndex();
    string ipath = string(outdir) + "/index";
    index->writeToFile(ipath.c_str());

    delete index;
    ifile.close();
    delete[] buffer;
    delete[] data;
    return true;
}
