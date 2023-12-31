#OBJS specifies which files to compile as part of the project
OBJS = main.cpp glad.c

#CC specifies which compiler we're using
CC = g++

#COMPILER_FLAGS specifies the additional compilation options we're using
# -w suppresses all warnings
COMPILER_FLAGS = 

#LINKER_FLAGS specifies the libraries we're linking against
LINKER_FLAGS = -ldl -lglfw

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = BlockGame.exe

#This is the target that compiles our executable
default : $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)

run:
	./$(OBJ_NAME)
all:
	make
	./$(OBJ_NAME)