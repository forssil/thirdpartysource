#!make
# ConfManager Makefile.
####################################################################
# Source code, header files, object files and result directories

ROOT_PATH    = ./../..
SRC_PATH     = .
OBJ_PATH    ?= $(ROOT_PATH)/objs
OUTDIR      ?= $(ROOT_PATH)/libs
STATIC_LIB   = $(OUTDIR)/libAPF.a
SHARED_LIB   = $(OUTDIR)/libAPF.so
NE10_INCLUDE_DIR ?= $(ROOT_PATH)/thirdparty/ne10/inc
TARGET_ARCH ?= generic
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
	-I./arm64 \
	-I$(NE10_INCLUDE_DIR)

GENERIC_C_FILES = \
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
	
ARM64_C_FILES = \
	$(SRC_PATH)/arm64/common_audio/source/fft.c\
	$(SRC_PATH)/arm64/common_audio/source/randomization_functions.c\
	$(SRC_PATH)/arm64/common_audio/source/fft1536.c\
	$(SRC_PATH)/arm64/agc/source/agc_new.c \
	$(SRC_PATH)/arm64/rnnoise/source/celt_lpc.c \
	$(SRC_PATH)/arm64/rnnoise/source/denoise.c \
	$(SRC_PATH)/arm64/rnnoise/source/kiss_fft.c \
	$(SRC_PATH)/arm64/rnnoise/source/pitch.c \
	$(SRC_PATH)/arm64/rnnoise/source/rnn.c \
	$(SRC_PATH)/arm64/rnnoise/source/rnn_data.c \
	$(SRC_PATH)/arm64/rnnoise/source/rnn_reader.c \
	$(SRC_PATH)/arm64/common_audio/source/Amp_compute.c

ARM64_ALL_C_FILES = \
	$(SRC_PATH)/arm64/agc/source/agc_new.c \
	$(SRC_PATH)/arm64/agc/source/agc.c \
	$(SRC_PATH)/arm64/common_audio/SampleRateConvert/src/samplerate.c \
	$(SRC_PATH)/arm64/common_audio/SampleRateConvert/src/src_linear.c \
	$(SRC_PATH)/arm64/common_audio/SampleRateConvert/src/src_sinc.c \
	$(SRC_PATH)/arm64/common_audio/SampleRateConvert/src/src_zoh.c \
	$(SRC_PATH)/arm64/common_audio/source/Amp_compute.c \
	$(SRC_PATH)/arm64/common_audio/source/fft.c \
	$(SRC_PATH)/arm64/common_audio/source/fft1536.c \
	$(SRC_PATH)/arm64/common_audio/source/randomization_functions.c \
	$(SRC_PATH)/arm64/rnnoise/source/celt_lpc.c \
	$(SRC_PATH)/arm64/rnnoise/source/denoise.c \
	$(SRC_PATH)/arm64/rnnoise/source/kiss_fft.c \
	$(SRC_PATH)/arm64/rnnoise/source/pitch.c \
	$(SRC_PATH)/arm64/rnnoise/source/rnn_data.c \
	$(SRC_PATH)/arm64/rnnoise/source/rnn_reader.c \
	$(SRC_PATH)/arm64/rnnoise/source/rnn.c

ARM64_TEST_BIN = $(OUTDIR)/arm64_parity_test
ARM64_TEST_OBJS = \
	$(OBJ_PATH)/arm64_test.o \
	$(OBJ_PATH)/arm64_test_scalar_amp.o \
	$(OBJ_PATH)/arm64_test_neon_amp.o \
	$(OBJ_PATH)/arm64_test_scalar_rnn.o \
	$(OBJ_PATH)/arm64_test_neon_rnn.o \
	$(OBJ_PATH)/arm64_test_scalar_denoise.o \
	$(OBJ_PATH)/arm64_test_neon_denoise.o \
	$(OBJ_PATH)/arm64_test_scalar_celt_lpc.o \
	$(OBJ_PATH)/arm64_test_neon_celt_lpc.o \
	$(OBJ_PATH)/arm64_test_scalar_src_linear.o \
	$(OBJ_PATH)/arm64_test_neon_src_linear.o

