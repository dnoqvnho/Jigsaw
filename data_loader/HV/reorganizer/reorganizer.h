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
#ifndef REORGANIZER
#define REORGANIZER

#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include <unordered_map>
#include "../storage/partition.h"

using namespace std;

template<typename T>
using vpair = pair<bitset<ANUM>, T*>;

template<typename T>
using Idmap = unordered_map<uint64_t, vector<vpair<T>>>;

template<typename T>
using Colmap = unordered_map<bitset<ANUM>, vector<Tuple<T>*>*>;

template<typename T>
struct Range
{
	T r_min;
	T r_max;
	int o_min;
	int o_max;
	Range(T x=0, T y=99999999, int o=0, int p=0) {
		r_min = x;
		r_max = y;
		o_min = o;
		o_max = p;
	}
	bool satisfy(T t) {
		if (t > r_min && t < r_max) {
			return true;
		}
		else if (t == r_min && o_min == 1) {
			return true;
		}
		else if (t == r_max && o_max == 1) {
			return true;
		}
		return false;
	}
};

template<typename T>
class Layout {
public:
	vector<Range<T>> ranges;
	bitset<ANUM> cols;
	int p_id;
	int a_num;
	Layout(vector<Range<T>> ranges, bitset<ANUM> cols, int p_id);
	string to_string();
	static Layout<T> parse(string s);
};

template<typename T>
vector<Segment<T>*> *evaluate(Partition<T> *p, vector<Layout<T>> layouts);

template<typename T>
vector<Segment<T>*> *transfer(vector<Segment<T>*> *segs);

template<typename T>
void reorganize(string partition_file, string dest, string table_file, string index_file);

template<typename T>
void test();

#endif
