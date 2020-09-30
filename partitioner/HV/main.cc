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
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sys/time.h>

#include "constants.h"
#include "segment.h"
#include "query.h"
#include "partitioner.h"

using namespace std;

template class std::vector<Query<VTYPE>*>;

int main(const int argc, const char* argv[]){

    if(argc < 4){
        cout << "Inputs table description, query description, and output file" << endl;
        return -1;
    }

    cout << "HV TNUM " << TNUM << " QNUM " << QNUM << endl;

    //read the table description file
    ifstream tfile(argv[1]);
    uint64_t fake_pid;
    tfile >> fake_pid;

    string line;
    std::getline(tfile, line);
    Segment<VTYPE> table = Segment<VTYPE>::parse_string(line);
    tfile.close();


    //read the query file
    vector<Query<VTYPE>*> pq;
    ifstream qfile(argv[2]);

    while(std::getline(qfile, line)){
        Query<VTYPE>* q = Query<VTYPE>::parse_string(line);
        pq.push_back(q);
    }
    qfile.close();

    //add the queries to the table segment
    table.add_query(pq);


    struct timeval start, end;
    gettimeofday(&start, NULL); 
    
    //partition the table
    vector<vector<Segment<VTYPE>*>> partitions;
    double cost = Partitioner<VTYPE>::partition(&table, partitions);
    
    gettimeofday(&end, NULL); 
    unsigned long long t = 1000000*(end.tv_sec - start.tv_sec )
            + end.tv_usec -start.tv_usec;
    cout << "Total time: " << t << " micorseconds" << endl;
    cout << "Number of partitions: " << partitions.size() << endl;
    
    //write to output file
    ofstream ofile(argv[3]);
    for(uint64_t pid = 0; pid < partitions.size(); pid++){
        for(auto s : partitions[pid]){
            ofile << pid << " ";
            ofile << s->to_string() << endl;
            if(s != &table)
                delete s;
        }
    }
    ofile.close();

    cout << "Estimated cost: " << cost << "microseconds" << endl;

    for(auto q : pq)
        delete q;

    return 0;
}
