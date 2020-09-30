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
#include "unit_cost.h"
#include <limits>
#include <sstream>
#include <string.h>
#include <bits/stdc++.h>

template<typename T>
Segment<T>::Segment(const bitset<ANUM>& attributes, uint64_t tnum
                    , const vector<T>& ranges
                    , const bitset<2*ANUM>& closed
                    , const vector<Query<T>*>& queries){
    this->attributes = attributes;
    this->anum = attributes.count();
    this->tnum = tnum;
    this->ranges = ranges;
    this->closed = closed;
    this->queries = queries;
    this->benefit = -2;
    
    update_cost();
}


template<typename T>
bool Segment<T>::fit_max_psize(vector<Segment<T>*>& children){
    children.clear(); 
    int qnum[ANUM];
    memset(qnum, 0, sizeof(int) * ANUM);
    for(auto q : queries){
        for(int i = 0; i < ANUM; i++)
            if(q->selection[i] == 1)
                qnum[i]++;
    }
    int max_attr = 0;
    for(int i = 0; i < ANUM; i++)
        if(qnum[i] > qnum[max_attr])
            max_attr = i;

    bool ret = this->fit_max_psize(max_attr, children);
    return ret;
}

template<typename T>
bool Segment<T>::fit_max_psize(int aid, vector<Segment<T>*> &children){
    
    T l(this->ranges[2*aid]), r(this->ranges[2*aid+ 1]);
    T v = (l + r) / 2;
    if(v == r || v == l){
        children.push_back(this);
        return false;
    }

    bool in_first = true;
    Segment<T>* ts[2];
    this->horizontal_split(aid, v, in_first, ts);
    if(ts[0] == NULL){
        children.push_back(this);
        return false;
    }
    
    for(int i = 0; i < 2; i++){
        if(ts[i]->get_size() <= MAX_PSIZE)
            children.push_back(ts[i]);
        else{
            bool ret = ts[i]->fit_max_psize(aid, children);
            if (ret)
                delete ts[i];
        }
    }
    return true;
}


template<typename T>
double Segment<T>::split(vector<Segment<T>*>& children){
    children.clear();
    vector<Segment<T>*> tmp;
    this->benefit = -1;
    for(auto q : queries){
        double b = this->split(*q, tmp);
        if(b > 0){
            if(b > this->benefit){
                for(auto c : children)
                    delete c;
                children = tmp;
                this->benefit = b;
            }else{
                for(auto c : tmp)
                    delete c;
            }
        }
    }

    return this->benefit;
}

template<typename T>
double Segment<T>::split(const Query<T>& q, vector<Segment<T>*>& children)
{
    children.clear();

    //vertically partition
    bitset<ANUM> va[3];
    va[0] = q.selection & this->attributes;
    va[2] = q.projection & this->attributes;
    va[1] = (q.selection | q.projection).flip();
    va[1] &= this->attributes;

    Segment<T>* vs[3];
    for(int i = 0; i < 3; i++){
        if(va[i].count() == 0){
            vs[i] = NULL;
            continue;
        }
        vs[i] = new Segment(va[i], this->tnum, this->ranges
                , this->closed, this->queries);
        vs[i]->filter_queries();
        /*if(vs[i]->queries.size() == 0){
            delete vs[i];
            vs[i] = NULL;
        }*/
    }

    
    //horizontally partition vs[2]
    Segment<T>* hs[2] = {NULL, NULL};
    if(vs[2] != NULL)
        vs[2]->horizontal_split(q, hs);

    double pre_cost = this->cost;
    double aft_cost = 0;
    for(int i = 0; i < 2; i++){
        if(vs[i] == NULL)
            continue;
        aft_cost += vs[i]->cost;
        children.push_back(vs[i]);
    }

    if(vs[2] != NULL){
        if(hs[0] == NULL){
            children.push_back(vs[2]);
            aft_cost += vs[2]->cost;
        }else{
            children.push_back(hs[0]);
            children.push_back(hs[1]);
            aft_cost += hs[0]->cost + hs[1]->cost;
            delete vs[2];
        }
    }
    
    if(aft_cost >= pre_cost){
        for(auto s : children)
            delete s;
        children.clear();
        return -1;
    }
    return pre_cost - aft_cost;
}



