TARGET_DIR = $(CURDIR)/../Bin
TARGET = $(TARGET_DIR)/server

###############################################################################
# variables
###############################################################################
CC  =   gcc 
CXX =   g++
CPP =   g++

###############################################################################
# flags
###############################################################################


CPPFLAGS= -fPIC -O2 -L/usr/include/mysql -Wall -std=c++11 -Wno-deprecated -Wno-unused-parameter -L/usr/lib64/ -g -Wno-unused-variable -Wno-unused-function -L/usr/lib64/ `pkg-config --cflags --libs protobuf`  -Lpthread -std=c++11
#LDFLAGS = -g -fPIC -O0 -Wall -std=c++11 -Wno-deprecated -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -lpthread -lconnector  -lpublic -lcommon -ltinyxml -llzma
LDFLAGS = -g -lcommon -lmysqlclient -lhiredis -lpackage -lproto_out  -lprotobuf `pkg-config --cflags --libs protobuf` -lpthread -g

###############################################################################
# paths
###############################################################################
OBJ_DIR = ./.objs


COMMON_INCLUDE_DIR = ../Common
NEW_COMMON_INCLUDE_DIR = ../Package

PUBLIC_INC  = -I$(COMMON_INCLUDE_DIR)
NEW_PUBLIC_INC = -I$(NEW_COMMON_INCLUDE_DIR)
LIB_DIR = ../Common/Libs
NEW_LIB_DIR = ../Package/Libs
PUBLIC_LIB  = -L/usr/local/lib/ -L$(LIB_DIR) -L$(NEW_LIB_DIR)
SRC = $(wildcard *.cpp) $(wildcard *.c) $(wildcard *.cxx) $(wildcard */*.cpp) $(wildcard */*/*.cpp) $(wildcard */*/*/*.cpp) $(wildcard */*/*/*/*.cpp) $(wildcard */*/*/*/*/*.cpp)
OBJS = $(addprefix $(OBJ_DIR)/,$(subst .cpp,.o,$(SRC)))

###############################################################################
# args
###############################################################################
all : $(TARGET)

$(TARGET) : $(OBJS)
	@if [ ! -e $(TARGET_DIR) ]; then \
		mkdir -p $(TARGET_DIR); \
	fi

	$(CXX) -o $@ $(OBJS) $(PUBLIC_LIB) $(LDFLAGS)

$(OBJ_DIR)/%.o:%.cpp
	@if [ ! -e $(dir $@) ]; then \
	    mkdir -p $(dir $@); \
	fi
	$(CXX) $(CPPFLAGS) $(PUBLIC_INC) $(NEW_PUBLIC_INC) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

