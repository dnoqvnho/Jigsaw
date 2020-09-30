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
#include "partitioner.h"
#include "unit_cost.h"
#include <queue>
#include <unordered_map>


/*
template<typename T>
void Partitioner<T>::split(Segment<T>* table
        , vector<Segment<T>*> &segments){

    priority_queue<snode> pq;
    vector<Segment<T>*> tmp;
    double benefit;
    benefit = table->split(tmp);
    pq.push(snode(table, tmp));
    
    while(pq.top().self->benefit > 0){
        snode t = pq.top();
        pq.pop();

        for(auto c : t.children){
            benefit = c->split(tmp);
            pq.push(snode(c, tmp));
        }
    }


    segments.clear();
    segments.reserve(pq.size());
    while(pq.size() > 0){
        segments.push_back(pq.top().self);
        for(auto t : pq.top().children)
            delete t;
        pq.pop();
    }

}
*/

template<typename T>
void Partitioner<T>::split(Segment<T>* table
        , vector<Segment<T>*> &segments){

    segments.clear();

    priority_queue<snode> pq;
    vector<Segment<T>*> tmp;
    double benefit;
    benefit = table->split(tmp);
    pq.push(snode(table, tmp));
    
    while(pq.size() > 0 && pq.top().self->benefit > 0){
        snode t = pq.top();
        pq.pop();

        bool push = false;
        for(auto c : t.children)
            if(c->get_size() <= MIN_PSIZE){
                push = true;
                break;
            }
        if(push){
            segments.push_back(t.self);
            for(auto c : t.children)
                delete c;
        }else{

            for(auto c : t.children){
                benefit = c->split(tmp);
                pq.push(snode(c, tmp));
            }
        }
    }

    while(pq.size() > 0){
        segments.push_back(pq.top().self);
        for(auto t : pq.top().children)
            delete t;
        pq.pop();
    }

}

template<typename T>
void Partitioner<T>::merge(const vector<Segment<T>*> &segments
        , vector<vector<Segment<T>*>> &partitions){

    partitions.clear();
   
    vector<Segment<T>*> rem;
    rem.reserve(segments.size());
    for(auto s : segments){
        if(s->get_size() >= MIN_PSIZE){
            vector<Segment<T>*> v;
            v.push_back(s);
            partitions.push_back(v);
        }else
            rem.push_back(s);
    }

    unordered_map<bitset<QNUM>, vector<Segment<T>*>> map;
    map.reserve(rem.size());
    typename unordered_map<bitset<QNUM>, vector<Segment<T>*>>::iterator got;

    bitset<QNUM> key;
    for(Segment<T>* s : rem){
        s->query_bits(key);
        got = map.find(key);
        if(got == map.end()){
            map[key] = vector<Segment<T>*>();
            got = map.find(key);
        }

        got->second.push_back(s);
    }

    //partitions.reserve(map.size());
    for(got = map.begin(); got != map.end(); got++){
        //partitions.push_back(got->second);
        uint64_t size = 0;
        vector<Segment<T>*> part;
        for(auto s : got->second){
            part.push_back(s);
            size += s->get_size();
            if(size >= MAX_PSIZE){
                partitions.push_back(part);
                part.clear();
            }
        }
        if(part.size() != 0)
            partitions.push_back(part);
    }
}

template<typename T>
double Partitioner<T>::partition(Segment<T>* table
        , vector<vector<Segment<T>*>> &partitions){

    vector<Segment<T>*> segments;
    split(table, segments);

    vector<Segment<T>*> fit_segments;

    //int k = 0;
    for(auto s : segments){
        //printf("%d out of %d\n", k++, segments.size());
        if(s->get_size() <= MAX_PSIZE){
            fit_segments.push_back(s);
            continue;
        }
        vector<Segment<T>*> children;
        bool ret = s->fit_max_psize(children);
        fit_segments.insert(fit_segments.end(), children.begin()
                , children.end());
        if(ret)
            delete s;
    }

    merge(fit_segments, partitions);

    //estimate the cost given the partition
    double cost = 0;
    bitset<QNUM> q;
    for(auto& p : partitions){
        uint64_t size = 0;
        for(auto& s : p)
            size += s->get_size();
        p[0]->query_bits(q);
        cost += UnitCost::getCost(size) * q.count();
    }
    return cost;

}

template class Partitioner<VTYPE>;