/*template<typename T>
void Segment<T>::horizontal_split(const Query<T>& q
        , Segment<T>* children[2]){
    
    children[0] = children[1] = NULL;
    double hcost = this->cost;

    for(int i = 0; i < ANUM; i++){
        if(q.selection[i] == 0)
            continue;
        T l(this->ranges[2*i]), r(this->ranges[2*i + 1]);
        for(int j = 2*i; j < 2*i+2; j++){
            T v = q.ranges[j];
            if(v >= r || v <= l)
                continue;
            //estimate the number of tuples
            double ratio = (double)(v - l) / (r - l);

            uint64_t t1 = this->tnum * ratio;
            uint64_t t2 = this->tnum - t1;

            //compute the ranges
            vector<T> r1(this->ranges), r2(this->ranges);
            bitset<2*ANUM> c1(this->closed), c2(this->closed);
            r1[2*i + 1] = v;
            r2[2*i] = v;
            //set the closed
            if((q.closed[j] && j == 2*i)
                || (q.closed[j] == 0 && j == 2*i + 1)){
                c1[2*i + 1] = 0;
                c2[2*i] = 1;
            }else{
                c1[2*i + 1] = 1;
                c2[2*i] = 0;
            }

            Segment<T>* ts[2];
            ts[0] = new Segment<T>(this->attributes, t1, r1, c1
                    , this->queries);
            ts[1] = new Segment<T>(this->attributes, t2, r2, c2
                    , this->queries);
            ts[0]->filter_queries();
            ts[1]->filter_queries();
            double tcost = ts[0]->cost + ts[1]->cost;
            
            //place the segments with minimal cost in children 
            if(tcost < hcost){
                if(children[0] != NULL){
                    delete children[0];
                    delete children[1];
                }
                children[0] = ts[0];
                children[1] = ts[1];
                hcost = tcost;
            }
        }
    }

}*/

template<typename T>
void Segment<T>::horizontal_split(const Query<T>& q
        , Segment<T>* children[2]){
    children[0] = children[1] = NULL;
    double hcost = this->cost;

    for(int i = 0; i < ANUM; i++){
        if(q.selection[i] == 0)
            continue;
        for(int j = 2*i; j < 2*i + 2; j++){
            T v = q.ranges[j];
            bool in_first = true;
            if((j == 2*i && q.closed[j] == 1) 
                 || (j == 2*i +1 && q.closed[j] == 0))
                in_first = false;
            Segment<T>* ts[2];
            this->horizontal_split(i, v, in_first, ts);
            if(ts[0] == NULL)
                continue;
            double tcost = ts[0]->cost + ts[1]->cost;
            if(tcost < hcost){
                if(children[0] != NULL){
                    delete children[0];
                    delete children[1];
                }
                children[0] = ts[0];
                children[1] = ts[1];
                hcost = tcost;
            }else{
                delete ts[0];
                delete ts[1];
            }
        }
    }
}

template<typename T>
void Segment<T>::horizontal_split(int aid, T v, bool in_first
        , Segment<T>* children[2]){
    children[0] = children[1] = NULL;

    T l(this->ranges[2*aid]), r(this->ranges[2*aid+1]);
    if(v >= r || v <= l)
        return;

    double ratio = (double)(v-l) / (r - l);
    uint64_t t1 = this->tnum * ratio;
    uint64_t t2 = this->tnum - t1;

    //compute the ranges
    vector<T> r1(this->ranges), r2(this->ranges);
    bitset<2*ANUM> c1(this->closed), c2(this->closed);
    r1[2*aid + 1] = v;
    r2[2*aid] = v;
    //set the closed
    if(in_first){
        c1[2*aid+1] = 1;
        c2[2*aid] = 0;
    }else{
        c1[2*aid + 1] = 0;
        c2[2*aid] = 1;
    }

    children[0] = new Segment<T>(this->attributes, t1, r1, c1
                    , this->queries);
    children[1] = new Segment<T>(this->attributes, t2, r2, c2
                    , this->queries);
    children[0]->filter_queries();
    children[1]->filter_queries();
}


