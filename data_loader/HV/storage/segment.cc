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
#include "segment.h"
#include <assert.h>

template<typename T>
Segment<T>::Segment(uint64_t segment_id, const bitset<ANUM> &cols, vector<Tuple<T>*> *tuples) {
	this->segment_id = segment_id;
	this->cols = cols;
	this->tuples = tuples;
	this->tuple_num = (*tuples).size();
}

template<typename T>
Segment<T>::Segment(char* &buffer) {
	this->segment_id = *(uint64_t*)buffer;
	buffer += sizeof(uint64_t);

	string s(buffer, ANUM);
    bitset<ANUM> cols(s);
	this->cols = cols;
	buffer += ANUM;

	int col_num = this->getColNum();	

	this->tuple_num = *(uint64_t*)buffer;
	buffer += sizeof(uint64_t);

	this->tuples = new vector<Tuple<T>*>(this->tuple_num);
	for (uint64_t i = 0; i < this->tuple_num; i++) {
		Tuple<T> *t = new Tuple<T>();
		t->tuple_id = *(uint64_t*)buffer;		// tuple_id
		buffer += sizeof(uint64_t);

		t->vals = (T*)buffer;					// values
		buffer += sizeof(T) * col_num;

		(*(this->tuples))[i] = t;
	}
}

template<typename T>
Segment<T>::~Segment() {
	for (auto t: *(this->tuples)) {
		delete t;
	}
	delete this->tuples;
}

template<typename T>
uint64_t Segment<T>::getSegId() {
	return this->segment_id;
}

template<typename T>
vector<Tuple<T>*> *Segment<T>::getTuples() {
	return this->tuples;
}

template<typename T>
bitset<ANUM> Segment<T>::getColumns() {
	return this->cols;
}

template<typename T>
uint64_t Segment<T>::getTupleNum() {
	return this->tuple_num;
}

template<typename T>
int Segment<T>::getColNum() {
	int num = 0;
	for (int i = 0; i < this->cols.size(); i++) {
		if (this->cols[i] == 1) {
			num++;
		}
	}
	return num;
}

template<typename T>
int Segment<T>::getPosition(int attr) {
	assert(this->cols[attr] == 1);
	int pos = 0;
	for (int i = 0; i < attr; i++) {
		if (i != attr && this->cols[i] == 1) {
			pos++;
		}
		if (i == attr){
			return pos;
		}
	}
}

template<typename T>
int Segment<T>::writeToFile(string filename) {
	uint64_t meta_buffer_size;
	int col_num = this->getColNum();


	meta_buffer_size = sizeof(this->segment_id) + ANUM + 
						sizeof(this->tuple_num); 	
						//size of data
	// construct buffer
	char *meta_buffer = new char[meta_buffer_size];
	uint64_t c = 0;
	memcpy(meta_buffer + c, &this->segment_id, sizeof(this->segment_id));	// segment_id
	c += sizeof(this->segment_id);
	memcpy(meta_buffer + c, this->cols.to_string().c_str(), ANUM);	// bitmap of columns
	c += ANUM;
	memcpy(meta_buffer + c, &this->tuple_num, sizeof(this->tuple_num));		// number of tuples

	ofstream outfile(filename, ios::binary | ios::app);
	outfile.write(meta_buffer, meta_buffer_size);

	delete[] meta_buffer;

	uint64_t buffer_size = col_num * sizeof(T) + sizeof(uint64_t);
	char *buffer = new char[buffer_size];
	for (uint64_t i = 0; i < this->tuple_num; i++) {
		c = 0;
		memcpy(buffer + c, &((*(this->tuples))[i]->tuple_id), sizeof(uint64_t));	// tuple id
		c += sizeof(uint64_t);
		
		memcpy(buffer + c, (*(this->tuples))[i]->vals, sizeof(T) * col_num);	// values

		outfile.write(buffer, buffer_size);
	
	}
	// open file and write metadata
	outfile.close();
	delete[] buffer;
	return 0;
}


template class Segment<VTYPE>;