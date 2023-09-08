build:
	mpicc scan.c -o scan
run:
	mpirun -np 4 scan --mca opal_warn_on_missing_libcuda 0
clear:
	rm scan