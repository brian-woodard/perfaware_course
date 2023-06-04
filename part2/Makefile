
CPPFLAGS = --std=c++17 -Wall -I. -O3

all:
	g++ $(CPPFLAGS) haversine_generator.cpp -o haversine_generator
	g++ $(CPPFLAGS) haversine_processor.cpp -o haversine_processor

clean:
	rm -f haversine_generator
	rm -f haversine_processor