//remove the queries that does not access the segment
template<typename T>
void Segment<T>::filter_queries(){
    vector<Query<T>*> new_queries;
    new_queries.reserve(queries.size());
    for(auto q : queries){
        if(intersect(q))
            new_queries.push_back(q);
    }
    this->queries = new_queries;
    update_cost();
}

/*
 * Return ture if the all ranges in the query and in the segment intersect
 */
template<typename T>
bool Segment<T>::intersect(Query<T>* query){
    if((query->selection & this->attributes).count() > 0)
        return true;
    if((query->projection & this->attributes).count() == 0)
        return false;
    for(int i = 0; i < ranges.size(); i+=2){
        //i1 <- segment
        //i2 <- query
        //i1.l and i2.r
        int j = i + 1;
        if(this->closed[i] && query->closed[j]){
            if(this->ranges[i] > query->ranges[j])
                return false;
        }else{
            if(this->ranges[i] >= query->ranges[j])
                return false;
        }
        //i1.r and i2.l
        if(this->closed[j] && query->closed[i]){
            if(this->ranges[j] < query->ranges[i])
                return false;
        }else{
            if(this->ranges[j] <= query->ranges[i])
                return false;
        }
    }
    return true;
}

template<typename T>
void Segment<T>::update_cost(){
    uint64_t size = this->tnum * this->anum * sizeof(T)
        + this->tnum * sizeof(uint64_t);
    this->cost = UnitCost::getCost(size) * this->queries.size();
}

template<typename T>
uint64_t Segment<T>::get_size(){
    uint64_t size = this->tnum * this->anum * sizeof(T)
        + this->tnum * sizeof(uint64_t);
    return size;
}

template<typename T>
uint64_t Segment<T>::get_data_size(){
    return this->tnum * this->anum * sizeof(T);
}

template<typename T>
uint64_t Segment<T>::get_id_size(){
    return this->tnum * sizeof(uint64_t);
}

template<typename T>
void Segment<T>::query_bits(bitset<QNUM> &bits){

    bits.reset();
    for(auto q : queries)
        bits.set(q->qid);

}


template<typename T>
string Segment<T>::to_string(){
    string tmp;
    //flush the attributes
    tmp = attributes.to_string();
    reverse(tmp.begin(), tmp.end());
    string ret = tmp + " ";
    //flush the ranges
    for(auto t : ranges)
        ret.append(std::to_string(t) + " ");
    //flush the closed bitmap
    tmp = closed.to_string();
    reverse(tmp.begin(), tmp.end());
    ret += tmp;
    return ret;
}

template<typename T>
Segment<T> Segment<T>::parse_string(string str){
    stringstream ss(str);
    string token;
    
    //parse attributes
    ss >> token;
    reverse(token.begin(), token.end());
    bitset<ANUM> attribute(token);
    
    //parse the ranges
    vector<T> ranges;
    ranges.reserve(2*ANUM);
    for(int i = 0; i < 2*ANUM; i++){
        ranges.push_back(0);
        ss >> ranges[i];
    }

    //parse the closed bitmap
    ss >> token;
    reverse(token.begin(), token.end());
    bitset<2*ANUM> closed(token);

    //an empty query set
    vector<Query<T>*> queries;

    return Segment<T>(attribute, TNUM, ranges, closed, queries);
}

template<typename T>
void Segment<T>::add_query(const vector<Query<T>*>& queries){
    this->queries.insert(this->queries.end(), queries.begin(), queries.end());
    filter_queries();
}

template class Segment<VTYPE>;
