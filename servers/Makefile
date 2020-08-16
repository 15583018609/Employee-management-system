
all:service.o function.o
	gcc service.o function.o -o service -lsqlite3

# main.o:main.c
#	gcc -c -o main.o main.c 

# lcd.o:lcd.c
#	gcc -c -o lcd.o lcd.c
			




.PHONY:clean
clean:
	rm service *.o
	#rm service  *.db   *.o