ARM64_CPP_TEST_BIN = $(OUTDIR)/arm64_cpp_parity_test
ARM64_CPP_TEST_OBJS = \
	$(OBJ_PATH)/arm64_cpp_test.o \
	$(OBJ_PATH)/arm64_cpp_test_scalar_spest.o \
	$(OBJ_PATH)/arm64_cpp_test_neon_spest.o \
	$(OBJ_PATH)/arm64_cpp_test_scalar_dc.o \
	$(OBJ_PATH)/arm64_cpp_test_neon_dc.o \
	$(OBJ_PATH)/arm64_cpp_test_scalar_psd.o \
	$(OBJ_PATH)/arm64_cpp_test_neon_psd.o \
	$(OBJ_PATH)/arm64_cpp_test_scalar_psd_adapter.o \
	$(OBJ_PATH)/arm64_cpp_test_neon_psd_adapter.o \
	$(OBJ_PATH)/arm64_cpp_test_scalar_postfilter.o \
	$(OBJ_PATH)/arm64_cpp_test_neon_postfilter.o \
	$(OBJ_PATH)/arm64_cpp_test_scalar_postfilter_adapter.o \
	$(OBJ_PATH)/arm64_cpp_test_neon_postfilter_adapter.o \
	$(OBJ_PATH)/arm64_cpp_test_scalar_adap.o \
	$(OBJ_PATH)/arm64_cpp_test_neon_adap.o \
	$(OBJ_PATH)/arm64_cpp_test_scalar_adap_adapter.o \
	$(OBJ_PATH)/arm64_cpp_test_neon_adap_adapter.o

ifeq ($(TARGET_ARCH),arm64)
C_FILES = $(ARM64_C_FILES)
ARM64_OPT_FLAGS = -march=armv8-a+simd -ffp-contract=fast -fvectorize -fslp-vectorize -DAPF_ARM64_OPTIMIZED
else
C_FILES = $(GENERIC_C_FILES)
endif
	

GENERIC_CPP_FILES = \
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

ARM64_CPP_FILES = \
	$(SRC_PATH)/arm64/acoustic_echo_cancellation/source/AcousticEchoCancellation.cpp \
	$(SRC_PATH)/arm64/acoustic_echo_cancellation/source/AdapFilterGroup.cpp \
	$(SRC_PATH)/arm64/acoustic_echo_cancellation/source/delaybuffer.cpp \
	$(SRC_PATH)/arm64/acoustic_echo_cancellation/source/delayest.cpp \
	$(SRC_PATH)/arm64/acoustic_echo_cancellation/source/noiseest.cpp \
	$(SRC_PATH)/arm64/acoustic_echo_cancellation/source/noisereduction.cpp \
	$(SRC_PATH)/arm64/acoustic_echo_cancellation/source/postfilter.cpp \
	$(SRC_PATH)/arm64/acoustic_echo_cancellation/source/PSDsmooth.cpp \
	$(SRC_PATH)/arm64/acoustic_echo_cancellation/source/SpeechEst.cpp \
	$(SRC_PATH)/arm64/acoustic_echo_cancellation/source/SubbandAdap.cpp \
	$(SRC_PATH)/arm64/audio_processing/source/AudioProcessingFramework.cpp \
	$(SRC_PATH)/arm64/audio_processing/source/AudioAecCpp.cpp \
	$(SRC_PATH)/arm64/audio_processing/source/SUBinterface.cpp \
	$(SRC_PATH)/arm64/common_audio/source/basemath.cpp \
	$(SRC_PATH)/arm64/common_audio/source/DCRemover.cpp \
	$(SRC_PATH)/arm64/common_audio/source/F2Ttransformer.cpp \
	$(SRC_PATH)/arm64/common_audio/source/SPEst.cpp \
	$(SRC_PATH)/arm64/common_audio/source/T2Ftransformer.cpp \
	$(SRC_PATH)/arm64/beam_forming/source/MVDR.cpp \
	$(SRC_PATH)/arm64/vad/source/VAD.cpp \
	$(SRC_PATH)/arm64/AudioLog.cpp

