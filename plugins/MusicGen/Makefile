NAME = MusicGenVST
CXXFLAGS += -std=c++17 -Wall -Wextra

UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

ifeq ($(UNAME_S), Linux)
	CXXFLAGS += -I/usr/include/jsoncpp -I/usr/include -I/opt/homebrew/include -DLINUX
	LDFLAGS += -lcurl -ljsoncpp -lsndfile
endif

ifeq ($(UNAME_S), Darwin)
	CFXXLAGS += -mmacosx-version-min=12.0
	LDFLAGS += -mmacosx-version-min=12.0
	ifeq ($(UNAME_M), arm64)
		# Arm64 (Apple Silicon) specific flags
		CXXFLAGS += -I/usr/local/include/jsoncpp \
					-I/usr/local/include -I/opt/homebrew/include \
					-DDARWIN -arch arm64
		LDFLAGS += -lcurl -L/opt/homebrew/lib -ljsoncpp -lsndfile
	else ifeq ($(UNAME_M), x86_64)
		# x86_64 (Intel) specific flags
		CXXFLAGS += -I/usr/local/include/jsoncpp \
					-I/usr/local/include \
					-DDARWIN -arch x86_64 -O2
		LDFLAGS += -lcurl -L/usr/local/lib -ljsoncpp -lsndfile
	endif
endif

ifeq ($(OS), Windows_NT)
	# Windows-specific flags can be added here
endif

# --------------------------------------------------------------
# Files to build

FILES_DSP = \
	MusicGen.cpp

FILES_UI  = \
	MusicGenUI.cpp \
	./src/SimpleButton.cpp \
	./src/Knob.cpp \
	./src/TextInput.cpp \
	./src/NumberInput.cpp \
	./src/Panel.cpp \
	./src/Label.cpp \
	./src/Checkbox.cpp \
	./src/ValueIndicator.cpp \
	./src/WidgetGroup.cpp \
	./src/Layout.cpp \
	./src/WAIVEWidget.cpp \
	tinyfiledialogs.c

# --------------------------------------------------------------
# Do some magic

include ../../dpf/Makefile.plugins.mk

# --------------------------------------------------------------
# VST2, LV2, and Standalone targets

TARGETS += vst
TARGETS += vst3
ifeq ($(HAVE_DGL), true)
TARGETS += lv2_sep
else
TARGETS += lv2_dsp
endif

TARGETS += jack

all: $(TARGETS)