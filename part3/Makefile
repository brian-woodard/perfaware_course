CPPFLAGS = --std=c++17 -Wall -I. -O1 -Wno-format
#CPPFLAGS = --std=c++17 -Wall -I. -O3 -Wno-format
#CPPFLAGS = --std=c++17 -Wall -I. -g -Wno-format

all:
	#gcc -c listing_0132_nop_loop_linux.s
	##nasm -f elf64 listing_0132_nop_loop_linux.asm
	#g++ $(CPPFLAGS) listing_0133_front_end_test_main.cpp -o listing_0133_front_end_test_main listing_0132_nop_loop_linux.o
	#gcc -c listing_0134_multinop_loops_linux.s
	##nasm -f elf64 listing_0134_multinop_loops_linux.asm
	#g++ $(CPPFLAGS) listing_0135_multinop_loops_main.cpp -o listing_0135_multinop_loops_main listing_0134_multinop_loops_linux.o
	#gcc -c listing_0136_conditional_nop_loops_linux.s
	##nasm -f elf64 listing_0136_conditional_nop_loops_linux.asm
	#g++ $(CPPFLAGS) listing_0138_conditional_nop_loops_main.cpp -o listing_0138_conditional_nop_loops_main listing_0136_conditional_nop_loops_linux.o
	#gcc -c listing_0139_jump_alignment_linux.s
	#nasm -f elf64 listing_0139_jump_alignment_linux.asm
	#g++ $(CPPFLAGS) listing_0140_jump_alignment_main.cpp -o listing_0140_jump_alignment_main listing_0139_jump_alignment_linux.o
	nasm -f elf64 listing_0141_rat_linux.asm
	g++ $(CPPFLAGS) listing_0142_rat_main.cpp -o listing_0142_rat_main listing_0141_rat_linux.o


clean:
	rm -f *.o
	rm -f *_main
