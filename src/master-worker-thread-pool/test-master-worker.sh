#!/bin/bash
# script takes 4 arguments that are given to the master worker program
# ./master-worker #total_items #max_buf_size #num_workers #masters e.g. ./exe 10000 1000 4 3

gcc -o master-worker master-worker.c -lpthread
./master-worker $1 $2 $3 $4 > output 
awk -f check.awk MAX=$1 output