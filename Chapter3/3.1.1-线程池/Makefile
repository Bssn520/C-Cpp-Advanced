all: threadpool test_thread_pool

threadpool: thread_pool.c
	gcc thread_pool.c -o libthreadpool.so -I./ -lpthread -fPIC -shared

test_thread_pool: test_thread_pool.c thread_pool.c
	gcc test_thread_pool.c -o test_thread_pool -g -Wl,-rpath=./ -I./ -L./ -lthreadpool -lpthread

# test_thread_pool: test_thread_pool.c thread_pool.c
# 	gcc thread_pool.c test_thread_pool.c -o test_thread_pool -lpthread