FLAGS = -Wall -g
TEST_FLAGS = 
LIBRARIES = -lSDL2 
TEST_LIBRARIES = $(LIBRARIES) \
								 -lcriterion

INCLUDES = -I. \

BUILDS = ./build
TARGETS = $(BUILDS)/targets
TEST_TARGET = $(TARGETS)/test
BINARY_NAME = dlib
BUILD_TARGET = $(TARGETS)/$(BINARY_NAME)


# Recursively find all *_test.c files in the current directory and subdirectories
test-sources-tests := $(shell find . -name '*_test.c')
test-sources-sources := $(test-sources-tests:_test.c=.c)
test-sources := $(test-sources-tests) $(test-sources-sources)

# Recursively find all source files (filter out test sources)
sources-all:= $(shell find . -name '*.c')
sources-no-tests := $(filter-out $(test-sources-tests) ./main.c, $(sources-all))
sources-no-ft := $(patsubst %.c,%,$(sources-no-tests))

# Generate corresponding object files for non-test sources
objects-names-only := $(patsubst ./%, %, $(sources-no-ft))
objects := $(patsubst %,$(BUILDS)/%.o,$(objects-names-only))
build-filepaths := $(sort $(dir $(objects)))


build_directory:
	@ mkdir $(TARGETS) $(build-filepaths) -p

build_objects: build_directory
	@ $(foreach var,$(sources-no-ft),cc $(var).c $(INCLUDES) $(FLAGS) $(LIBRARIES) -c -o $(BUILDS)/$(var).o;)

build: build_objects
	@ cc main.c $(objects) $(FLAGS) $(INCLUDES) $(LIBRARIES) -o $(TARGETS)/$(BINARY_NAME)

run: build
	@ $(BUILD_TARGET)

build_tests: build_directory
	@ cc $(test-sources) $(FLAGS) $(INCLUDES) $(TEST_LIBRARIES) -o $(TEST_TARGET)

test: build_tests
	@ $(TEST_TARGET) $(TEST_FLAGS)


clean:
	@ rm -rf $(BUILDS)
	@ echo --cleaned--
