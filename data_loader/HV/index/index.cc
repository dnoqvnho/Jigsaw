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
#include <sstream>
#include "index.h"
#include "../storage/partition.h"

Index::Index(uint64_t p_num, vector<vector<int> > attr_index,
		vector<vector<int> > schema_index, vector<TupleIndex> tuple_index) {
	this->p_num = p_num;
	this->attr_index = attr_index;
	this->schema_index = schema_index;
	this->tuple_index = tuple_index;
	this->t_num = tuple_index.size();
}


void Index::serialize(string filename) {
	uint64_t size = sizeof(uint64_t); // p_num
	for (int i = 0; i < ANUM; i++) {
		size += sizeof(int);	//# of attr value
		size += attr_index[i].size() * sizeof(int);
	}
	size += sizeof(int);	//# of schema_index
	for (int i = 0; i < schema_index.size(); i++) {
		size += ANUM * sizeof(int);
	}
	for (int i = 0; i < TNUM; i++) {
		size += sizeof(int);
		size += sizeof(int);
		size += tuple_index[i].second.size() * sizeof(int);
	}

	// write to buffer

	char *buffer = new char[size];
	uint64_t c = 0;
	memcpy(buffer + c, &p_num, sizeof(uint64_t));
	c += sizeof(uint64_t);

	for (int i = 0; i < ANUM; i++) {
		int v_size = attr_index[i].size();
		memcpy(buffer + c, &v_size, sizeof(int));
		c += sizeof(int);
		memcpy(buffer + c, attr_index[i].data(), v_size * sizeof(int));
		c += v_size * sizeof(int);
	}

	int schema_index_size = schema_index.size();
	memcpy(buffer + c, &schema_index_size, sizeof(int));
	c += sizeof(int);

	for (int i = 0; i < schema_index_size; i++) {	
		memcpy(buffer + c, schema_index[i].data(), ANUM * sizeof(int));
		c += ANUM * sizeof(int);	
	}
	for (int i = 0; i < TNUM; i++) {
		int schema_id = tuple_index[i].first;
		memcpy(buffer + c, &schema_id, sizeof(int));
		c += sizeof(int);

		int t_size = tuple_index[i].second.size();
		memcpy(buffer + c, &t_size, sizeof(int));
		c += sizeof(int);

		memcpy(buffer + c, tuple_index[i].second.data(), t_size * sizeof(int));
		c += t_size * sizeof(int);
	}
	ofstream outfile(filename.c_str(), ios::binary);
	outfile.write(buffer, size);
	outfile.close();
	delete[] buffer;
}


Index * Index::deserialize(string filename) {
	ifstream infile(filename.c_str(), ios::binary);
	infile.seekg(0, ios::end);
	size_t size = infile.tellg();
	infile.seekg(0, ios::beg);

	char *buffer = new char[size];
	char *c = buffer;
	infile.read(buffer, size);
    if(infile.gcount() != size){
        infile.close();
        cout << "initialize error" << endl;
    }
    infile.close();

    vector<vector<int> > attr_index(ANUM);

    int p_num = *(uint64_t*)c;
    c += sizeof(uint64_t);

    for (int i = 0; i < ANUM; i++) {
    	int v_size = *(int*)c;
    	c += sizeof(int);
    	vector<int> attr(v_size);
    	for (int j = 0; j < v_size; j++) {
    		attr[j] = *(int*)c;
    		c += sizeof(int);
    	}
    	attr_index[i] = attr;
    }

    int schema_index_size = *(int*)c;
    vector<vector<int> > schema_index(schema_index_size);
    c += sizeof(int);
    for (int i = 0; i < schema_index_size; i++) {
    	int s_size = ANUM;

    	vector<int> schema(ANUM);
    	for (int j = 0; j < s_size; j++) {
    		schema[j] = *(int*)c;
    		c += sizeof(int);
    	}
    	schema_index[i] = schema;
    }

    vector<TupleIndex> tuple_index(TNUM);
    for (uint64_t i = 0; i < TNUM; i++) {
    	int schema_id = *(int*)c;
    	c += sizeof(int);
    	
    	int p_num = *(int*)c;
    	c += sizeof(int);

    	vector<int> partitions(p_num);
    	for (int j = 0; j < p_num; j++) {
    		partitions[j] = *(int*)c;
    		c += sizeof(int);
    	}
    	TupleIndex tindex = make_pair(schema_id, partitions);
    	tuple_index[i] = tindex;
    }

    delete[] buffer;
    return new Index(p_num, attr_index, schema_index, tuple_index);
}

set<int> Index::getPartitionByAttr(vector<int>attrs) {
	set<int> re;
	for (int i = 0; i < attrs.size(); i++) {
		vector<int> p_ids = attr_index[attrs[i]];
		for (int j = 0; j < p_ids.size(); j++) {
			re.insert(p_ids[j]);
		}
	}
	return re;
}

string Index::to_string() {
	stringstream buffer;
	buffer << "attr_index: " << endl;
	for (int i = 0; i < attr_index.size(); i++) {
		buffer << "\tattr: " << i <<endl;
		buffer << "\t\t";
		for (int j = 0; j < attr_index[i].size(); j++) {
			buffer << attr_index[i][j] << ",";
		}
		buffer << endl;
	}
	buffer << "schema_index: " << endl;
	for (int i = 0; i < schema_index.size(); i++) {
		buffer << "\tschema: " << i <<endl;
		buffer << "\t\t";
		for (int j = 0; j < schema_index[i].size(); j++) {
			buffer << schema_index[i][j] << ",";
		}
		buffer << endl;
	}
	buffer << "tuple_index: " << endl; 
	for (int i = 0; i < tuple_index.size(); i++) {
		buffer << "\t";
		buffer << "t_id: " << i  << ", schema:" << tuple_index[i].first <<endl;
		buffer << "\tpartitions: " << endl;
		buffer << "\t\t";
		for (int j = 0; j < tuple_index[i].second.size(); j++) {
			buffer << tuple_index[i].second[j] << ",";
		}
		buffer << endl;
	}
	return buffer.str();
}
