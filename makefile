OPT = -g3 -O0
LIB_SOURCES =  iron/mem.c iron/process.c iron/array.c iron/math.c iron/time.c  iron/log.c iron/fileio.c iron/linmath.c iron/test.c test.c xxhash.c hash.c shader_utils.c data_table.c string_table.c game.c stb_image.c  image.c image_filters.c optical_flow.c vr_video_test.c distance_fields.c
CC = gcc
TARGET = glitch
LIB_OBJECTS =$(LIB_SOURCES:.c=.o)
LDFLAGS= -L. $(OPT) -Wextra #-lmcheck #-ftlo #setrlimit on linux 
LIBS= -ldl -lm -lGL -lpthread -lglfw -lGLEW -lpng
ALL= $(TARGET)
CFLAGS = -I. -std=c11 -c $(OPT) -Wall -Wextra -Werror=implicit-function-declaration -Wformat=0 -D_GNU_SOURCE -fdiagnostics-color -Wextra  -Wwrite-strings -fbounds-check -Werror -msse4.2 -mtune=corei7 -DSTB_IMAGE_IMPLEMENTATION #-DDEBUG 

$(TARGET): $(LIB_OBJECTS)
	$(CC) $(LDFLAGS) $(LIB_OBJECTS) $(LIBS) -o $@

all: $(ALL)

.c.o: $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@ -MMD -MF $@.depends 
depend: h-depend
clean:
	rm -f $(LIB_OBJECTS) $(ALL) *.o.depends

-include $(LIB_OBJECTS:.o=.o.depends)

