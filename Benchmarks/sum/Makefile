## change or use make TARGET=inputfilename
TARGET=sum

## replace LLVMROOT and CLANG as appropriate
LLVMROOT = $(HOME)/llvm-3.7.1-build
CLANG = $(HOME)/llvm-3.7.1-build/tools/clang/bin/clang

LLVMAS  = $(LLVMROOT)/bin/llvm-as
LLVMDIS = $(LLVMROOT)/bin/llvm-dis
LLVMOPT = $(LLVMROOT)/bin/opt

## change library name appropriately for your system
DYLIB   = $(LLVMROOT)/lib/libAssign2.so

## Other choices: test or comparecfe (these will be provided later)
default: debug
DEBUGOPTS =  #-verify -inline -mem2reg -simplifycfg -globalopt -globaldce -adce -simplifycfg
BUILDOPTS = -mem2reg -inline -instcombine -globaldce -adce

debug:  clean
	$(MAKE) $(TARGET).debug.ll


old: $(TARGET).old

new: $(TARGET).new

INPUTFILE = input.txt


.PRECIOUS: %.ll


%.ll: %.bc
	$(LLVMDIS) -f $<


%.llvm.bc: %.c
	$(CLANG) -S -emit-llvm -o - $< | $(LLVMAS) > $@

# change the FirstName to your first name
%.debug.bc: %.llvm.bc
	$(LLVMOPT) $(DEBUGOPTS) < $< | \
	$(LLVMOPT) -load $(DYLIB) -stats -partialeval -inputfile $(INPUTFILE) $(BUILDOPTS) > $@

%.old: %.c
	$(CLANG) -o $@ $<

%.new: %.debug.bc
	$(CLANG) -o $@ $<

clean:
	$(RM) -f *.debug.bc *.llvm.bc *.old *.new $(TARGET).debug.ll
