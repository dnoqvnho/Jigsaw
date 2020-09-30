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
#include <cstdint>
#include <string.h>
#include <string>
#include <sstream>
#include <new>
#include <unistd.h>

using namespace std;

string getStdoutFromCommand(string cmd) {
    string data;
    FILE * stream;
    const int max_buffer = 256;
    char buffer[max_buffer];
    cmd.append(" 2>&1");

    stream = popen(cmd.c_str(), "r");
    if (stream) {
        while (!feof(stream))
            if (fgets(buffer, max_buffer, stream) != NULL) 
                data.append(buffer);
        pclose(stream);
    }
    return data;
}

uint64_t parseFree(string ostr, int pos){
    char const* digits = "0123456789";
    size_t n = ostr.find_first_of(digits);
    ostr = ostr.substr(n);
    stringstream ss(ostr);
    uint64_t ret;
    for(int i = 0; i <= pos; i++)
        ss >> ret;
    return ret;
}

void clear_cache(){
    uint64_t unit_size = 32;
    unit_size *= 1024*1024;
    string free_out = getStdoutFromCommand("free -b");
    printf("%s", free_out.c_str());
    uint64_t total_size = parseFree(free_out, 5);
    //uint64_t total_size = sysconf(_SC_AVPHYS_PAGES);
    //total_size *= sysconf(_SC_PAGE_SIZE);
    
    //uint64_t total_size = 1024;
    //total_size *= 1024*1024*1024;
    uint64_t unit_num = total_size / unit_size;
    printf("Request %lu\n", total_size);
    
    char* buff[unit_num];
    memset(buff, 0, unit_num);
    
    for(uint64_t i = 0; i < unit_num; i++){
        try{
            buff[i] = new char[unit_size];
            memset(buff[i], 1, unit_size); 
        }catch(std::bad_alloc &excepObj){
            for(uint64_t j = 0; j < unit_num; j++)
                if(buff[j] != 0)
                    delete buff[j];
            return;
        }
    }
}

int main(const int argc, const char* argv[]){
    clear_cache();
    return 0;
}
