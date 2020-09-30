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
#ifndef SEGMENT
#define SEGMENT

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <bitset>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <cstring>
#include "constants.h"

using namespace std;



template<typename T>
struct Tuple
{
	uint64_t tuple_id;
	T *vals;
};


template<typename T>
class Segment {
private:

	uint64_t segment_id;
	bitset<ANUM> cols;
	vector<Tuple<T>*> *tuples;
	uint64_t tuple_num;

public:
	Segment(uint64_t segment_id, const bitset<ANUM> &cols, vector<Tuple<T>*> *tuples);
	Segment(char* &buffer);				//initialize with a buffer
	~Segment();

	int writeToFile(string filename);	// write to disk file
	vector<Tuple<T>*>* getTuples();
	bitset<ANUM> getColumns();
	uint64_t getTupleNum();
	int getColNum();
	uint64_t getSegId();

	int getPosition(int attr);

};




#endif