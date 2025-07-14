#
# Students' Makefile for the Malloc Lab
#
TEAM = bovik
VERSION = 1
HANDINDIR = /afs/cs.cmu.edu/academic/class/15213-f01/malloclab/handin

CC = gcc
CFLAGS = -Wall -O2 -m32
CFLAGS_DEBUG = -Wall -Og -m32

OBJS = mdriver.o mm.o memlib.o fsecs.o fcyc.o clock.o ftimer.o
OBJ1 = mdriver.o mm1.o memlib.o fsecs.o fcyc.o clock.o ftimer.o
OBJ2 = mdriver.o mm2.o memlib.o fsecs.o fcyc.o clock.o ftimer.o
OBJ3 = mdriver.o mm3.o memlib.o fsecs.o fcyc.o clock.o ftimer.o


VERSION ?= 0

ifeq ($(VERSION), 1)
  SELECTED_OBJS = $(OBJ1)
  SELECTED_CFLAGS = $(CFLAGS)
  TARGET = mdriver1

else ifeq ($(VERSION), 2)
  SELECTED_OBJS = $(OBJ2)
  SELECTED_CFLAGS = $(CFLAGS)
  TARGET = mdriver2

else ifeq ($(VERSION), 3)
  SELECTED_OBJS = $(OBJ3)
  SELECTED_CFLAGS = $(CFLAGS)
  TARGET = mdriver3

else ifeq ($(VERSION), debug)
  SELECTED_OBJS = $(OBJS)
  SELECTED_CFLAGS = $(CFLAGS_DEBUG)
  TARGET = mdriverg

else ifeq ($(VERSION), test)
  SELECTED_OBJS = $(OBJS)
  SELECTED_CFLAGS = $(CFLAGS)
  TARGET = mdriver

endif

# 默认构建目标
all: $(TARGET)

# 通用构建规则：目标、调试版本、测试版本
$(TARGET): $(SELECTED_OBJS)
	$(CC) $(SELECTED_CFLAGS) -o $(TARGET) $(SELECTED_OBJS)



# mdriver: $(OBJS)
# 	$(CC) $(CFLAGS) -o mdriver $(OBJS)
# mdriver_g: $(OBJS)
# 	$(CC) $(CFLAGS_DEBUG) -o mdriver_g $(OBJS)
# mdriver1: $(OBJ1)
# 	$(CC) $(CFLAGS) -o mdriver1 $(OBJ2)
# mdriver2: $(OBJ2)
# 	$(CC) $(CFLAGS) -o mdriver1 $(OBJ2)



mdriver.o: mdriver.c fsecs.h fcyc.h clock.h memlib.h config.h mm.h
memlib.o: memlib.c memlib.h
mm3.o: mm3.c mm.h memlib.h
mm2.o: mm2.c mm.h memlib.h
mm1.o: mm1.c mm.h memlib.h
mm.o: mm.c mm.h memlib.h
fsecs.o: fsecs.c fsecs.h config.h
fcyc.o: fcyc.c fcyc.h
ftimer.o: ftimer.c ftimer.h config.h
clock.o: clock.c clock.h

handin:
	cp mm.c $(HANDINDIR)/$(TEAM)-$(VERSION)-mm.c

clean:
	rm -f *~ *.o mdriver, mdriverg, mdriver1, mdriver2, mdriver3