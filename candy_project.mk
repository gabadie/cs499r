
# ------------------------------------------------------------------------------ CS499R library's headers directory

# ------------------------------------------------------------ configurations
PROJECT_GCC_ENV := $(call env_preference, gcc-4.9 gcc-4.8 gcc-4.7 gcc-4.6 gcc)
PROJECT_DEFAULT_CC := $(call env_preference, clang $(PROJECT_GCC_ENV))

PROJECT_RELEASES := $(addprefix release-,$(call env_which,clang $(PROJECT_GCC_ENV)))
PROJECT_CONFIGS := $(addsuffix -$(PROJECT_DEFAULT_CC),debug nightly) $(PROJECT_RELEASES)

$(call trash_configs, $(PROJECT_CONFIGS))
$(call hook_precommit_configs, debug-$(PROJECT_DEFAULT_CC) $(PROJECT_RELEASES))


# ------------------------------------------------------------ default configuration
ifneq ($(filter debug nightly,$(config)),)
    override config:=$(config)-$(PROJECT_DEFAULT_CC)
endif

ifeq ($(filter $(PROJECT_CONFIGS),$(config)),)
    override config:=$(firstword $(PROJECT_RELEASES))
endif


# ------------------------------------------------------------ gcc/clang configuration
override CC:=$(PROJECT_DEFAULT_CC)

ifeq ($(filter-out release-gcc%,$(config)),)
    override CC:=$(PROJECT_GCC_ENV)
endif

ifeq ($(filter-out release-clang%,$(config)),)
    override CC:=clang
endif

override CXX:=$(CC)

ifeq ($(CXX),clang)
	override CXX:=$(CXX)++
endif

ifeq ($(filter-out gcc%,$(CXX)),)
	override CXX:=$(subst gcc,g++,$(CXX))
	override CC:=$(CXX)
endif

override LD:=$(CXX)

# ------------------------------------------------------------ default parameters
PROJECT_CLFLAGS := -D__CS499R_OPENCL_PREPROCESSOR
PROJECT_CCFLAGS := -m64 -std=gnu11
PROJECT_CXXFLAGS := -Wall -Wextra -m64 -std=c++11
PROJECT_LDFLAGS := -std=c++11 -lpthread $(call bin_officiallib,opencl)

# ------------------------------------------------------------ release parameters
ifeq ($(filter-out release-%,$(config)),)
    PROJECT_CXXFLAGS += -O3 -Werror -mmmx
endif

# ------------------------------------------------------------ nightly parameters
ifeq ($(filter-out nightly-%,$(config)),)
    PROJECT_CXXFLAGS += -g
endif

# ------------------------------------------------------------ debug parameters
ifeq ($(filter-out debug-%,$(config)),)
    PROJECT_CXXFLAGS += -g -DYOTTA_DEBUG
endif


# ------------------------------------------------------------------------------ CS499R library's headers directory

LIB_HEADERS_PRODUCT := $(call product_create,BINHEADERS,headers)
LIB_HEADERS_TARGET := $(call product_target,$(LIB_HEADERS_PRODUCT))
$(call product_public,$(LIB_HEADERS_PRODUCT))

LIB_HEADERS := $(call bin_header_deps,./src/cs499r.hpp)

$(LIB_HEADERS_TARGET): $(LIB_HEADERS)
$(LIB_HEADERS_TARGET): CPFLAGS += $(LIB_HEADERS)
$(LIB_HEADERS_TARGET): CPROOTDIR = src/


# ------------------------------------------------------------------------------ CS499R library's binaries

LIB_BINARIES_PRODUCT := $(call product_create,BINLIBSTATIC,static_lib)
LIB_BINARIES_TARGET := $(call product_target,$(LIB_BINARIES_PRODUCT))
$(call product_public,$(LIB_BINARIES_PRODUCT))

# ------------------------------------------------------------ C/C++ source files
LIB_OBJECT_LOCAL_SRC := $(call filelist,./src/cs499r.flist)
LIB_OBJECT_LOCAL_BINARIES := $(call bin_object_files,$(LIB_OBJECT_LOCAL_SRC))

