
CXX=$(ROCM_PATH)/bin/hipcc

CXXFLAGS = -O3 -std=c++11 -g -L${ROCM_PATH}/lib -lrocm_smi64

SOURCES = src/main.cpp 
OBJECTS = $(SOURCES:.cpp=.o)

EXECUTABLE=./power_measure

.PHONY: test


all: $(EXECUTABLE) test


$(EXECUTABLE): $(OBJECTS) 
	$(CXX) ${CXXFLAGS} $(OBJECTS) -o $@


# test: $(EXECUTABLE)
# 	$(EXECUTABLE)


clean:
	rm -f $(EXECUTABLE)
	rm -f $(OBJECTS)