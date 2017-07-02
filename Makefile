TARGET := malhas
CC := g++

# Flags para Linux
LIB_FLAGS := -lglut -lGLU -lGL -lm

# Flags para Windows - MSYS2 ou Cygwin
ifeq ($(OS),Windows_NT)
    INC_PATH := -Ideps_windows/include
    LIB_PATH := -Ldeps_windows/lib/x64
    LIBS     := -lfreeglut -lglu32 -lopengl32 -lm

    LIB_FLAGS := $(INC_PATH) $(LIB_PATH) $(LIBS)
endif

# Pega todos os arquivos diretamente na pasta src/
# NÃO acessa subpastas da pasta src/
SOURCES = $(wildcard src/*.cpp)

.PHONY: all
all:
	@mkdir -p bin/
	$(CC) $(SOURCES) -Wall -Wextra -Isrc -std=c++11 -o bin/$(TARGET) $(LIB_FLAGS)
ifeq ($(OS),Windows_NT)
	cp deps_windows/bin/x64/freeglut.dll bin/
endif

.PHONY: run
run:
	./bin/$(TARGET)

.PHONY: clean
clean:
	rm -f bin/$(TARGET)
