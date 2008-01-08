CXX = g++
CXXFLAGS += -Wall -O3 -g

# Set up include paths for OpenCV and Teem
CXXFLAGS += `pkg-config --cflags opencv`
CXXFLAGS += -I/usr/local/include/

# Set up library paths and flags for OpenCV and Teem
LDFLAGS += -L/usr/local/lib/
LIBS += `pkg-config --libs opencv`
LIBS += -lteem

MAINSOURCES := $(foreach dir,.,$(notdir $(wildcard $(dir)/*.cpp)))
MAINOBJECTS := $(MAINSOURCES:.cpp=.o)

# Use gcc's built-in dependency generation
CXXFLAGS += -MMD -MP
DEPENDS := $(MAINOBJECTS:.o=.d)

BINARY := autoseg

# The targets all and clean do not actually generate files named all and clean
.PHONY: all clean run

all: $(BINARY)

run: $(BINARY)
	./$(BINARY) 208 218 scan1_400.nhdr

clean:
	rm -f $(BINARY) $(MAINOBJECTS) $(DEPENDS)

$(BINARY): $(MAINOBJECTS)
	$(CXX) $^ -o $@ $(LDFLAGS) $(LIBS)

-include $(DEPENDS)
