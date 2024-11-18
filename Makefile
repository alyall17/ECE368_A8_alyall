WARNING = -Wall -Wshadow --pedantic
ERROR = -Wvla -Werror
GCC = gcc -std=c99 -g $(WARNING) $(ERROR) 
#GCC = gcc -g $(WARNING) $(ERROR) 

SRCS = a8.c
OBJS = $(SRCS:%.c=%.o)

# diff -w means do not care about space

a8: $(OBJS) 
	$(GCC) $(OBJS) -o a8

.c.o: 
	$(GCC) -c $*.c

run: a8
	./a8 points.txt

clean: # remove all machine generated files
	rm -f a8 *.o output* *~