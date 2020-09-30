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
#include <assert.h>
#include <algorithm>
#include "reorganizer.h"
#include "../index/index.h"
#include <sys/time.h>

template<typename T>
Layout<T>::Layout(vector<Range<T>> ranges, bitset<ANUM> cols, int p_id) {
	this->ranges = ranges;
	this->cols = cols;
	this->p_id = p_id;
	int num = 0;
	for (int i = 0; i < this->cols.size(); i++) {
		if (this->cols[i] == 1) {
			num++;
		}
	}
	this->a_num = num;
}

template<typename T>
Layout<T> Layout<T>::parse(string s) {
	vector<string> tokens;
	vector<Range<T>> ranges;
	size_t pos = 0;
	string token;
	while ((pos = s.find(" ")) != string::npos) {
		token = s.substr(0, pos);
		s.erase(0, pos + 1);
		tokens.push_back(token);
	}
	tokens.push_back(s);
	int p_id;
	istringstream iss(tokens[0]);
	iss >> p_id;

	reverse(tokens[1].begin(), tokens[1].end());
	bitset<ANUM> cols (tokens[1]);

	reverse(tokens[tokens.size() - 1].begin(), tokens[tokens.size() - 1].end());
	bitset<ANUM * 2> o (tokens[tokens.size() - 1]);
	for (int i = 0; i < ANUM; i++){
		uint64_t x, y;
		istringstream iss(tokens[2 * i + 2]);
		iss >> x;
		istringstream iss1(tokens[2 * i + 3]);
		iss1 >> y;
		Range<T> r(x, y, o[2 * i], o[2 * i + 1]);
		ranges.push_back(r);
	}
	return Layout(ranges, cols, p_id);

}

template<typename T>
vector<Segment<T>*> *evaluate(Partition<T> *p, vector<Layout<T>> layouts) {
	assert(p->getSegments()->size() == 1);
	vector<Segment<T>*> *re = new vector<Segment<T>*>();

	Segment<T> *seg = p->getSegments()->front();
	vector<Tuple<T>*> *tuples = seg->getTuples();
	uint64_t s_id = 0;
	for (auto l: layouts) {
		vector<Tuple<T>*> *new_tuples = new vector<Tuple<T>*>();
		for (auto t: *tuples) {
			bool result = true;
			for (int i = 0; i < ANUM; i++) {
				result = l.ranges[i].satisfy(*(t->vals + i)) && result;
				if (result == false){
					break;
				}
			}
			if (result) {
				T *new_vals = new T[l.a_num];
				int pos = 0;
				for (int i = 0; i < ANUM; i++) {
					if (l.cols[i] == 1) {
						*(new_vals + pos) = *(t->vals + i);
						pos++;
					}
				}
				Tuple<T> *new_t = new Tuple<T>();
				new_t->vals = new_vals;
				new_t->tuple_id = t->tuple_id;

				new_tuples->push_back(new_t);
			}
		}
		Segment<T> *new_seg = new Segment<T>(s_id, l.cols, new_tuples);
		s_id++;
		re->push_back(new_seg);
	}
	return re;
}

