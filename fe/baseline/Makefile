CXX = g++
CXX_FLAGS = -O3
LINK_FLAGS = -lopencv_core \
			 -lopencv_highgui \
			 -lopencv_imgproc \
			 -lopencv_nonfree \
			 -lopencv_features2d
LINK_FLAGS += /opt/intel/vtune_amplifier_xe/lib64/libittnotify.a -lpthread -ldl
CXX_FLAGS  += -I/opt/intel/vtune_amplifier_xe/include/

INPUT = ../input/2048x2048.jpg

# File names
EXEC = surf-fe
SOURCES = surf-fe.cpp ../../utils/timer.cpp
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
	@./$(EXEC) $(INPUT)

# To remove generated files
clean:
	$(RM) $(EXEC) $(OBJECTS) *.o ../input/$(EXEC).baseline.jpg
