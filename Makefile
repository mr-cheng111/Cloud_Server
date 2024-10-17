PROJECT_NAME := UDP_Server

SRC_DIR := ./src
INC_DIR := ./inc


ARCH=x86
TARGET_NAME := $(PROJECT_NAME)

$(info PROJECT_NAME 被定义，当前值为：$(PROJECT_NAME))
$(info SYSROOT 被定义，当前值为：$(SYSROOT))

CXX:=g++

PKG_CONFIG := PKG_CONFIG_PATH=${SYSROOT}/usr/lib/pkgconfig/ \
	      PKG_CONFIG_SYSROOT_DIR=${SYSROOT} \
	      /usr/bin/pkg-config
CFLAGS := ${CFLAGS}
LDFLAGS := ${LDFLAGS} --sysroot=${SYSROOT} -L${SYSROOT}/usr/lib

LD:=ld
CFLAGS	+= -O0 -g -ggdb -Wall -IM
CFLAGS  += -I$(INC_DIR) 


TARGET = $(PROJECT_NAME)

SRC = $(shell find ./$(SRC_DIR) -name "*.cpp")
$(info SRC 被定义，当前值为：$(SRC))
INC = $(shell find ./$(INC_DIR) -name "*.h")
$(info INC 被定义，当前值为：$(INC))
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(SRC_DIR)/%.h)

$(TARGET) : $(OBJ) $(INC)
	"$(CXX)" $(CFLAGS) $^ $(LIBS) -o $@
	rm -rf *.o *.d
 
# %.o: %.cpp $(INC)
# 	$(CXX) -c $< -o $@

# -include $(DEP)

clean:
	rm -rf $(TARGET)