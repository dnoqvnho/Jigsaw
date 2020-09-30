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
#include <bitset>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <sys/time.h>

#include "constants.h"
#include "query.h"

using namespace std;

vector<Query<VTYPE>*> parseQuery(const char* query_path);

int main(const int argc, const char* argv[]){
    if(argc < 3){
        printf("query file, output file\n");
        return -1;
    } 

    cout << "Row-V " << " QNUM " << QNUM << endl;

    vector<Query<VTYPE>*> pq = parseQuery(argv[1]); 
   
    struct timeval start, end;
    gettimeofday(&start, NULL); 
    unordered_map<bitset<ANUM>, int> num;
    for(auto q : pq){
        bitset<ANUM> attrs = q->selection | q->projection;
        if(num.count(attrs) == 0)
            num[attrs] = 0;
        num[attrs]++;
    }

    for(auto it = num.begin(); it != num.end(); ++it){
        bitset<ANUM> attrs = it->first;
        unsigned int size = sizeof(VTYPE)*attrs.count();
        it->second *= size;
    }


    vector<string> parts;
    bitset<ANUM> rem;
    rem.set();

    while(num.size() > 0){
        int max_size = -1;
        bitset<ANUM> max_attr;
        for(auto it = num.begin(); it != num.end(); it++)
            if(it->second > max_size){
                max_attr = it->first;
                max_size = it->second;
            }
        num.erase(max_attr);

        max_attr &= rem;
        
        rem = rem & (~max_attr);
        
        if(max_attr.count()){
            string attr_str = max_attr.to_string();
            reverse(attr_str.begin(), attr_str.end());
            parts.push_back(attr_str);
        }
    }

    if(rem.count()){
        string rem_str = rem.to_string();
        reverse(rem_str.begin(), rem_str.end());
        parts.push_back(rem_str);
    }


    gettimeofday(&end, NULL);
    unsigned long long t = 1000000 * ( end.tv_sec - start.tv_sec )
            + end.tv_usec -start.tv_usec;

    cout << "Number of partitions: " << parts.size() << endl;
    cout << "Total time: " << t << " microseconds" << endl;

    ofstream ofile(argv[2]);
    
    for(auto s : parts)
        ofile << s << endl;

    ofile.close();
    for(auto q : pq)
        delete q;
    return 0;
}

vector<Query<VTYPE>*> parseQuery(const char* query_path){
    vector<Query<VTYPE>*> pq;
    ifstream qfile(query_path);

    string line;
    while(getline(qfile, line)){
        Query<VTYPE>* q = Query<VTYPE>::parse_string(line);
        pq.push_back(q);
    }
    qfile.close();
    return pq;
}
