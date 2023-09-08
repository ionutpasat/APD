build:
	mpicc ranksort.c -o ranksort -lm
run:
	mpirun -np 4 ranksort --mca opal_warn_on_missing_libcuda 0
clear:
	rm ranksort
