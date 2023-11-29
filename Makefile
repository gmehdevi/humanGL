CC = g++
CFLAGS = -g -std=c++17

TARGET = humanGL

INCLUDE = ./include
INCLUDES = humanGL Camera GL_Prog include/utils include/iterators include/ft_mat include/ft_vec
INCLUDES_EXT = .hpp
INCLUDES := $(addsuffix $(INCLUDES_EXT), $(INCLUDES))

IMGUI_SRC = ./include/imgui.cpp ./include/imgui_draw.cpp ./include/imgui_impl_glfw.cpp ./include/imgui_impl_opengl3.cpp ./include/imgui_widgets.cpp ./include/imgui_tables.cpp
SRCS = main.cpp $(IMGUI_SRC)
OBJS = $(SRCS:.cpp=.o)

LIBS = -lglfw -lGLEW -lGL -ldl

LIBS_MAC = -lglfw -lGLEW -framework OpenGL

all: $(TARGET)

%.o: %.cpp $(INCLUDES) Makefile
	$(CC) -I$(INCLUDE) $(CFLAGS) -c $< -o $@ $(LIBS)

$(TARGET): $(OBJS)
	$(CC) -I$(INCLUDE) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

mac: $(SRCS) $(INCLUDES) Makefile
	$(CC) -I$(INCLUDE) $(CFLAGS) -o $(TARGET) $(SRCS) $(LIBS_MAC)

clean:
	rm $(TARGET)

fclean: clean
	rm $(OBJS)

re: fclean all

.PHONY: all clean fclean re
