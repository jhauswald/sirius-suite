CXX = g++
LINK_FLAGS = -lpthread
CXX_FLAGS = -O2
LINK_FLAGS += /opt/intel/vtune_amplifier_xe/lib64/libittnotify.a -lpthread -ldl
CXX_FLAGS  += -I/opt/intel/vtune_amplifier_xe/include/

THREADS = 4
SCORES  = 1
INPUT   = ../input/gmm_data.txt

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
	@./$(EXEC) $(THREADS) $(SCORES) $(INPUT)
	-@diff -q ../input/$(EXEC).baseline ../input/$(EXEC).pthread

# To remove generated files
clean:
	$(RM) -f $(EXEC) $(OBJECTS) *.o ../input/$(EXEC).pthread
