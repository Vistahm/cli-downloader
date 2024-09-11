cli-downloader: main.c 
	$(CC) -o cldl main.c network.c ssl_utils.c file_utils.c progress.c -lssl -lcrypto 
