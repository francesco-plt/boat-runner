PWD = $(shell pwd)
PROJ_NAME = "Boat Runner"
INC = -I./headers
SHAD_DIR = ./shaders
OUT_DIR = ./build
CFLAGS = -std=c++17 -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread
DBGFLAGS = -g

$(PROJ_NAME): main.cpp
	glslc -o $(SHAD_DIR)/frag.spv $(SHAD_DIR)/shader.frag
	glslc -o $(SHAD_DIR)/vert.spv $(SHAD_DIR)/shader.vert
	g++ $(CFLAGS) $(INC) -o $(OUT_DIR)/$(PROJ_NAME) main.cpp $(LDFLAGS)

.PHONY: test clean

test: $(PROJ_NAME)
	./$(PROJ_NAME)

build:
	g++ $(CFLAGS) $(INC) -o $(OUT_DIR)/$(PROJ_NAME) main.cpp $(LDFLAGS) $(DBGFLAGS)

shad:
	glslc -o $(SHAD_DIR)/frag.spv $(SHAD_DIR)/shader.frag
	glslc -o $(SHAD_DIR)/vert.spv $(SHAD_DIR)/shader.vert

clean:
	rm -f $(PROJ_NAME) $(SHAD_DIR)/frag.spv $(SHAD_DIR)/vert.spv