template<typename T>
vector<Segment<T>*> *transfer(vector<Segment<T>*> *segs){
	Idmap<T> seg_map;
	Colmap<T> re;
	// hash values and bitmap of tuples by t_id
	for (auto s: *segs) {
		bitset<ANUM> k = s->getColumns();

		for (auto t: *(s->getTuples())) {
			uint64_t t_id = t->tuple_id;
			T *vals = t->vals;
			vpair<T> v(k, vals);
			if (seg_map.find(t_id) == seg_map.end()) {
				vector<vpair<T>> new_vec{v};
				seg_map[t_id] = new_vec;
			}
			else {
				seg_map[t_id].push_back(v);
			}
		}
	}

	// merge values of tuples with same t_id
	for (auto const &it: seg_map) {
		uint64_t t_id = it.first;
		vector<vpair<T>> tuples = it.second;
		bitset<ANUM> merge_cols;
		for (auto t: tuples) {
			merge_cols |= t.first;
		}
		int a_num = 0;
		for (int i = 0; i < merge_cols.size(); i++) {
			if (merge_cols[i] == 1) {
				a_num++;
			}
		}
		T *new_vals = new T[a_num];
		int pos = 0;
		for (int i = 0; i < ANUM; i++) {
			for (auto &t: tuples) {
				if (t.first[i] == 1) {
					*(new_vals + pos) = *((T*)t.second);
					t.second ++;
					pos++;
				}
			}
		}
		Tuple<T> *new_t = new Tuple<T>();
		new_t->vals = new_vals;
		new_t->tuple_id = t_id;

		// hash new tuple by bitmap

		if (re.find(merge_cols) == re.end()) {
			// vector<Tuple<T>*> *new_vec = new vector<Tuple<T>*>{new_t};
			vector<Tuple<T>*> *new_vec = new vector<Tuple<T>*>(TNUM);
			(*new_vec)[new_t->tuple_id] = new_t;
			re[merge_cols] = new_vec;
		}
		else {
			// re[merge_cols]->push_back(new_t);
			(*(re[merge_cols]))[new_t->tuple_id] = new_t;
		}
	}

	// merge tuples with same bitmap into same segment
	uint64_t seg_id = 0;
	vector<Segment<T>*> *segments = new vector<Segment<T>*>();
	for (auto const &it: re) {
		bitset<ANUM> cols = it.first;
		// vector<Tuple<T>*> *tuples = it.second;
		vector<Tuple<T>*> *tuples = new vector<Tuple<T>*>();
		for (int i = 0; i < TNUM; i++) {
			if (it.second->at(i))
				tuples->push_back(it.second->at(i));
		}
		Segment<T> *new_seg = new Segment<T>(seg_id, cols, tuples);
		segments->push_back(new_seg);

		delete it.second;
	}
	re.clear();
	return segments;
}


