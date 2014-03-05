#!/bin/bash


chunksize=(50 25 10 5 1)

# $1 numele executabil
# $2 numar saptamani
# $3 nume fisier intrare
# $4 nume fisier iesire
# $5 redirectare terminal

echo "$1   $2   $3   $4   $5" >> $5

###################################################
# THREADS = 1

echo >> $5
 
export OMP_NUM_THREADS=1

echo "Threads = 1" >> $5

for i in `seq 0 4`;
do
	export OMP_SCHEDULE="dynamic,${chunksize[$i]}"
	echo "dynamic,${chunksize[$i]}" >> $5
	{ time ./$1 $2 $3 $4 ; } 2>> $5
	echo >> $5
done

echo >> $5
 
for i in `seq 0 4`;
do
	export OMP_SCHEDULE="static,${chunksize[$i]}"
	echo "static,${chunksize[$i]}" >> $5
	{ time ./$1 $2 $3 $4 ; } 2>> $5
	echo >> $5
done

echo >> $5
 
for i in `seq 0 4`;
do
	export OMP_SCHEDULE="guided,${chunksize[$i]}"
	echo "guided,${chunksize[$i]}" >> $5
	{ time ./$1 $2 $3 $4 ; } 2>> $5
	echo  >> $5
done

echo >> $5
###################################################
# THREADS = 2
 
export OMP_NUM_THREADS=2

echo "Threads = 2" >> $5

for i in `seq 0 4`;
do
	export OMP_SCHEDULE="dynamic,${chunksize[$i]}"
	echo "dynamic,${chunksize[$i]}" >> $5
	{ time ./$1 $2 $3 $4 ; } 2>> $5
	echo  >> $5
done

echo >> $5
 
for i in `seq 0 4`;
do
	export OMP_SCHEDULE="static,${chunksize[$i]}"
	echo "static,${chunksize[$i]}" >> $5
	{ time ./$1 $2 $3 $4 ; } 2>> $5
	echo  >> $5
done

echo >> $5

for i in `seq 0 4`;
do
	export OMP_SCHEDULE="guided,${chunksize[$i]}"
	echo "guided,${chunksize[$i]}" >> $5
	{ time ./$1 $2 $3 $4 ; } 2>> $5
	echo  >> $5
done

echo >> $5

###################################################
# THREADS = 4

export OMP_NUM_THREADS=4;

echo "Threads = 4" >> $5

for i in `seq 0 4`;
do
	export OMP_SCHEDULE="dynamic,${chunksize[$i]}"
	echo "dynamic,${chunksize[$i]}" >> $5
	{ time ./$1 $2 $3 $4 ; } 2>> $5
	echo >> $5
done

echo >> $5

for i in `seq 0 4`;
do
	export OMP_SCHEDULE="static,${chunksize[$i]}"
	echo "static,${chunksize[$i]}" >> $5
	{ time ./$1 $2 $3 $4 ; } 2>> $5
	echo  >> $5
done

echo >> $5

for i in `seq 0 4`;
do
	export OMP_SCHEDULE="guided,${chunksize[$i]}"
	echo "guided,${chunksize[$i]}" >> $5
	{ time ./$1 $2 $3 $4 ; } 2>> $5
	echo >> $5
done

echo >> $5
