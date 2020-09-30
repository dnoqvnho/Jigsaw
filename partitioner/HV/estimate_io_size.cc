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
#include <vector>
#include <fstream>

#include "query.h"
#include "segment.h"

using namespace std;

const VTYPE min_val = 0;
const VTYPE max_val = 100000000;

void estimate_tnum(Segment<VTYPE>& s){
    double ratio = 1;
    for(int i = 0; i < s.ranges.size() / 2; i++){
        VTYPE l = s.ranges[2*i + 1] - s.ranges[2*i] - 1
            + s.closed[2*i] + s.closed[2*i+1];
        ratio *= (double)l / (max_val - min_val);
    }
    s.tnum = TNUM * ratio;
}

int main(const int argc, const char* argv[]){
    ifstream sfile(argv[1]);
    vector<vector<Segment<VTYPE>>> segments;
    string line;
    
    while(getline(sfile, line)){
        size_t found = line.find_first_of(" ");
        int pid = stoi(line.substr(0, found));
        if(pid >= segments.size())
            segments.resize(pid+1);
        Segment<VTYPE> s = Segment<VTYPE>::parse_string(line.substr(found + 1));
        estimate_tnum(s);
        segments[pid].push_back(s);
    }
    sfile.close();

    ifstream qfile(argv[2]);
   
    uint64_t io_size = 0;
    uint64_t data_size = 0;
    uint64_t id_size = 0;
    while(getline(qfile, line)){
        Query<VTYPE>* q = Query<VTYPE>::parse_string(line);
        
        for(auto& part: segments)
            for(auto& s: part)
                if(s.intersect(q)){
                    for(auto& t: part){
                        io_size += t.get_size();
                        data_size += t.get_data_size();
                        id_size += t.get_id_size();
                    }
                    break;
                }
        delete q;
    }
    
    qfile.close();

    printf("I/O size is %d GB\n", io_size/1024/1024/1024);
    printf("Actual data size is %d GB\n", data_size/1024/1024/1024);
    printf("TID size is %d GB\n", id_size/1024/1024/1024);
    return 0;
}
