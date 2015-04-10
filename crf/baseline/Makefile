CXX = g++
CXX_FLAGS  = -O3
LINK_FLAGS += /opt/intel/vtune_amplifier_xe/lib64/libittnotify.a -lpthread -ldl
CXX_FLAGS  += -I/opt/intel/vtune_amplifier_xe/include/

MODEL    = ../input/model.la
INPUT    = ../input/test-input.txt

# File names
EXEC = crf_tag
SOURCES = $(wildcard *.cpp) ../../utils/timer.cpp
OBJECTS = $(SOURCES:.cpp=.o)

.PHONY: all test

all : $(EXEC)
$(EXEC): $(OBJECTS) Makefile
	$(CXX) $(OBJECTS) -o $(EXEC) $(LINK_FLAGS)

# To obtain object files
%.o: %.cpp Makefile
	$(CXX) -c $(CXX_FLAGS) $(EXTRA_FLAGS) $< -o $@

# test
test: CXX_FLAGS += -DTESTING
test: all
	@./$(EXEC) $(MODEL) $(INPUT)

# To remove generated files
clean:
	$(RM) -f $(EXEC) $(OBJECTS) *.o ../input/$(EXEC).baseline
