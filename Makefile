IDIR=inc
CXX=gcc
CXXFLAGS=-I$(IDIR)/ -c -Wall -Wpedantic -Werror -std=c11 -O3 \
		 -D_XOPEN_SOURCE=700
SHAREDFLAGS=-pthread

OBJ_DIR=obj
SRC_DIR=src
TEST_DIR=test
SRC_SUB_DIRS=lib tils
ALL_DIRS=$(SRC_SUB_DIRS:%=$(OBJ_DIR)/%)

EXECUTABLE=tils

TEST_EXECUTABLE=test-tils

# Files needed only by c-http executable
TILS_SRCS=main.c tils/routes.c tils/worker_thread.c tils/io_util.c \
    tils/accept.c tils/request.c tils/serve.c tils/conn.c \
	tils/tils.c lib/hashtable.c lib/logging.c lib/queue.c

# Files required by unit tests & c-http executable
SHRD_SRCS=

# Files required only by unit tests
TEST_SRCS=

SHRD_OBJS=$(SHRD_SRCS:%.c=$(OBJ_DIR)/%.o)

TILS_OBJS=$(TILS_SRCS:%.c=$(OBJ_DIR)/%.o)

TEST_OBJS=$(TEST_SRCS:%.c=$(OBJ_DIR)/%.o)

.PHONY: all clean dirs test

all: dirs $(EXECUTABLE)

test: dirs $(TEST_EXECUTABLE)
	
$(TEST_EXECUTABLE): $(SHRD_OBJS) $(TEST_OBJS)
	$(CXX) $^ -o $(TEST_EXECUTABLE) $(SHAREDFLAGS)

$(EXECUTABLE): $(SHRD_OBJS) $(TILS_OBJS)
	$(CXX) $^ -o $(EXECUTABLE) $(SHAREDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CXX) $(CXXFLAGS) $(SHAREDFLAGS) $< -o $@

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.c
	$(CXX) $(CXXFLAGS) $(SHAREDFLAGS) $< -o $@

dirs: 
	-mkdir -p $(OBJ_DIR) $(ALL_DIRS)

clean:
	-rm -rf $(OBJ_DIR)
	-rm $(EXECUTABLE)
	-rm $(TEST_EXECUTABLE)

#-include $(OBJS:%.o=%.d)
