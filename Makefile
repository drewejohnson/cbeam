INSTALL_DIR:=/usr/bin

.PHONY: install test clean

cbeam : cbeam.c
	gcc $< -o $@

install : cbeam
	test -d ${INSTALL_DIR}
	mv $< ${INSTALL_DIR}

test : cbeam
	./$< < test.md | diff --ignore-all-space test.tex -
	
clean:
	rm -f cbeam
