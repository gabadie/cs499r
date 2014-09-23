
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

override LD:=$(CC)

# ------------------------------------------------------------ default parameters
PROJECT_CFLAGS := -Wall -Wextra -m64 -std=gnu99
PROJECT_LDFLAGS := -lpthread $(call bin_officiallib,opencl)

# ------------------------------------------------------------ release parameters
ifeq ($(filter-out release_%,$(config)),)
    PROJECT_CFLAGS += -O3 -Werror -mmmx -mavx
endif

# ------------------------------------------------------------ nightly parameters
ifeq ($(config),nightly)
    PROJECT_CFLAGS += -g
endif

# ------------------------------------------------------------ debug parameters
ifeq ($(config),debug)
    PROJECT_CFLAGS += -g -DYOTTA_DEBUG
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

LIB_OBJECT_BINARIES := $(call bin_object_files,$(call filelist,./src/cs499r.flist))

# ------------------------------------------------------------ compilation/link configuration
$(LIB_BINARIES_TARGET): $(LIB_OBJECT_BINARIES)
$(LIB_BINARIES_TARGET): CFLAGS += $(PROJECT_CFLAGS)
$(LIB_BINARIES_TARGET): ARFLAGS += $(LIB_OBJECT_BINARIES)


# ------------------------------------------------------------------------------ CS499R executable binaries

EXEC_PRODUCT := $(call product_create,BINEXEC,cs499r)
EXEC_TARGET := $(call product_target,$(EXEC_PRODUCT))
$(call product_public,$(EXEC_PRODUCT))

EXEC_OBJECT_BINARIES := $(call bin_object_files,$(call filelist,./src/cs499rMain.flist))

# ------------------------------------------------------------ compilation/link configuration
$(EXEC_TARGET): $(EXEC_OBJECT_BINARIES) $(LIB_BINARIES_TARGET)
$(EXEC_TARGET): CFLAGS += $(PROJECT_CFLAGS)
$(EXEC_TARGET): LDFLAGS += $(PROJECT_LDFLAGS) $(EXEC_OBJECT_BINARIES) $(LIB_BINARIES_TARGET)
