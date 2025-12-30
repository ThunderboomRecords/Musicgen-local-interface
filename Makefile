# Compiler and flags
CXX = g++
CXXFLAGS = -I./include/onnxruntime -I./include/nlohmann -I./include/sentencepiece -std=c++17
LDFLAGS = -L./src/onnxruntime -lonnxruntime -L./src/sentencepiece -lsentencepiece -lsndfile
RPATH_FLAGS = -Wl,-rpath,'$$ORIGIN/./src/onnxruntime:$$ORIGIN/./src/sentencepiece:$$ORIGIN/./src' -Wl,--enable-new-dtags 

# Target executable
TARGET = test_model

# Source files
SRC = processor.cpp ./src/tokenizer.cpp

# Build target
all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS) $(RPATH_FLAGS)

# Clean the build
clean:
	rm -f $(TARGET)
