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
#ifndef INDEX
#define INDEX

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <cstring>
#include <set>
#include <utility>
#include "../storage/constants.h"
using namespace std;

typedef pair<int, vector<int> > TupleIndex;

class Index {
public:
	vector<vector<int> > attr_index;
	vector<vector<int> > schema_index;
	vector<TupleIndex> tuple_index;
	uint64_t p_num;
	uint64_t t_num;

	Index(uint64_t p_num, vector<vector<int> > attr_index,
		vector<vector<int> > schema_index, vector<TupleIndex> tuple_index);

	void serialize(string filename);
	static Index *deserialize(string filename);

	set<int> getPartitionByAttr(vector<int>attrs);
	int getPartitionByCell(uint64_t tid, int aid);

	string to_string();
};

inline int Index::getPartitionByCell(uint64_t tid, int aid){
    int s = tuple_index[tid].first;
    int p_offset = schema_index[s][aid];
    return (tuple_index[tid].second)[p_offset];
}

#endif
