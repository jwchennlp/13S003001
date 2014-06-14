sender: socket.o file.o data.o send.c socket.h file.h data.h 
	gcc -g file.o socket.o data.o send.c -o sender
receiver: socket.o file.o data.o receive.c socket.h file.h data.h 
	gcc -g file.o socket.o data.o receive.c -o receiver 
socket.o: socket.c socket.h
	gcc -c socket.c
file.o: file.c file.h
	gcc -c file.c
data.o: data.c data.h
	gcc -c data.c
clean:
	rm -r *o sender receiver
