CXX=g++
CXX_FLAGS = -O2 -DCPU_ONLY
# LINK_FLAGS = -lcaffe \
# 			 -lopenblas \
# 			 -lglog
# LINK_FLAGS = -lcaffe \
# 			 libopenblas-noavx.a \
# 			 -lglog
LINK_FLAGS = libcaffe-noavx.a \
			 -lprotobuf \
			 -lgflags -lleveldb -lsnappy \
			 -llmdb -lhdf5 -lhdf5_hl -lboost_thread -lpthread -lopencv_core -lopencv_imgproc \
			 libopenblas-noavx.a \
			 -lglog
# LINK_FLAGS = libcaffe-noavx.a \
# 			 -lprotobuf \
# 			 -lgflags -lleveldb -lsnappy \
# 			 -llmdb -lhdf5 -lhdf5_hl -lboost_thread -lpthread -lopencv_core -lopencv_imgproc \
# 			 libopenblas-noavx.a \
# 			 -lglog
LINK_FLAGS += /opt/intel/vtune_amplifier_xe/lib64/libittnotify.a -lpthread -ldl
CXX_FLAGS  += -I/opt/intel/vtune_amplifier_xe/include/

MODEL   = ../model/asr.prototxt
WEIGHTS = ../model/asr.caffemodel
INPUT   = ../input/features.in

EXEC = dnn_asr
SOURCES=dnn-asr.cpp ../../utils/timer.cpp
OBJECTS=$(SOURCES:.cpp=.o)

.PHONY: all test clean

all : $(EXEC)
$(EXEC): $(OBJECTS) Makefile
	$(CXX) $(OBJECTS) -o $(EXEC) $(LINK_FLAGS)

# To obtain object files
%.o: %.cpp Makefile
	$(CXX) -c $(CXX_FLAGS) $(EXTRA_FLAGS) $< -o $@

# test
test: CXX_FLAGS += -DTESTING
test: all
	@./$(EXEC) $(MODEL) $(WEIGHTS) $(PASSES) $(INPUT)

# To remove generated files
clean:
	rm -f $(EXEC) $(OBJECTS) *.o
