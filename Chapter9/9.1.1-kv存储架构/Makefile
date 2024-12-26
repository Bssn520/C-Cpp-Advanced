target: kvstore client testcase

src=kvstore.c kvstore.h epoll_entry.c ntyco_entry.c kvstore_array.c kvstore_rbtree.c kvstore_hash.c

kvstore: ${src}
	gcc ${src} -o kvstore -I./ -I ./Ntyco/core -L ./Ntyco -lntyco

client: client.c
	gcc client.c -o client

testcase: testcase.c
	gcc -o testcase testcase.c -I./

.PHONY:clean
clean:
	-trash-put kvstore
	-trash-put client
	-trash-put testcase