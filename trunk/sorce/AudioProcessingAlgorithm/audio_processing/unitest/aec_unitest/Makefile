#!make
# ConfManager Makefile.
####################################################################
# Source code, header files, object files and result directories

ROOT_PATH    = ./../../../../..
SRC_PATH     = .
OBJ_PATH     = ./objs
OUTDIR	     = ./bin
TARGET       = $(OUTDIR)/apftest
####################################################################
# Compile and link options
LINK = ld

INCLUDES =\
	-I.\
	-I$(ROOT_PATH)/include\
	-I$(ROOT_PATH)/sorce/AudioProcessingAlgorithm/acoustic_echo_cancellation/include\
	-I$(ROOT_PATH)/sorce/AudioProcessingAlgorithm/acoustic_echo_cancellation/include/interface\
	-I$(ROOT_PATH)/sorce/AudioProcessingAlgorithm/audio_processing/include\
	-I$(ROOT_PATH)/sorce/AudioProcessingAlgorithm/common_audio/include\
	-I$(ROOT_PATH)/sorce/AudioProcessingAlgorithm/vad/include\
	-I$(ROOT_PATH)/tools/WaveFunction\
	-I$(ROOT_PATH)/sorce/AudioProcessingAlgorithm/agc/include\
	-I$(ROOT_PATH)/thirdparty/ne10/inc


CPP_FILES = \
	./test.cpp \
	$(ROOT_PATH)/tools/WaveFunction/WaveIO.cpp 

CPPOBJ_FILES = $(addprefix $(OBJ_PATH)/,$(notdir $(CPP_FILES:.cpp=.o)))

OBJ_FILES = $(CPPOBJ_FILES)

#compile and lib parameter
CC      := $(ROOT_PATH)/tools/rk3308gClientSDK/prebuilts/host/usr/bin/arm-rockchip-linux-gnueabihf-gcc
CXX     := $(ROOT_PATH)/tools/rk3308gClientSDK/prebuilts/host/usr/bin/arm-rockchip-linux-gnueabihf-g++
LD      := $(ROOT_PATH)/tools/rk3308gClientSDK/prebuilts/host/usr/bin/arm-rockchip-linux-gnueabihf-ld
AR       = $(ROOT_PATH)/tools/rk3308gClientSDK/prebuilts/host/usr/bin/arm-rockchip-linux-gnueabihf-ar
DEFINES := -fPIC -Wl,-rpath=. -DARM_NEON
INCLUDE := -Iinclude -I./extlib
CFLAGS  := -Wall -O3 $(DEFINES) $(INCLUDE) $(INCLUDES)

LFLAGS = -L$(ROOT_PATH)/libs -L$(ROOT_PATH)/thirdparty/ne10/libs_mca
LIBS = -lAPF -lNE10
CXXFLAGS:= $(CFLAGS)

$(TARGET): checkbindir checkdir $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ_FILES) $(LFLAGS) $(LIBS)

	
$(OBJ_PATH)/%.o: $(ROOT_PATH)/tools/WaveFunction/%.cpp
	$(CXX) $(CXXFLAGS) $(INC_DIRS) -c $< -o $@

$(OBJ_PATH)/%.o: ./%.cpp
	$(CXX) $(CXXFLAGS) $(INC_DIRS) -c $< -o $@
	
objs: $(OBJ_FILES)

checkbindir:
	@if test ! -d $(OBJ_PATH) ; \
	then \
		mkdir $(OBJ_PATH) ; \
	fi

checkdir: 
	@if test ! -d $(OUTDIR) ; \
	then \
		mkdir $(OUTDIR) ; \
	fi

	
all: $(TARGET)
.PHONY:  clean
clean:
	@rm -rf $(OBJ_FILES) $(TARGET)
