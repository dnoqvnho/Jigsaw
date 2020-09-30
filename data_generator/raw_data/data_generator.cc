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
#include <fstream>

using namespace std;

int main(const int argc, const char* argv[]){
    int tnum = atoi(argv[1]);
    int anum = atoi(argv[2]);

    ofstream ofile(argv[3], ios::binary);

    int step = 1024*1024;
    int* buffer = new int[step*anum];

    for(int i = 0; i < tnum; i += step){
        int l = tnum - i > step ? step : tnum - i;
        for(int j = 0; j < l * anum; j++){
            buffer[j] = rand() % tnum;
        }

        ofile.write((char*) buffer, sizeof(int) * l * anum);
    }

    delete[] buffer;
    ofile.close();

    ofile.open(argv[4]);

    //write fake partition ID
    ofile << 0 << " ";
    //write the attribute bitmap
    for(int i = 0; i < anum; i++)
        ofile << "1";
    ofile << " ";
    //write the ranges
    for(int i = 0; i < anum; i++)
        ofile << 0 << " " << tnum - 1 << " ";
    //write the closed bitmap
    for(int i = 0; i < 2*anum; i++)
        ofile << "1";

    ofile.close();
}
