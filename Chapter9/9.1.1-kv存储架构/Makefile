target: kvstore client

src=kvstore.c reactor.c kvstore_array.c ntyco_entry.c kvstore.h

kvstore: ${src}
	gcc ${src} -o kvstore -I ./ -I ~/code/Ntyco/core -L ~/code/Ntyco -lntyco

client: client.c
	gcc client.c -o client

.PHONY:clean
clean:
	-trash-put kvstore
	-trash-put client