ARM64_ALL_CPP_FILES = \
	$(SRC_PATH)/arm64/acoustic_echo_cancellation/source/AcousticEchoCancellation.cpp \
	$(SRC_PATH)/arm64/acoustic_echo_cancellation/source/AdapFilterGroup.cpp \
	$(SRC_PATH)/arm64/acoustic_echo_cancellation/source/delaybuffer.cpp \
	$(SRC_PATH)/arm64/acoustic_echo_cancellation/source/delayest.cpp \
	$(SRC_PATH)/arm64/acoustic_echo_cancellation/source/noiseest.cpp \
	$(SRC_PATH)/arm64/acoustic_echo_cancellation/source/noisereduction.cpp \
	$(SRC_PATH)/arm64/acoustic_echo_cancellation/source/postfilter.cpp \
	$(SRC_PATH)/arm64/acoustic_echo_cancellation/source/PSDsmooth.cpp \
	$(SRC_PATH)/arm64/acoustic_echo_cancellation/source/SpeechEst.cpp \
	$(SRC_PATH)/arm64/acoustic_echo_cancellation/source/SubbandAdap.cpp \
	$(SRC_PATH)/arm64/acoustic_echo_cancellation/unitest/aec_unitest/test.cpp \
	$(SRC_PATH)/arm64/audio_processing/source/AudioAecCpp.cpp \
	$(SRC_PATH)/arm64/audio_processing/source/AudioProcessingFramework.cpp \
	$(SRC_PATH)/arm64/audio_processing/source/SUBinterface.cpp \
	$(SRC_PATH)/arm64/audio_processing/unitest/aec_unitest/test.cpp \
	$(SRC_PATH)/arm64/AudioLog.cpp \
	$(SRC_PATH)/arm64/beam_forming/source/MVDR.cpp \
	$(SRC_PATH)/arm64/common_audio/SampleRateConvert/src/SampleRateConvert.cpp \
	$(SRC_PATH)/arm64/common_audio/source/AudioDataSorter.cpp \
	$(SRC_PATH)/arm64/common_audio/source/basemath.cpp \
	$(SRC_PATH)/arm64/common_audio/source/DCRemover.cpp \
	$(SRC_PATH)/arm64/common_audio/source/F2Ttransformer.cpp \
	$(SRC_PATH)/arm64/common_audio/source/PitchTrack.cpp \
	$(SRC_PATH)/arm64/common_audio/source/SilenceDetector.cpp \
	$(SRC_PATH)/arm64/common_audio/source/SPEst.cpp \
	$(SRC_PATH)/arm64/common_audio/source/T2Ftransformer.cpp \
	$(SRC_PATH)/arm64/mixer/source/AudioMixer.cpp \
	$(SRC_PATH)/arm64/mixer/source/AudioMixImpl.cpp \
	$(SRC_PATH)/arm64/mixer/source/Clip.cpp \
	$(SRC_PATH)/arm64/mixer/source/Compandor.cpp \
	$(SRC_PATH)/arm64/mixer/source/Mix.cpp \
	$(SRC_PATH)/arm64/mixer/unitest/test.cpp \
	$(SRC_PATH)/arm64/resample/source/ReSampleImpl.cpp \
	$(SRC_PATH)/arm64/resample/unittest/resample_unit.cpp \
	$(SRC_PATH)/arm64/vad/source/VAD.cpp

ifeq ($(TARGET_ARCH),arm64)
CPP_FILES = $(ARM64_CPP_FILES)
else
CPP_FILES = $(GENERIC_CPP_FILES)
endif

COBJ_FILES = $(addprefix $(OBJ_PATH)/,$(notdir $(C_FILES:.c=.o)))
CPPOBJ_FILES = $(addprefix $(OBJ_PATH)/,$(notdir $(CPP_FILES:.cpp=.o)))

OBJ_FILES = $(COBJ_FILES) $(CPPOBJ_FILES)

#compile and lib parameter
CROSS_COMPILE ?= $(ROOT_PATH)/tools/rk3308gClientSDK/prebuilts/host/usr/bin/arm-rockchip-linux-gnueabihf-
CC      ?= $(CROSS_COMPILE)gcc
CXX     ?= $(CROSS_COMPILE)g++
LD      ?= $(CROSS_COMPILE)ld
AR      ?= $(CROSS_COMPILE)ar

DEFINES ?= -fPIC -fpermissive -Wl,-rpath=. -Wl,-soname,libAPF.so
INCLUDE := -Iinclude -I./extlib

