CXX = gcc
CXX_FLAGS  = -O3
LINK_FLAGS =
LINK_FLAGS += /opt/intel/vtune_amplifier_xe/lib64/libittnotify.a -lpthread -ldl -lm
CXX_FLAGS  += -I/opt/intel/vtune_amplifier_xe/include/

SCORES = 1
INPUT  = ../input/gmm_data.txt

# File names
EXEC = gmm_scoring
SOURCES = gmm_scoring.cpp ../../utils/timer.cpp
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
	@./$(EXEC) $(SCORES) $(INPUT)

# To remove generated files
clean:
	$(RM) -f $(EXEC) $(OBJECTS) *.o ../input/$(EXEC).baseline
