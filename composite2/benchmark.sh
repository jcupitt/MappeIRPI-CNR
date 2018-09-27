#!/bin/bash

# make 100 copies of the data image, then create maps for all of them

echo making test data set ...
mkdir -p tmp
rm -f tmp/*
for i in {1..100}; do
	cp data.tif tmp/$i
done

echo executing ...
/usr/bin/time -f %M:%e ./a.out tmp/*

rm -rf tmp