ifneq (,$(findstring -DARM_NEON,$(DEFINES)))
NE10_LIBS ?= -L$(ROOT_PATH)/thirdparty/ne10/sigmastar/lib -lNE10 -lm
endif
#
#CFLAGS  := -Wall -O3 -g -fno-omit-frame-pointer $(DEFINES) $(INCLUDE) $(INCLUDES)
CFLAGS  := -Wall -O3 $(ARM64_OPT_FLAGS) $(DEFINES) $(INCLUDE) $(INCLUDES)

CXXFLAGS:= $(CFLAGS)

ifneq (,$(findstring darwin,$(shell $(CC) -dumpmachine 2>/dev/null)))
ARM64_TEST_GC_LDFLAGS ?= -Wl,-dead_strip
else
ARM64_TEST_GC_LDFLAGS ?= -Wl,--gc-sections
endif

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

arm64-source-check:
	@if test "$(TARGET_ARCH)" != "arm64"; then \
		echo "Error: arm64-source-check requires TARGET_ARCH=arm64"; \
		exit 1; \
	fi
	@for src in $(ARM64_ALL_CPP_FILES); do \
		if test ! -f "$$src"; then \
			echo "Error: missing ARM64 C++ source $$src"; \
			exit 1; \
		fi; \
	done
	@for src in $(ARM64_ALL_C_FILES); do \
		echo "Checking $$src"; \
		case "$$src" in \
			*SampleRateConvert*) extra_include="-I./common_audio/SampleRateConvert/include" ;; \
			*) extra_include="" ;; \
		esac; \
		$(CC) $$extra_include $(CFLAGS) -fsyntax-only "$$src" || exit 1; \
	done
	@for src in $(ARM64_CPP_FILES); do \
		echo "Checking $$src"; \
		case "$$src" in \
			*SampleRateConvert*) extra_include="-I./common_audio/SampleRateConvert/include" ;; \
			*AudioDataSorter.cpp) extra_include="-I$(ROOT_PATH)/sorce/AudioBufferBase/include -I$(ROOT_PATH)/sorce/AudioUtility/interface" ;; \
			*"/mixer/"*) extra_include="-I./mixer/include -I./mixer/include/interface -I$(ROOT_PATH)/sorce/AudioUtility/interface -I$(ROOT_PATH)/tools/WaveFunction" ;; \
			*"/resample/"*) extra_include="-I./resample/include -I./common_audio/SampleRateConvert/include -I$(ROOT_PATH)/tools/WaveFunction" ;; \
			*"/unitest/"*) extra_include="-DAUDIO_WAVE_DEBUG -I$(ROOT_PATH)/tools/WaveFunction" ;; \
			*) extra_include="" ;; \
		esac; \
		$(CXX) $$extra_include $(CXXFLAGS) -fsyntax-only "$$src" || exit 1; \
	done

$(OBJ_PATH)/arm64_test.o: $(SRC_PATH)/arm64/tests/arm64_parity_test.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_PATH)/arm64_test_scalar_amp.o: $(SRC_PATH)/common_audio/source/Amp_compute.c
	$(CC) $(CFLAGS) \
		-DClipping=scalar_Clipping \
		-DGetPowerLevel=scalar_GetPowerLevel \
		-DGetPowerEnergy=scalar_GetPowerEnergy \
		-DGetPowerdB=scalar_GetPowerdB \
		-DGetPowerdBShort=scalar_GetPowerdBShort \
		-DChangeDbToWebExLevel=scalar_ChangeDbToWebExLevel \
		-DShortToFloat=scalar_ShortToFloat \
		-DFloatToShort=scalar_FloatToShort \
		-c $< -o $@

$(OBJ_PATH)/arm64_test_neon_amp.o: $(SRC_PATH)/arm64/common_audio/source/Amp_compute.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_PATH)/arm64_test_scalar_rnn.o: $(SRC_PATH)/rnnoise/source/rnn.c
	$(CC) $(CFLAGS) \
		-Dcompute_dense=scalar_compute_dense \
		-Dcompute_gru=scalar_compute_gru \
		-Dcompute_rnn=scalar_compute_rnn \
		-c $< -o $@

