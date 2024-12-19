target: kvstore client testcase

src=kvstore.c epoll_entry.c kvstore_array.c ntyco_entry.c kvstore.h

kvstore: ${src}
	gcc ${src} -o kvstore -I./ -I ~/code/Ntyco/core -L ~/code/Ntyco -lntyco

client: client.c
	gcc client.c -o client

testcase: testcase.c
	gcc -o testcase testcase.c -I./

.PHONY:clean
clean:
	-trash-put kvstore
	-trash-put client
	-trash-put testcase