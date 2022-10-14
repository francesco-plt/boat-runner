PROJ_NAME = "BoatRunner"
INC = -I./headers
SHAD_DIR = ./shaders
OUT_DIR = ./build
CFLAGS = -std=c++17
LDFLAGS = -lglfw -lvulkan -ldl -lpthread
DBGFLAGS = -g -ggdb -glldb -ferror-limit=999
FLAGS = -w

$(PROJ_NAME): BoatRunner.cpp
	glslc -o $(SHAD_DIR)/frag.spv $(SHAD_DIR)/shader.frag
	glslc -o $(SHAD_DIR)/vert.spv $(SHAD_DIR)/shader.vert
	glslc -o $(SHAD_DIR)/SkyBoxFrag.spv $(SHAD_DIR)/SkyBoxShader.frag
	glslc -o $(SHAD_DIR)/SkyBoxVert.spv $(SHAD_DIR)/SkyBoxShader.vert
	g++ $(FLAGS) $(CFLAGS) $(LDFLAGS) $(INC) -o $(OUT_DIR)/$(PROJ_NAME) BoatRunner.cpp

debug:
	g++ $(DBGFLAGS) $(CFLAGS) $(LDFLAGS) $(INC) -o $(OUT_DIR)/$(PROJ_NAME) BoatRunner.cpp

shad:
	glslc -o $(SHAD_DIR)/frag.spv $(SHAD_DIR)/shader.frag
	glslc -o $(SHAD_DIR)/vert.spv $(SHAD_DIR)/shader.vert
	glslc -o $(SHAD_DIR)/SkyBoxFrag.spv $(SHAD_DIR)/SkyBoxShader.frag
	glslc -o $(SHAD_DIR)/SkyBoxVert.spv $(SHAD_DIR)/SkyBoxShader.vert

clean:
	rm -f build/$(PROJ_NAME) $(SHAD_DIR)/frag.spv $(SHAD_DIR)/vert.spv
