# Emscripten
# https://emscripten.org/docs/getting_started/downloads.html

CC=emcc
SRCDIR=../..
OBJDIR=obj
EXE=sstest
SRCS := $(SRCDIR)/SSTest/SSTest.cpp $(wildcard $(SRCDIR)/SSCode/*.cpp) $(wildcard $(SRCDIR)/SSCode/**/*.cpp)
OBJS := $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))

# Compiler flags
CFLAGS=-O2 \
-Wno-unused-result \
-I$(SRCDIR)/SSCode \
-I$(SRCDIR)/SSCode/VSOP2013

# Linker flags
# -sALLOW_MEMORY_GROWTH=1         Alow Emscripten's heap memory to grow.
# -sWASM=2                        Generate both wasm and js.
# -sEXIT_RUNTIME=0                Workaround to prevent emrun from prematurely quitting. Also see "timeout" below.
# --preload-file ../../SSData/@   Loads the contents of SSData into memory.
# --emrun                         Allow command-line arguments to be passed to main.
# -pthread                        Use threads (flag must also be present in CFLAGS).
#                                 Note, using both pthreads and ALLOW_MEMORY_GROWTH may be slow - https://github.com/WebAssembly/design/issues/1271
LDFLAGS= \
-sALLOW_MEMORY_GROWTH=1 \
-sWASM=2 \
-sEXIT_RUNTIME=0 \
--emrun \
--preload-file ../../SSData/@

all: test

run: test
	emrun --serve_after_close --timeout 10 $(EXE).html / .

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -std=c99 -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -std=c++11 -c $< -o $@

test: $(OBJS)
	$(CC) -o $(EXE).html $(CFLAGS) $(OBJS) $(LDFLAGS)

clean:
	rm -Rf $(OBJDIR)
	rm -f \
	$(EXE).data \
	$(EXE).html \
	$(EXE).html.mem \
	$(EXE).js \
	$(EXE).wasm \
	$(EXE).wasm.js \
	$(EXE).worker.js
