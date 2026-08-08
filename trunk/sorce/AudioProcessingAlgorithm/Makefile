#!make
# ConfManager Makefile.
####################################################################
# Source code, header files, object files and result directories

ROOT_PATH    = ./../..
SRC_PATH     = .
OBJ_PATH     = $(ROOT_PATH)/objs
OUTDIR      ?= $(ROOT_PATH)/libs
STATIC_LIB   = $(OUTDIR)/libAPF.a
SHARED_LIB   = $(OUTDIR)/libAPF.so
NE10_INCLUDE_DIR ?= $(ROOT_PATH)/thirdparty/ne10/inc
NE10_LIBS ?=
####################################################################
# Compile and link options
LINK = ld

INCLUDES =\
	-I.\
	-I$(ROOT_PATH)/..\
	-I$(ROOT_PATH)/include\
	-I./acoustic_echo_cancellation/include\
	-I./acoustic_echo_cancellation/include/interface\
	-I./audio_processing/include\
	-I./common_audio/include\
	-I./agc/include\
	-I./rnnoise/include\
	-I./vad/include \
	-I./beam_forming/include \
	-I$(NE10_INCLUDE_DIR)

C_FILES = \
	$(SRC_PATH)/common_audio/source/fft.c\
	$(SRC_PATH)/common_audio/source/randomization_functions.c\
	$(SRC_PATH)/common_audio/source/fft1536.c\
	$(SRC_PATH)/agc/source/agc_new.c \
	$(SRC_PATH)/rnnoise/source/celt_lpc.c \
	$(SRC_PATH)/rnnoise/source/denoise.c \
	$(SRC_PATH)/rnnoise/source/kiss_fft.c \
	$(SRC_PATH)/rnnoise/source/pitch.c \
	$(SRC_PATH)/rnnoise/source/rnn.c \
	$(SRC_PATH)/rnnoise/source/rnn_data.c \
	$(SRC_PATH)/rnnoise/source/rnn_reader.c \
	$(SRC_PATH)/common_audio/source/Amp_compute.c \
	
	

CPP_FILES = \
	$(SRC_PATH)/acoustic_echo_cancellation/source/AcousticEchoCancellation.cpp \
	$(SRC_PATH)/acoustic_echo_cancellation/source/AdapFilterGroup.cpp \
	$(SRC_PATH)/acoustic_echo_cancellation/source/delaybuffer.cpp \
	$(SRC_PATH)/acoustic_echo_cancellation/source/delayest.cpp \
	$(SRC_PATH)/acoustic_echo_cancellation/source/noiseest.cpp \
	$(SRC_PATH)/acoustic_echo_cancellation/source/noisereduction.cpp \
	$(SRC_PATH)/acoustic_echo_cancellation/source/postfilter.cpp \
	$(SRC_PATH)/acoustic_echo_cancellation/source/PSDsmooth.cpp \
	$(SRC_PATH)/acoustic_echo_cancellation/source/SpeechEst.cpp \
	$(SRC_PATH)/acoustic_echo_cancellation/source/SubbandAdap.cpp \
    $(SRC_PATH)/audio_processing/source/AudioProcessingFramework.cpp \
	$(SRC_PATH)/audio_processing/source/AudioAecCpp.cpp \
	$(SRC_PATH)/audio_processing/source/SUBinterface.cpp \
	$(SRC_PATH)/common_audio/source/basemath.cpp \
	$(SRC_PATH)/common_audio/source/DCRemover.cpp \
	$(SRC_PATH)/common_audio/source/F2Ttransformer.cpp \
	$(SRC_PATH)/common_audio/source/SPEst.cpp \
	$(SRC_PATH)/common_audio/source/T2Ftransformer.cpp \
	$(SRC_PATH)/beam_forming/source/MVDR.cpp \
	$(SRC_PATH)/vad/source/VAD.cpp \
	$(SRC_PATH)/AudioLog.cpp 

