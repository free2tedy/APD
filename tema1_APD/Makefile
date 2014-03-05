serial_neoptimizat: serial_neoptimizat.c
	gcc serial_neoptimizat.c -o serial

serial_optimizat: serial_optimizat.c
	gcc serial_optimizat.c -o serial
	
paralel_optimizat: paralel_optimizat.c
	gcc -fopenmp paralel_optimizat.c -o paralel

clean:
	rm -f serial paralel