$(OBJ_PATH)/arm64_test_neon_rnn.o: $(SRC_PATH)/arm64/rnnoise/source/rnn.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_PATH)/arm64_test_scalar_denoise.o: $(SRC_PATH)/rnnoise/source/denoise.c
	$(CC) $(CFLAGS) -ffunction-sections \
		-Dcompute_band_energy=scalar_compute_band_energy \
		-Dcompute_band_corr=scalar_compute_band_corr \
		-Dinterp_band_gain=scalar_interp_band_gain \
		-Dcommon=scalar_common \
		-Drnnoise_get_size=scalar_rnnoise_get_size \
		-Drnnoise_get_frame_size=scalar_rnnoise_get_frame_size \
		-Drnnoise_init=scalar_rnnoise_init \
		-Drnnoise_create=scalar_rnnoise_create \
		-Drnnoise_destroy=scalar_rnnoise_destroy \
		-Dpitch_filter=scalar_pitch_filter \
		-Drnnoise_process_frame=scalar_rnnoise_process_frame \
		-Dget_rnn_gain=scalar_get_rnn_gain \
		-c $< -o $@

$(OBJ_PATH)/arm64_test_neon_denoise.o: $(SRC_PATH)/arm64/rnnoise/source/denoise.c
	$(CC) $(CFLAGS) -ffunction-sections -c $< -o $@

$(OBJ_PATH)/arm64_test_scalar_celt_lpc.o: $(SRC_PATH)/rnnoise/source/celt_lpc.c
	$(CC) $(CFLAGS) -ffunction-sections \
		-D_celt_lpc=scalar_celt_lpc \
		-Dcelt_fir=scalar_celt_fir \
		-Dcelt_iir=scalar_celt_iir \
		-D_celt_autocorr=scalar_celt_autocorr \
		-c $< -o $@

$(OBJ_PATH)/arm64_test_neon_celt_lpc.o: $(SRC_PATH)/arm64/rnnoise/source/celt_lpc.c
	$(CC) $(CFLAGS) -ffunction-sections -c $< -o $@

$(OBJ_PATH)/arm64_test_scalar_src_linear.o: $(SRC_PATH)/common_audio/SampleRateConvert/src/src_linear.c
	$(CC) -I./common_audio/SampleRateConvert/include $(CFLAGS) \
		-DHAVE_LRINT=1 -DHAVE_LRINTF=1 \
		-Dlinear_get_name=scalar_linear_get_name \
		-Dlinear_get_description=scalar_linear_get_description \
		-Dlinear_set_converter=scalar_linear_set_converter \
		-c $< -o $@

$(OBJ_PATH)/arm64_test_neon_src_linear.o: $(SRC_PATH)/arm64/common_audio/SampleRateConvert/src/src_linear.c
	$(CC) -I./common_audio/SampleRateConvert/include $(CFLAGS) \
		-DHAVE_LRINT=1 -DHAVE_LRINTF=1 -c $< -o $@

$(ARM64_TEST_BIN): checkbindir checkdir $(ARM64_TEST_OBJS)
	$(CC) $(CFLAGS) $(ARM64_TEST_GC_LDFLAGS) -o $@ $(ARM64_TEST_OBJS) -lm

arm64-parity-test: $(ARM64_TEST_BIN)

$(OBJ_PATH)/arm64_cpp_test.o: $(SRC_PATH)/arm64/tests/arm64_cpp_parity_test.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_PATH)/arm64_cpp_test_scalar_spest.o: $(SRC_PATH)/common_audio/source/SPEst.cpp
	$(CXX) $(CXXFLAGS) -DSPEst=ScalarSPEst -c $< -o $@

$(OBJ_PATH)/arm64_cpp_test_neon_spest.o: $(SRC_PATH)/arm64/common_audio/source/SPEst.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_PATH)/arm64_cpp_test_scalar_dc.o: $(SRC_PATH)/common_audio/source/DCRemover.cpp
	$(CXX) $(CXXFLAGS) -DDCRemover=ScalarDCRemover -c $< -o $@

$(OBJ_PATH)/arm64_cpp_test_neon_dc.o: $(SRC_PATH)/arm64/common_audio/source/DCRemover.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_PATH)/arm64_cpp_test_scalar_psd.o: $(SRC_PATH)/acoustic_echo_cancellation/source/PSDsmooth.cpp
	$(CXX) $(CXXFLAGS) -ffunction-sections \
		-DCPSDsmooth=ScalarCPSDsmooth -c $< -o $@

