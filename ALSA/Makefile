
all:
#	$(CC) -Wall -fPIC -DPIC -save-temps -g3 -O3 -c s2mono.c
	$(CC) -Wall -fPIC -DPIC -O3 -c s2mono.c
	$(CC) -shared -o libasound_module_pcm_s2mono.so s2mono.o
#	$(CC) -shared -Wl,-soname,libasound_module_pcm_s2mono.so -o libasound_module_pcm_s2mono.so s2mono.o 
#	-lasound
