#!/bin/bash

tar zxf externals/metis-5.1.0.tar.gz
cd metis-5.1.0
make config prefix=$PWD/../dist
make
make install
cd ../
rm -rf metis-5.1.0
