#!make
# ConfManager Makefile.
####################################################################
# Source code, header files, object files and result directories

ROOT_PATH    = ./../..
SRC_PATH     = .
OBJ_PATH     = $(ROOT_PATH)/objs
OUTDIR	     = $(ROOT_PATH)/libs
STATIC_LIB   = $(OUTDIR)/libAPF.a
#STATIC_LIB   = $(OUTDIR)/libaec.so
####################################################################
# Compile and link options
LINK = ld

INCLUDES =\
	-I.\
	-I$(ROOT_PATH)/include\
	-I./acoustic_echo_cancellation/include\
	-I./acoustic_echo_cancellation/include/interface\
	-I./audio_processing/include\
	-I./common_audio/include\
	-I./agc/include\
	-I./vad/include \
	-I$(ROOT_PATH)/thirdparty/ne10/inc

C_FILES = \
	$(SRC_PATH)/common_audio/source/fft.c\
	$(SRC_PATH)/common_audio/source/randomization_functions.c\
	$(SRC_PATH)/agc/source/agc_new.c \
	$(SRC_PATH)/common_audio/source/Amp_compute.c
	

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
	$(SRC_PATH)/common_audio/source/basemath.cpp \
	$(SRC_PATH)/common_audio/source/DCRemover.cpp \
	$(SRC_PATH)/common_audio/source/F2Ttransformer.cpp \
	$(SRC_PATH)/common_audio/source/SPEst.cpp \
	$(SRC_PATH)/common_audio/source/T2Ftransformer.cpp \
	$(SRC_PATH)/vad/source/VAD.cpp 

COBJ_FILES = $(addprefix $(OBJ_PATH)/,$(notdir $(C_FILES:.c=.o)))
CPPOBJ_FILES = $(addprefix $(OBJ_PATH)/,$(notdir $(CPP_FILES:.cpp=.o)))

OBJ_FILES = $(COBJ_FILES) $(CPPOBJ_FILES)

#compile and lib parameter
CC      := $(ROOT_PATH)/tools/rk3308gClientSDK/prebuilts/host/usr/bin/arm-rockchip-linux-gnueabihf-gcc
CXX     := $(ROOT_PATH)/tools/rk3308gClientSDK/prebuilts/host/usr/bin/arm-rockchip-linux-gnueabihf-g++
LD      := $(ROOT_PATH)/tools/rk3308gClientSDK/prebuilts/host/usr/bin/arm-rockchip-linux-gnueabihf-ld
AR       = $(ROOT_PATH)/tools/rk3308gClientSDK/prebuilts/host/usr/bin/arm-rockchip-linux-gnueabihf-ar
DEFINES := -fPIC -fpermissive -Wl,-rpath=. -DARM_NEON
INCLUDE := -Iinclude -I./extlib
CFLAGS  := -g -Wall -O3 $(DEFINES) $(INCLUDE) $(INCLUDES)

CXXFLAGS:= $(CFLAGS)

$(STATIC_LIB): checkbindir checkdir $(OBJ_FILES)
	$(AR) rcs $(STATIC_LIB) $(OBJ_FILES)
	#$(CXX) -shared -o $(STATIC_LIB) $(OBJ_FILES)

$(OBJ_PATH)/%.o: $(SRC_PATH)/common_audio/source/%.c
	
	$(CXX) $(CXXFLAGS) $(INC_DIRS) -c $< -o $@

	
$(OBJ_PATH)/%.o: $(SRC_PATH)/agc/source/%.c
	$(CXX) $(CXXFLAGS) $(INC_DIRS) -c $< -o $@

$(OBJ_PATH)/%.o: $(SRC_PATH)/audio_processing/source/%.cpp
	$(CXX) $(CXXFLAGS) $(INC_DIRS) -c $< -o $@
	
$(OBJ_PATH)/%.o: $(SRC_PATH)/acoustic_echo_cancellation/source/%.cpp
	$(CXX) $(CXXFLAGS) $(INC_DIRS) -c $< -o $@

$(OBJ_PATH)/%.o: $(SRC_PATH)/common_audio/source/%.cpp
	$(CXX) $(CXXFLAGS) $(INC_DIRS) -c $< -o $@
	
$(OBJ_PATH)/%.o: $(SRC_PATH)/vad/source/%.cpp
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

	
all: $(STATIC_LIB)
.PHONY:  clean
clean:
	@rm -rf $(OBJ_FILES) $(STATIC_LIB)