$(OBJ_PATH)/arm64_cpp_test_neon_psd.o: $(SRC_PATH)/arm64/acoustic_echo_cancellation/source/PSDsmooth.cpp
	$(CXX) $(CXXFLAGS) -ffunction-sections -c $< -o $@

$(OBJ_PATH)/arm64_cpp_test_scalar_psd_adapter.o: $(SRC_PATH)/arm64/tests/psd_test_adapter.cpp
	$(CXX) $(CXXFLAGS) -DCPSDsmooth=ScalarCPSDsmooth \
		-DAPF_SCALAR_PSD_ADAPTER -c $< -o $@

$(OBJ_PATH)/arm64_cpp_test_neon_psd_adapter.o: $(SRC_PATH)/arm64/tests/psd_test_adapter.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_PATH)/arm64_cpp_test_scalar_postfilter.o: $(SRC_PATH)/acoustic_echo_cancellation/source/postfilter.cpp
	$(CXX) $(CXXFLAGS) -ffunction-sections \
		-DCPostFilter=ScalarCPostFilter -Dwin=scalar_postfilter_win \
		-c $< -o $@

$(OBJ_PATH)/arm64_cpp_test_neon_postfilter.o: $(SRC_PATH)/arm64/acoustic_echo_cancellation/source/postfilter.cpp
	$(CXX) $(CXXFLAGS) -ffunction-sections -c $< -o $@

$(OBJ_PATH)/arm64_cpp_test_scalar_postfilter_adapter.o: $(SRC_PATH)/arm64/tests/postfilter_test_adapter.cpp
	$(CXX) $(CXXFLAGS) -DCPostFilter=ScalarCPostFilter \
		-DAPF_SCALAR_POSTFILTER_ADAPTER -c $< -o $@

$(OBJ_PATH)/arm64_cpp_test_neon_postfilter_adapter.o: $(SRC_PATH)/arm64/tests/postfilter_test_adapter.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_PATH)/arm64_cpp_test_scalar_adap.o: $(SRC_PATH)/acoustic_echo_cancellation/source/AdapFilterGroup.cpp
	$(CXX) $(CXXFLAGS) -ffunction-sections \
		-DCAdapFilterGroup=ScalarCAdapFilterGroup -c $< -o $@

$(OBJ_PATH)/arm64_cpp_test_neon_adap.o: $(SRC_PATH)/arm64/acoustic_echo_cancellation/source/AdapFilterGroup.cpp
	$(CXX) $(CXXFLAGS) -ffunction-sections -c $< -o $@

$(OBJ_PATH)/arm64_cpp_test_scalar_adap_adapter.o: $(SRC_PATH)/arm64/tests/adap_filter_test_adapter.cpp
	$(CXX) $(CXXFLAGS) -DCAdapFilterGroup=ScalarCAdapFilterGroup \
		-DAPF_SCALAR_ADAP_ADAPTER -c $< -o $@

$(OBJ_PATH)/arm64_cpp_test_neon_adap_adapter.o: $(SRC_PATH)/arm64/tests/adap_filter_test_adapter.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(ARM64_CPP_TEST_BIN): checkbindir checkdir $(ARM64_CPP_TEST_OBJS)
	$(CXX) $(CXXFLAGS) $(ARM64_TEST_GC_LDFLAGS) -o $@ $(ARM64_CPP_TEST_OBJS)

arm64-cpp-parity-test: $(ARM64_CPP_TEST_BIN)

arm64-function-parity-test: arm64-source-check arm64-parity-test arm64-cpp-parity-test
	@sh $(SRC_PATH)/arm64/tests/check_source_mapping.sh $(SRC_PATH)
	@$(ARM64_TEST_BIN)
	@$(ARM64_CPP_TEST_BIN)

.PHONY: all static share arm64-source-check arm64-parity-test \
	arm64-cpp-parity-test arm64-function-parity-test clean
clean:
	@rm -rf $(OBJ_FILES) $(ARM64_TEST_OBJS) $(ARM64_CPP_TEST_OBJS) \
		$(ARM64_TEST_BIN) $(ARM64_CPP_TEST_BIN) $(STATIC_LIB) $(SHARED_LIB)
