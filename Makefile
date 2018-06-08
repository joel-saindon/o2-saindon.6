proj6: main.c info.h oss user
	gcc -c main.c
	gcc main.o -o proj6 

oss: oss.c info.h
	gcc -c oss.c
	gcc oss.o -o oss

user: user.c info.h
	gcc -c user.c
	gcc user.o -o user

clean: 
	rm *.o proj6 oss user logfile
