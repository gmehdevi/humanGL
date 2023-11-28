CC = g++

CFLAGS = -g -std=c++17

INCLUDE = ./include

TARGET = humanGL

IMGUI_SRC = ./include/imgui.cpp ./include/imgui_draw.cpp ./include/imgui_impl_glfw.cpp ./include/imgui_impl_opengl3.cpp ./include/imgui_widgets.cpp ./include/imgui_tables.cpp

SRC = main.cpp $(IMGUI_SRC) 

LIBS = -lglfw -lGLEW -lGL

LIBS_MAC = -lglfw -lGLEW -framework OpenGL

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) -I$(INCLUDE) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

linux: $(SRC)
	$(CC) -I$(INCLUDE) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

mac: $(SRC)
	$(CC) -I$(INCLUDE) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS_MAC)

clean:
	rm $(TARGET)

re: clean all

.PHONY: all clean re
