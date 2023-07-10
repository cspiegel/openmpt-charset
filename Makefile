openmpt-charset: openmpt-charset.cpp
	$(CXX) $(shell pkg-config libopenmpt --cflags --libs) -std=c++20 -Wall $< -o $@

clean:
	rm -f openmpt-charset
