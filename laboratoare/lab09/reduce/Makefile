build:
	mpicc reduce.c -o reduce
run:
	mpirun -np 4 reduce --mca opal_warn_on_missing_libcuda 0
clear:
	rm reduce