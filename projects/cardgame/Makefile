CXX ?= g++
CPPFLAGS += -Iinclude
CXXFLAGS ?= -O2 -g
CXXFLAGS += -std=c++17 -Wall -Wextra -Wpedantic -Werror -MMD -MP
LDFLAGS ?=
BUILD_DIR ?= build
BIN_DIR ?= bin

SOURCES := $(sort $(wildcard src/carte/*.cpp src/plateau/*.cpp src/jeu/*.cpp src/vue/*.cpp))
OBJECTS := $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))
MAIN := $(BUILD_DIR)/Main.o
TEST := $(BUILD_DIR)/tests/TestProjet.o
TARGET := $(BIN_DIR)/jeuCartes
TEST_TARGET := $(BIN_DIR)/tests

.DEFAULT_GOAL := all
.PHONY: all test smoke sanitize clean
all: $(TARGET)

$(TARGET): $(OBJECTS) $(MAIN)
	@mkdir -p $(@D)
	$(CXX) $^ $(LDFLAGS) -o $@

$(TEST_TARGET): $(OBJECTS) $(TEST)
	@mkdir -p $(@D)
	$(CXX) $^ $(LDFLAGS) -o $@

$(BUILD_DIR)/%.o: src/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/tests/%.o: tests/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

test: $(TARGET) $(TEST_TARGET)
	$(TEST_TARGET)
	python3 tests/test_cli.py $(TARGET)

smoke: $(TARGET)
	python3 tests/test_cli.py $(TARGET)

# Dedicated paths avoid mixing release and instrumented objects.
sanitize:
	$(MAKE) BUILD_DIR=build/sanitize BIN_DIR=bin/sanitize CXXFLAGS='-O1 -g -std=c++17 -Wall -Wextra -Wpedantic -Werror -MMD -MP -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie' LDFLAGS='-fsanitize=address,undefined -no-pie' test

clean:
	rm -rf build bin

-include $(OBJECTS:.o=.d) $(MAIN:.o=.d) $(TEST:.o=.d)
