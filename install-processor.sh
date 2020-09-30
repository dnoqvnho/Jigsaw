#!/bin/bash

git clone https://code.osu.edu/pythia/core.git query_processor
cd query_processor
git checkout 707bfb00
#git am --signoff < ../query_processor.patch
git apply ../query_processor.patch
#cp ../query_processor_constants.h ./util/partitions/constants.h

make dist
make baseline_driver
make partition_seq_driver
make partition_lockfree_seq_driver

mv ./drivers/baselines/executequery-row ../bins/
mv ./drivers/baselines/executequery-column ../bins/
mv ./drivers/partitions/executequery-hv-seq ../bins/execute-jigsaw-l
mv ./drivers/partitions/executequery-hv-lock-free-seq \
       ../bins/execute-jigsaw-s
