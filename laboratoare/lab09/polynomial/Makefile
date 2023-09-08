build:
	mpicc polynomial.c -o polynomial -lm
run1:
	mpirun -np 2 polynomial a1.txt --mca opal_warn_on_missing_libcuda 0
run2:
	mpirun -np 14 polynomial a2.txt --mca opal_warn_on_missing_libcuda 0
clear:
	rm polynomial