COBJ_FILES = $(addprefix $(OBJ_PATH)/,$(notdir $(C_FILES:.c=.o)))
CPPOBJ_FILES = $(addprefix $(OBJ_PATH)/,$(notdir $(CPP_FILES:.cpp=.o)))

OBJ_FILES = $(COBJ_FILES) $(CPPOBJ_FILES)

#compile and lib parameter
CROSS_COMPILE ?= $(ROOT_PATH)/tools/rk3308gClientSDK/prebuilts/host/usr/bin/arm-rockchip-linux-gnueabihf-
CC      ?= $(CROSS_COMPILE)gcc
CXX     ?= $(CROSS_COMPILE)g++
LD      ?= $(CROSS_COMPILE)ld
AR      ?= $(CROSS_COMPILE)ar

#DEFINES := -fPIC -fpermissive -Wl,-rpath=. -DARM_NEON
DEFINES ?= -fPIC -fpermissive -Wl,-rpath=. -Wl,-soname,libAPF.so 
INCLUDE := -Iinclude -I./extlib
#
#CFLAGS  := -Wall -O3 -g -fno-omit-frame-pointer $(DEFINES) $(INCLUDE) $(INCLUDES)
CFLAGS  := -Wall -O3 $(DEFINES) $(INCLUDE) $(INCLUDES)

CXXFLAGS:= $(CFLAGS)

$(STATIC_LIB): checkbindir checkdir $(OBJ_FILES)
	$(AR) rcs $(STATIC_LIB) $(OBJ_FILES)

$(SHARED_LIB): checkbindir checkdir $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) -shared -fPIC -o $(SHARED_LIB) $(OBJ_FILES) $(NE10_LIBS)

#$(OBJ_PATH)/%.o: $(SRC_PATH)/common_audio/source/%.c
#	
#	$(CXX) $(CXXFLAGS) $(INC_DIRS) -c $< -o $@
#
#$(OBJ_PATH)/%.o: $(SRC_PATH)/rnnoise/source/%.c
#	$(CC) $(CXXFLAGS) $(INC_DIRS) -c $< -o $@
#	
#$(OBJ_PATH)/%.o: $(SRC_PATH)/agc/source/%.c
#	$(CXX) $(CXXFLAGS) $(INC_DIRS) -c $< -o $@
#
#$(OBJ_PATH)/%.o: $(SRC_PATH)/audio_processing/source/%.cpp
#	$(CXX) $(CXXFLAGS) $(INC_DIRS) -c $< -o $@
#	
#$(OBJ_PATH)/%.o: $(SRC_PATH)/acoustic_echo_cancellation/source/%.cpp
#	$(CXX) $(CXXFLAGS) $(INC_DIRS) -c $< -o $@
#
#$(OBJ_PATH)/%.o: $(SRC_PATH)/common_audio/source/%.cpp
#	$(CXX) $(CXXFLAGS) $(INC_DIRS) -c $< -o $@
#	
#$(OBJ_PATH)/%.o: $(SRC_PATH)/vad/source/%.cpp
#	$(CXX) $(CXXFLAGS) $(INC_DIRS) -c $< -o $@
#
#$(OBJ_PATH)/%.o: $(SRC_PATH)/beam_forming/source/%.cpp
#	$(CXX) $(CXXFLAGS) $(INC_DIRS) -c $< -o $@

# 自动提取所有源文件所在的目录路径
VPATH = $(sort $(dir $(C_FILES) $(CPP_FILES)))

$(OBJ_PATH)/%.o: %.c
	$(CC) $(CFLAGS) $(INC_DIRS) -c $< -o $@

$(OBJ_PATH)/%.o: %.cpp
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

	
static: $(STATIC_LIB)

share: $(SHARED_LIB)

all: share static
.PHONY: all static share clean
clean:
	@rm -rf $(OBJ_FILES) $(STATIC_LIB) $(SHARED_LIB)
