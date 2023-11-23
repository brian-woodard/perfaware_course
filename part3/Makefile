CPPFLAGS = --std=c++17 -Wall -I. -O1 -Wno-format
#CPPFLAGS = --std=c++17 -Wall -I. -O3 -Wno-format
#CPPFLAGS = --std=c++17 -Wall -I. -g -Wno-format

all:
	nasm -f elf64 listing_0132_nop_loop_linux.asm
	g++ $(CPPFLAGS) listing_0133_front_end_test_main.cpp -o listing_0133_front_end_test_main listing_0132_nop_loop_linux.o

clean:
	rm -f *.o
	rm -f *_main
