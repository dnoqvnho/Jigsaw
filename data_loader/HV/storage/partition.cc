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
#include "partition.h"

template<typename T>
Partition<T>::Partition(int partition_id, const bitset<ANUM> &cols, vector<Segment<T>*> *segments) {
	this->partition_id = partition_id;
	this->cols = cols;
	this->segments = segments;
	this->seg_num = (*segments).size();
	this->tuple_num = 0;
	for (auto s: (*segments)) {
		this->tuple_num += s->getTupleNum();
	}
}

template<typename T>
Partition<T>::Partition(int partition_id, vector<Segment<T>*> *segments) {
	this->partition_id = partition_id;
	bitset<ANUM> cols;
	this->segments = segments;
	this->seg_num = (*segments).size();
	this->tuple_num = 0;
	for (auto s: (*segments)) {
		this->tuple_num += s->getTupleNum();
		cols |= s->getColumns();
	}
	this->cols = cols;
}

template<typename T>
Partition<T>::~Partition() {
	for (auto s: *(this->segments)) {
		delete s;
	}
	delete this->segments;
}

template<typename T>
Partition<T>::Partition(char* buffer) {			// parse from a binary file

    this->partition_id = *(int*)buffer;		// partition_id
    buffer += sizeof(int);

    string s(buffer, ANUM);
    bitset<ANUM> cols(s);
    this->cols = cols;						// bitmap
    buffer += ANUM; 

    this->seg_num = *(uint64_t*)buffer;			// number of segments
    buffer += sizeof(uint64_t);


    this->tuple_num = *(uint64_t*)buffer;		//number of tuples
    buffer += sizeof(uint64_t);

    this->segments = new vector<Segment<T>*>(this->seg_num);
    for (uint64_t i = 0; i < this->seg_num; i++) {
    	Segment<T> *new_segment = new Segment<T>(buffer);
    	(*(this->segments))[i] = new_segment;
    }
}

template<typename T>
Partition<T>* Partition<T>::parseRegularTable(char* buffer, uint64_t size) {
    int p_id = 0;
    bitset<ANUM> cols;
    cols.set();

    uint64_t seg_num = 1;
    uint64_t tuple_num = size / (ANUM * sizeof(T));
    uint64_t seg_id = 0;
    vector<Tuple<T>*> *tuples = new vector<Tuple<T>*>(tuple_num);
    for (int i = 0; i < tuple_num; i++) {
    	Tuple<T> *t = new Tuple<T>();
    	t->tuple_id = i;
    	t->vals = (T*)buffer;

    	buffer += sizeof(T) * ANUM;
    	(*tuples)[i] = t;
    }
    Segment<T> *s = new Segment<T>(seg_id, cols, tuples);
    vector<Segment<T>*> *segs = new vector<Segment<T>*>{s};
    Partition *p = new Partition(p_id, cols, segs);
    return p;
}


template<typename T>
int Partition<T>::getPartitionId() {
	return this->partition_id;
}

template<typename T>
bitset<ANUM> Partition<T>::getColumns() {
	return this->cols;
}

template<typename T>
int Partition<T>::writeToFile(string filename) {
	uint64_t meta_buffer_size;

	meta_buffer_size = sizeof(this->partition_id) + ANUM + 
						sizeof(this->seg_num) + sizeof(this->tuple_num);		//size of metadata
	// construct metadata buffer
	char *meta_buffer = new char[meta_buffer_size];
	uint64_t c = 0;
	memcpy(meta_buffer + c, &this->partition_id, sizeof(this->partition_id));	// partition_id
	c += sizeof(this->partition_id);
	memcpy(meta_buffer + c, this->cols.to_string().c_str(), ANUM);	// bitmap of columns
	c += ANUM;
	memcpy(meta_buffer + c, &this->seg_num, sizeof(this->seg_num));		// number of segments
	c += sizeof(this->seg_num);
	memcpy(meta_buffer + c, &this->tuple_num, sizeof(this->tuple_num));	// number of tuples
	// c += sizeof(this->tuple_num);
	// uint64_t seg_postition = c;
	// for (uint64_t i = 0; i < *(this->seg_num); i++) {
	// 	memcpy(meta_buffer + c, &seg_position, sizeof(seg_position));	//start postition of seg
	// 	c += sizeof(seg_position);

	// 	// compute next position
	// 	seg_position += 64;		// segment_id
	// 	seg_position += ANUM;	// col's bitset
	// 	seg_position += 64;		// number of tuples
	// 	seg_position += ((*(this->segments))[i]->getColNum() * sizeof(T) + 64) 
	// 						* (*(this->segments))[i]->getTupleNum();	// tuple_id + values
	// }

	// open file and write metadata
	// int f;
	// f = open(filename, O_WRONLY|O_CREAT);
	// if (f == -1) {
	// 	cout << "open file error" << endl;
	// 	return 1;
	// }
	// ssize_t meta_out = write(f, meta_buffer, (ssize_t)meta_buffer_size);
	// if (meta_out != (ssize_t) meta_buffer_size) {
	// 	cout << "write metadata error" << endl;
	// 	return 2;
	// }
	// close(f);


	ofstream outfile(filename, ios::binary);
	outfile.write(meta_buffer, meta_buffer_size);
	outfile.close();
	delete[] meta_buffer;

	// call each segment's write to write values
	for (uint64_t i = 0; i < this->seg_num; i++) {
		int ret = (*(this->segments))[i]->writeToFile(filename);
		if (ret != 0) {
			cout << "write segment " << i <<" error" <<endl;
			return 3;
		}
	}
	return 0;
}

template<typename T>
vector<Segment<T>*> *Partition<T>::getSegments() {
	return this->segments;
}

template<typename T>
string Partition<T>::to_string() {
	stringstream buffer;
	buffer << "partition_id: " << this->getPartitionId()<< endl;
	buffer << "bitmap: " << this->getColumns() << endl;
	buffer << "Segments: " <<endl;
	for (auto s : *(this->getSegments())) {
		buffer << "\tsegment_id: " << s->getSegId() <<endl;
		buffer << "\tbitmap: " << s->getColumns() <<endl;
		buffer << "\tTuples: " <<endl;
		for (auto t : *(s->getTuples())) {
			buffer << "\t\ttuple_id: " << t->tuple_id <<endl;
			buffer << "\t\tvalues: [";
			for (int i = 0; i < s->getColNum(); i++) {
				buffer << *(t->vals + i) << ",";
			}
			buffer << "]" <<endl;
		}
	}
	return buffer.str();
}

template class Partition<VTYPE>;