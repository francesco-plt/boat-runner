PWD = $(shell pwd)
PROJ_NAME = $(shell cut -c 1-3 <<< $(notdir $(PWD)))
CFLAGS = -std=c++17 -O2
INC=-I./headers
LDFLAGS = -lglfw -lvulkan -ldl -lpthread

$(PROJ_NAME): main.cpp
	g++ $(CFLAGS) $(INC) -o $(PROJ_NAME) main.cpp $(LDFLAGS)

.PHONY: test clean

test: $(PROJ_NAME)
	./$(PROJ_NAME)

build:
	g++ $(CFLAGS) $(INC) -o $(PROJ_NAME) main.cpp $(LDFLAGS)

clean:
	rm -f $(PROJ_NAME)	

shad:
	glslc -o shaders/BRDF1Frag.spv shaders/BRDF1Frag.frag
	glslc -o shaders/BRDF1Vert.spv shaders/BRDF1Vert.vert
	glslc -o shaders/BRDF2Frag.spv shaders/BRDF2Frag.frag
	glslc -o shaders/BRDF2Vert.spv shaders/BRDF2Vert.vert
	