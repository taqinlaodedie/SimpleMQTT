CC = gcc
CFLAGS = -lwsock32 -std=C99 -Wall
 
TARGET  = MQTT_main
OBJ_DIR = ./
SRC_DIR = ../src
INC_DIR = ../inc
 
SRC = $(wildcard $(SRC_DIR)/*.c ./*.c)
DIR = $(notdir $(SRC))
OBJECTS = $(patsubst %.c, %.o, $(DIR))

VPATH = $(SRC_DIR)
vpath %.o $(OBJ_DIR)

all: $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CC) -o $@ $(addprefix $(OBJ_DIR)/, $(OBJECTS)) $(CFLAGS)

%.o : %.c
	$(CC) -c $< -o $(OBJ_DIR)/$@ -I$(INC_DIR)
 
.PHONY : clean
clean:
	rm *.o $(TARGET)*