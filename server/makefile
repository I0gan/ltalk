# This is a makefile for complie server
# Create time: 2020-04-20
# Author: i0gan

CC      := g++ 
TARGET  := ltalks
CFLAGS  := -O3 -std=c++11   
LDFLAGS := -lpthread
LDFLAGS += -lmysqlclient
LDFLAGS += -lhiredis

RM := rm -f 
CP := cp -r
MKDIR := mkdir

BUILD_PATH := ./build

# install path
INSTALL_PATH := /usr/share/ltalks

# src path
NET_PATH      :=  ./src/net
THREAD_PATH   :=  ./src/thread
WORK_PATH     := ./src/work
DATABASE_PATH :=  ./src/database

CRYPTO_PATH :=  ./src/crypto
JSON_PATH   :=  ./src/json
MAIN_PATH   :=  ./src

#---------------------OBJ-------------------------
OBJS :=
# main
MAIN_SRC := $(wildcard $(MAIN_PATH)/*.cc)  
MAIN_OBJ := $(patsubst %.cc, %.o, $(MAIN_SRC)) 
OBJS += $(MAIN_OBJ)

# net src
NET_SRC := $(wildcard $(NET_PATH)/*.cc)  
NET_OBJ := $(patsubst %.cc, %.o, $(NET_SRC)) 
OBJS += $(NET_OBJ)

# thread src
THREAD_SRC := $(wildcard $(THREAD_PATH)/*.cc)  
THREAD_OBJ := $(patsubst %.cc, %.o, $(THREAD_SRC)) 
OBJS += $(THREAD_OBJ)
# db src
DATABASE_SRC := $(wildcard $(DATABASE_PATH)/*.cc)  
DATABASE_OBJ := $(patsubst %.cc, %.o, $(DATABASE_SRC)) 
OBJS += $(DATABASE_OBJ)

# work src
WORK_SRC := $(wildcard $(WORK_PATH)/*.cc)  
WORK_OBJ := $(patsubst %.cc, %.o, $(WORK_SRC)) 
OBJS += $(WORK_OBJ)

# crypto src
CRYPTO_SRC := $(wildcard $(CRYPTO_PATH)/*.cc)  
CRYPTO_OBJ := $(patsubst %.cc, %.o, $(CRYPTO_SRC)) 
OBJS += $(CRYPTO_OBJ)

# complie
$(TARGET):$(OBJS)
	@echo -e "\033[33m\t linking all \033[0m"
	$(CC) $^ -o $(BUILD_PATH)/$@ $(LDFLAGS) $(CFLAGS) 
	@echo -e "\033[34m\t finished \033[0m"

$(OBJS):%.o:%.cc
	$(CC) -c $^ -o $@ 

print:
	@echo $(OBJS)	

clean:
	$(RM) $(BUILD_PATH)/$(TARGET) $(OBJS)

install:
	@sudo $(MKDIR) $(INSTALL_PATH)
	@sudo $(CP) $(BUILD_PATH)/* $(INSTALL_PATH)
	@ln -s $(INSTALL_PATH)/$(TARGET) /usr/bin/$(TARGET)
	@echo 'install complete'
