RELEASE_TARGET_DIR = ../Common/Libs/
TARGET = $(RELEASE_TARGET_DIR)/libproto_out.a
ARFLAGS = cr
CPPFLAGS= -g -O2 -Wall -Werror $(INC) -fPIC  `pkg-config --cflags --libs protobuf` -std=c++11 -lpthread



OBJ_DIR	= .objs


INC     = -I../
#-I../../../opensource/include/
VPATH	= .:$(SERVER_DIR)

SRC = $(wildcard *.cc)
OBJS = $(addprefix $(OBJ_DIR)/,$(subst .cc,.o,$(SRC)))


all : $(TARGET)

$(TARGET) : $(OBJS)
	@if [ ! -e $(RELEASE_TARGET_DIR) ]; then \
		mkdir -p $(RELEASE_TARGET_DIR); \
	fi
	$(AR) $(ARFLAGS) $@ $?

$(OBJ_DIR)/%.o : %.cc
	@if [ ! -e $(OBJ_DIR) ]; then \
		mkdir -p $(OBJ_DIR); \
	fi
	$(CXX) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

