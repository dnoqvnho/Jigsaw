# Welcome to Jigsaw!

Jigsaw is a data storage and query evaluation engine for irregular table partitioning. 

## Code structure
This repository includes the code to generate data, partition table, physically store the table, and evaluate queries.

- The `data_generator` folder contains the code to generate the table (`data_generator/raw_data`) and the queries `data_generator/query`.
- The `partitioner` folder logically partitions the table based on the queries. The logical partitioning describes how the table is partitioned, i.e. a specific table cell should be stored in which partition. The `partitioner/Column-H` and `partitioner/Row-V` implements the horizontal partitioning and vertical partitioning in the two baselines. `partitioner/HV` is the partitioning algorithm in Jigsaw.
- The `data_loader` folder physically stores the table according to its logical partitioning. The `data_loader/HV` generates the physical storage in Jigsaw and `data_loader/baselines` stores the table in the four baselines, i.e. Row, Column, Row-V, Column-H.
- The `install-processor.sh` script clones the public repository of Pythia, an open source query engine, and patches the code to support the Jigsaw evaluation model.
- The `clear_cache` folder contains the code to clear the OS cache between queries.
- The `bins` folder will contain all executables after a successful compile.

## Installation
```sh
$ make clean
$ make
```
