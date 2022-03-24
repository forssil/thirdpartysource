#####################################
### uartops test ###
#####################################

# source file
SOURCE  := $(wildcard ./*.c)
ROOT_PATH    = ./../..
#OBJS    := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))
OBJS := $(SOURCE:%.c=%.o)
TARGET_DIR	:= ./output
AEC_PATH		:= ./libs
# target you can change test to what you want
TARGET  := libaec.so
#compile and lib parameter
CC      := ../rk3308gClientSDK/prebuilts/host/usr/bin/arm-rockchip-linux-gnueabihf-gcc
CXX     := ../rk3308gClientSDK/prebuilts/host/usr/bin/arm-rockchip-linux-gnueabihf-g++
LD      := ../rk3308gClientSDK/prebuilts/host/usr/bin/arm-rockchip-linux-gnueabihf-ld
LIBS    := -ldl -lpthread -lasound -lssl -lcrypto -lcurl -lm -lc -lAPF -lNE10
#LDFLAGS := -L$(AEC_PATH)/ -L$(ROOT_PATH)/libs/
LDFLAGS := -L$(ROOT_PATH)/libs/  -L$(ROOT_PATH)/thirdparty/ne10/libs_mca
DEFINES := -fPIC -Wl,-rpath=. -DNE10_UNROLL_LEVEL=1
INCLUDE := -Iinclude -I./extlib  -I./../../sorce/AudioProcessingAlgorithm/audio_processing/include -I$(ROOT_PATH)/thirdparty/ne10/inc
CFLAGS  := -g -Wall -O3 $(DEFINES) $(INCLUDE)

CXXFLAGS:= $(CFLAGS)

%.o : %.c
	    $(CC) $(CXXFLAGS) -c $< -o $@ $(LDFLAGS) $(LIBS)

$(TARGET) : $(OBJS)
			rm -f $@
			$(CC) -shared -o $@ $^ $(LDFLAGS) $(LIBS) 
	    

all : $(TARGET)

objs : $(OBJS)

rebuild: veryclean everything

clean :
	rm -rf *.o
	rm $(TARGET)

veryclean : clean
	rm -rf $(TARGET)
