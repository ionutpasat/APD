build:
	mpicc broadcast.c -o broadcast
run:
	mpirun -np 4 broadcast --mca opal_warn_on_missing_libcuda 0
clear:
	rm broadcast