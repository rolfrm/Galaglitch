OPT = -g0 -O4
LIB_SOURCES = game.c ../iron/mem.c ../iron/process.c ../iron/array.c ../iron/math.c ../iron/time.c  ../iron/log.c ../iron/fileio.c ../iron/linmath.c mock_player_update.c mock_ai.c physics_update.c test.c game_ui.c xxhash.c hash.c shader_utils.c
CC = gcc
TARGET = glitch
LIB_OBJECTS =$(LIB_SOURCES:.c=.o)
LDFLAGS= -L. $(OPT) -Wextra #-lmcheck #-ftlo #setrlimit on linux 
LIBS= -ldl -lm -lGL -lpthread -lglfw -lGLEW
ALL= $(TARGET)
CFLAGS = -I.. -std=c11 -c $(OPT) -Wall -Wextra -Werror=implicit-function-declaration -Wformat=0 -D_GNU_SOURCE -fdiagnostics-color -Wextra  -Wwrite-strings -fbounds-check -Werror -msse4.1 -mtune=corei7  #-DDEBUG 

$(TARGET): $(LIB_OBJECTS)
	$(CC) $(LDFLAGS) $(LIB_OBJECTS) $(LIBS) -o $@

all: $(ALL)

.c.o: $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@ -MMD -MF $@.depends 
depend: h-depend
clean:
	rm -f $(LIB_OBJECTS) $(ALL) *.o.depends

-include $(LIB_OBJECTS:.o=.o.depends)