# ------------------------------------------------------------ OpenCL source files
LIB_OBJECT_LOCAL_OPENCL_SRC := $(call filelist,./src/cs499rProgram.flist)

LIB_OBJECT_LOCAL_OPENCL_I_SRC := $(foreach OPENCL_SRC,$(LIB_OBJECT_LOCAL_OPENCL_SRC), \
	$(eval $(BUILD_TMP_DIR)$(strip $(notdir $(OPENCL_SRC))).i: $(OPENCL_SRC)) \
	$(eval $(BUILD_TMP_DIR)$(strip $(notdir $(OPENCL_SRC))).i: _CL_SRC_FILE=$(OPENCL_SRC)) \
	$(BUILD_TMP_DIR)$(strip $(notdir $(OPENCL_SRC))).i \
)

LIB_OBJECT_LOCAL_OPENCL_CPP_SRC := $(patsubst %.i,%.cpp, $(LIB_OBJECT_LOCAL_OPENCL_I_SRC))

# rule for OpenCL preprocessing
$(LIB_OBJECT_LOCAL_OPENCL_I_SRC): $$(MK_DEPENDENCIES)
	$(call history_rule,preprocessing cl file,$(_CL_SRC_FILE))
	$(CMD_MKDIR_ALL) $(BUILD_TMP_DIR) $(BUILD_DEPS_DIR)
	$(CMD_PREFIX)$(CC) -E -x c -o $@ -MMD -MF $(patsubst %.i,%.d, $(BUILD_DEPS_DIR)$(notdir $@)) $(PROJECT_CLFLAGS) $(_CL_SRC_FILE)

# rule for OpenCL C++ generation
$(LIB_OBJECT_LOCAL_OPENCL_CPP_SRC): %.cpp: %.i scripts/cl_to_cpp.py
	$(call history_rule,generating c++ file,$@)
	$(CMD_PREFIX)python scripts/cl_to_cpp.py $@ $<

LIB_OBJECT_LOCAL_OPENCL_BINARIES := $(call bin_object_files,$(LIB_OBJECT_LOCAL_OPENCL_CPP_SRC))


# ------------------------------------------------------------ external source files
LIB_OBJECT_EXTERNAL_SRC := $(wildcard libs/*/*.c)
LIB_OBJECT_EXTERNAL_BINARIES := $(call bin_object_files,$(LIB_OBJECT_EXTERNAL_SRC))

# ------------------------------------------------------------ all binaries
LIB_OBJECT_BINARIES := $(LIB_OBJECT_LOCAL_BINARIES) $(LIB_OBJECT_LOCAL_OPENCL_BINARIES) $(LIB_OBJECT_EXTERNAL_BINARIES)

# ------------------------------------------------------------ compilation/link configuration
$(LIB_BINARIES_TARGET): $(LIB_OBJECT_BINARIES)
$(LIB_BINARIES_TARGET): CCFLAGS += PROJECT_CCFLAGS
$(LIB_BINARIES_TARGET): CXXFLAGS += $(PROJECT_CXXFLAGS)
$(LIB_BINARIES_TARGET): ARFLAGS += $(LIB_OBJECT_BINARIES)


# ------------------------------------------------------------------------------ CS499R executable binaries

EXEC_PRODUCT := $(call product_create,BINEXEC,raytracer)
EXEC_TARGET := $(call product_target,$(EXEC_PRODUCT))
EXEC_TEST := $(call test_product,$(EXEC_PRODUCT))
$(call product_public,$(EXEC_PRODUCT))

EXEC_OBJECT_BINARIES := $(call bin_object_files,$(call filelist,./src/app.flist))

# ------------------------------------------------------------ compilation/link configuration
$(EXEC_TARGET): $(EXEC_OBJECT_BINARIES) $(LIB_BINARIES_TARGET)
$(EXEC_TARGET): CXXFLAGS += $(PROJECT_CXXFLAGS)
$(EXEC_TARGET): LDFLAGS += $(PROJECT_LDFLAGS) $(EXEC_OBJECT_BINARIES) $(LIB_OBJECT_BINARIES)
