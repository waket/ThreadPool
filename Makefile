# The C++ compiler to use
CXX = g++
# Do not compute dependencies for CPP files using these targets
CXXNODEPS = clean
# The directories used by the build
DIR_SOURCE = .
DIR_OUTPUT = output
# You could add new include paths or libs to link here
CXXFLAGS = -I.
CXXLIBS = -pthread

# The CPP files that will be included in the build by default are all of the
# .cpp files in the directory.
CXXFILES := $(shell find $(DIR_SOURCE)/ -maxdepth 1 -name "*.cpp")
# We generate a dependency file for every cpp file as well.
CXXFILESDEP := $(patsubst %.cpp,%.d,$(DIR_SOURCE))

# Do not include the dependencies files as the dependencies to the .o's if we
# are doing a clean.
ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(CXXNODEPS))))
    -include $(DEPFILES)
endif

# How to generate dependencies from the c++ files.
$(DIR_SOURCE)/%.d: $(DIR_SOURCE)/%.cpp
	@mkdir -p $(DIR_OUTPUT)
	@echo "Generating dependencies for $<"...
	$(CXX) $(CXXFLAGS) -MM -MT '$(patsubst $(DIR_SOURCE)/%.cpp,$(DIR_OUTPUT)/%.o,$<)' $< -MF $@

# Dependencies for a given .o file will be any changes to the headers, .cpp, or the
# .d (generated dependencies) for any of them.
$(DIR_OUTPUT)/%.o: $(DIR_SOURCE)/%.cpp $(DIR_SOURCE)/%.d
	@mkdir -p $(DIR_OUTPUT)
	@echo "Compiling $<..."
	$(CXX) -c -o $@ $< $(CXXFLAGS)

# Default target, just build threadpooltask and all dependencies
# It's default because it's the first in the makefile!
all: threadpooltask

# Separate clean targets in case selective cleans are warranted
clean-artifacts:
	@echo "Cleaning build artifacts..."
	rm -rf $(DIR_OUTPUT)

clean-executable:
	@echo "Cleaning executable..."
	rm -f threadpooltask

clean: clean-artifacts clean-executable

# Target to build the main server executable
threadpooltask: $(patsubst %.cpp,%.o,$(patsubst $(DIR_SOURCE)/%,$(DIR_OUTPUT)/%,$(CXXFILES)))
	@echo "Building threadpooltask..."
	$(CXX) $(CXXFLAGS) -o $@ $(CXXLIBS) $^
	rm -rf $(DIR_OUTPUT)

# rebuild, clean up and do full build
rebuild: clean all

