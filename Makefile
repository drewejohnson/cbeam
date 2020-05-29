INSTALL_DIR:=/usr/bin
CC:=gcc

.PHONY: install test clean

cbeam : cbeam.c
	${CC} $< -o $@

install : cbeam
	test -d ${INSTALL_DIR}
	mv $< ${INSTALL_DIR}

test : cbeam
	./$< < test.md | diff --ignore-all-space test.tex -
	
clean:
	rm -f cbeam
