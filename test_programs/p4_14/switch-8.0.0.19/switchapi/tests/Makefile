CC = gcc
CFLAGS = -g -Wall -Wextra -Wshadow -Wuninitialized -O2
TARGET = switchapi
LIBS := -lJudy -lpthread -ltommyds

INCLUDE := -I../../p4src/includes
INCLUDE += -I../include
INCLUDE += -I../../../../install/include

OBJECTS = $(patsubst %.c, %.o, $(wildcard ../src/*.c))
HEADERS = $(wildcard ../include/switchapi/*.h)
OBJECTS += $(patsubst %.c, %.o, $(wildcard ./*_test.c))

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): switch_test.c $(OBJECTS)
	$(CC) switch_test.c $(OBJECTS) -Wall $(INCLUDE) -L./lib $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
