CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -lm
TARGET = fleet_system
SRCS = main_cpp.c module1.c module2.c module3.c module4.c

all: $(TARGET)

$(TARGET): $(SRCS) cpp_header.h
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe *.o
