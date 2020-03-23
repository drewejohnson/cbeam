cbeam : cbeam.c
	gcc $< -o $@
	
clean:
	rm -f cbeam
