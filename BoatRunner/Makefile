PWD = $(shell pwd)
PROJ_NAME = "BoatRunner"
INC = -I./headers
SHAD_DIR = ./shaders
OUT_DIR = ./build
CFLAGS = -std=c++20
LDFLAGS = -lglfw -lvulkan -ldl -lpthread
DBGFLAGS = -g

$(PROJ_NAME): BoatRunner.cpp
	glslc -o $(SHAD_DIR)/frag.spv $(SHAD_DIR)/shader.frag
	glslc -o $(SHAD_DIR)/vert.spv $(SHAD_DIR)/shader.vert
	g++ $(DBGFLAGS) $(CFLAGS) $(LDFLAGS) $(INC) -o $(OUT_DIR)/$(PROJ_NAME) BoatRunner.cpp

.PHONY: test clean

test: $(PROJ_NAME)
	./build/$(PROJ_NAME)

build:
	g++ $(DBGFLAGS) $(CFLAGS) $(LDFLAGS) $(INC) -o $(OUT_DIR)/$(PROJ_NAME) BoatRunner.cpp

shad:
	glslc -o $(SHAD_DIR)/frag.spv $(SHAD_DIR)/shader.frag
	glslc -o $(SHAD_DIR)/vert.spv $(SHAD_DIR)/shader.vert

clean:
	rm -f build/$(PROJ_NAME) $(SHAD_DIR)/frag.spv $(SHAD_DIR)/vert.spv
