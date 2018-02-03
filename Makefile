.PHONY: bin/lisp
bin/lisp:
	cc -std=c99 -Wall -g src/lisp.c src/lparser.c src/util.c src/lval.c src/mpc/mpc.c -ledit -o bin/lisp
	valgrind bin/lisp

.PHONY: test
test:
	cc -std=c99 -Wall -g test/test-util.c test/unity/unity.c -o test/test-util
	cc -std=c99 -Wall -g test/test-lparser.c src/mpc/mpc.c test/unity/unity.c -o test/test-lparser
	cc -std=c99 -Wall -g test/test-lval.c src/util.c src/lparser.c src/mpc/mpc.c test/unity/unity.c -o test/test-lval
	test/test-util
	test/test-lval
	test/test-lparser