template<typename T>
void reorganize(string partition_file, string dest, string table_file, string index_file) {

	struct timeval t0, t1, t2;
    

    double evaluate_t = 0, transfer_t = 0, tuple_index_t = 0;
	ifstream infile(table_file, ios::binary);
	infile.seekg(0, ios::end);
	size_t size = infile.tellg();
	infile.seekg(0, ios::beg);

	char *buffer = new char[size];
	infile.read(buffer, size);
    if(infile.gcount() != size){
        infile.close();
        cout << "initialize error" << endl;
    }
    infile.close();


	Partition<T> *p = Partition<T>::parseRegularTable(buffer, size);

	ifstream pf(partition_file);
	string content;
	unordered_map<int, vector<Layout<T>>> layouts;
	while(getline(pf, content)) {
		Layout<T> l = Layout<T>::parse(content);
		if (layouts.find(l.p_id) == layouts.end()) {
			vector<Layout<T>> new_layout{l};
			layouts[l.p_id] = new_layout;
		}
		else{
			layouts[l.p_id].push_back(l);
		}
	}

	uint64_t tuple_num = p->tuple_num;

	uint64_t p_num = 0;

	vector<vector<int> > attr_index(ANUM);
	for (int i = 0; i < ANUM; i++) {
		vector<int> new_vec;
		attr_index[i] = new_vec;
	}
	vector<vector<int> > schema_index;
	vector<TupleIndex> tuple_index(TNUM);

	int** intermediate = new int*[TNUM];
	for(int i = 0; i < TNUM; ++i)
	    intermediate[i] = new int[ANUM];

	for (auto const &it: layouts) {
		int p_id = it.first;
		vector<Layout<T>> ls = it.second;
		// write one partition
		gettimeofday(&t0, NULL);
		vector<Segment<T>*> *e = evaluate(p, ls);
		gettimeofday(&t1, NULL);
		vector<Segment<T>*> *t = transfer(e);
		gettimeofday(&t2, NULL);

		evaluate_t += (1000000 * ( t1.tv_sec - t0.tv_sec )
            + t1.tv_usec - t0.tv_usec) /1000000.0;

		transfer_t += (1000000 * ( t2.tv_sec - t1.tv_sec )
            + t2.tv_usec - t1.tv_usec) /1000000.0;


		for (auto s: *e) {
			for (auto t: *(s->getTuples())) {
				delete[] t->vals;
			}
		}
		delete e;
		Partition<T> *new_p = new Partition<T>(p_id, t);
		string filename = dest + "p_" + to_string(p_id);
		new_p->writeToFile(filename);

		p_num++;
		// attr_index
		bitset<ANUM> cols = new_p->getColumns();
		for (int i = 0; i < ANUM; i++) {
			if (cols[i] == 1) {
				attr_index[i].push_back(p_id);
			}
		}

		// intermediate schema index
		for (auto s: *(new_p->getSegments())) {
			bitset<ANUM> cols = s->getColumns();
			vector<int> attrs;
			for (int i = 0; i < ANUM; i++) {
				if (cols[i] == 1) {
					attrs.push_back(i);
				}
			}
			for (auto t: *(s->getTuples())) {
				uint64_t t_id = t->tuple_id;
				
				for (auto i: attrs) {
					intermediate[t_id][i] = p_id;
				}
			}
		}
		for (auto s: *t) {
			for (auto t: *(s->getTuples())) {
				delete[] t->vals;
			}
		}
		delete new_p;
	}
	delete[] buffer;

	gettimeofday(&t0, NULL);
	int pid_to_offset[PNUM];
	int schema_id = 0;
	for (int i = 0; i < TNUM; i++) {
		if (i % 20000000 == 0) {
			cout << i / 200000000 * 100 << "%% completed" << endl;
		}
		for (int j = 0; j < PNUM; j++)
			pid_to_offset[j] = -1;

		int offset = 0;
		vector<int> partitions;
		int tuple_schema_id;
		for (int j = 0; j < ANUM; j++) {
			int p_id = intermediate[i][j];
			if (pid_to_offset[p_id] == -1) 
			{
				pid_to_offset[p_id] = offset;
				offset++;
				partitions.push_back(p_id);
			}
			intermediate[i][j] = pid_to_offset[p_id];
		}
		// compare
		bool flag = false;
		for (int k = 0; k < schema_index.size(); k++){
			if (memcmp(schema_index[k].data(), intermediate[i], ANUM * sizeof(int)) == 0) {
				flag = true;
				tuple_schema_id = k;
				break;
			}
		}
		if (!flag) {
			vector<int> new_schema;
			new_schema.assign(intermediate[i], intermediate[i] + ANUM);
			schema_index.push_back(new_schema);
			tuple_schema_id = schema_id;
			schema_id++;
		}
		// insert tuple index

		tuple_index[i] = make_pair(tuple_schema_id, partitions);
	}
	gettimeofday(&t1, NULL);
	tuple_index_t += (1000000 * ( t1.tv_sec - t0.tv_sec )
            + t1.tv_usec - t0.tv_usec) /1000000.0;
	cout << "schema num: " << schema_index.size() << endl;
	cout << "evaluate time: " << evaluate_t << endl;
	cout << "transfer time: " << transfer_t << endl;
	cout << "tuple index build time: " << tuple_index_t << endl;
	Index *index = new Index(p_num, attr_index, schema_index, tuple_index);
	index->serialize(index_file);
	attr_index.clear();
	schema_index.clear();
	tuple_index.clear();
	delete[] intermediate;
	return;
}

template<typename T>
string Layout<T>::to_string() {
	stringstream buffer;
	buffer << "partition_id: " << this->p_id <<endl;
	buffer << "bitmap: " << this->cols << endl;
	buffer << "attribute number: " << this->a_num <<endl;
	buffer << "ranges: " <<endl;
	for (auto r : this->ranges) {
		buffer << "\t" << ((r.o_min == 0) ? "(" : "[") << r.r_min << "," << r.r_max << ((r.o_max == 0) ? ")" : "]") <<endl; 
	}
	return buffer.str();
}


template vector<Segment<VTYPE>*> *evaluate<VTYPE>(Partition<VTYPE> *p, vector<Layout<VTYPE>> layouts);
template vector<Segment<VTYPE>*> *transfer<VTYPE>(vector<Segment<VTYPE>*> *segs);
template void reorganize<VTYPE>(string partition_file, string dest, string table_file, string index_file);