NAME = fra-server-example

COMPILER_OPTIONS := -Wall -Wextra -pedantic -std=gnu99

ORIGI_CFLAGS := $(CFLAGS)
CFLAGS = $(ORIGI_CFLAGS) -Os $(COMPILER_OPTIONS) -DNDEBUG $(INCLUDE_DIRS) $(OPTFLAGS)
LDLIBS = $(OPTSLIBS)

BUILD_DIR = build


INCLUDE_DIRS = -I../include
LDLIBS += ../build/libfra.a -lfcgi -lcurl -lmysqlclient

SOURCES = $(wildcard **/**/*.c **/*.c *.c)
OBJECTS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SOURCES))

dummy_create_build_folders := $(shell ./create_dirs_copy.sh $(BUILD_DIR) $(OBJECTS))

TARGET = $(BUILD_DIR)/$(NAME)

#The Target Build
all: $(TARGET)

dev: CFLAGS = $(ORIGI_CFLAGS) -g $(COMPILER_OPTIONS) $(INCLUDE_DIRS) $(OPTFLAGS)
dev: all

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LDLIBS) -o $(TARGET)

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR)

