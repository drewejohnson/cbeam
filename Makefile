cbeam : cbeam.c
	gcc $< -o $@

test : cbeam
	./$< < test.md | diff --ignore-all-space test.tex -
	
clean:
	rm -f cbeam
