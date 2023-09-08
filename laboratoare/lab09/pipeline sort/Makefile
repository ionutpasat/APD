build:
	mpicc pipeline_sort.c -o pipeline_sort
run1:
	mpirun -np 2 pipeline_sort --mca opal_warn_on_missing_libcuda 0 > a1.txt
run2:
	mpirun -np 14 --oversubscribe pipeline_sort --mca opal_warn_on_missing_libcuda 0 > a2.txt
clear:
	rm pipeline_sort
