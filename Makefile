.PHONY:clean all

EXE = ICNNode

LIBS = pthread
LIBS_SEARCH_PATH =.
INCLUDES =

CXXFLAGS = -ggdb3 -O3 -Wall -Wextra  #-Wconversion -Wshadow
#CXXFLAGS = -ggdb3 -O2 -Wall -Wextra -Werror #-Wconversion -Wshadow


CC = g++ --std=gnu++11
RM = rm -rf
MKDIR = mkdir -p

DIR_BUILD = build
DIR_OBJS = objs
DIR_DEPS = deps

DIR_OBJS := $(addprefix $(DIR_BUILD)/,$(DIR_OBJS))
DIR_DEPS := $(addprefix $(DIR_BUILD)/,$(DIR_DEPS))
DIRS = $(DIR_BUILD) $(DIR_OBJS) $(DIR_DEPS)

SRCS = $(wildcard *.cpp)
OBJS = $(addprefix $(DIR_OBJS)/,$(SRCS:.cpp=.o))
DEPS = $(addprefix $(DIR_DEPS)/,$(SRCS:.cpp=.dep))

#EXE := $(addprefix $(DIR_BUILD)/,$(EXE))

ifneq ($(LIBS),"")
LIBS := $(addprefix -l,$(LIBS))
endif

ifneq ($(LIBS_SEARCH_PATH),"")
LIBS_SEARCH_PATH := $(addprefix -L,$(LIBS_SEARCH_PATH))
endif

ifneq ($(INCLUDES),"")
INCLUDES := $(addprefix -I,$(INCLUDES))
endif

$(shell mkdir -p $(DIRS))

all:$(EXE)

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif

$(EXE):$(OBJS)
	$(CC) -o $@ $(filter %.o, $^) $(LIBS_SEARCH_PATH) $(LIBS)

$(DIR_DEPS)/%.dep: %.cpp
	@echo "Making $@ ..."
	@set -e;\
		$(CC) $(INCLUDES) -E -MM $(filter %.cpp, $^) | sed 's,\(.*\)\.o[ :]*,$(DIR_OBJS)/\1.o $@:,g' > $@

$(DIR_OBJS)/%.o: %.cpp
	$(CC) $(INCLUDES) $(CXXFLAGS) -o $@ -c $(filter %.cpp, $^)

clean:
	$(RM) $(DIR_OBJS) $(DIR_DEPS)
