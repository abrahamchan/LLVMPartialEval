//===- SelectiveInline.cpp - Code to perform simple function inlining --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements bottom-up inlining of functions into callees.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/IPO.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/InlineCost.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/InlinerPass.h"

using namespace llvm;

#define DEBUG_TYPE "selectiveinline"

namespace {

/// \brief Actual inliner pass implementation.
///
/// The common implementation of the inlining logic is shared between this
/// inliner pass and the always inliner pass. The two passes use different cost
/// analyses to determine when to inline.
class SelectiveInline : public Inliner {
  InlineCostAnalysis *ICA;

public:
  SelectiveInline() : Inliner(ID), ICA(nullptr) {
  }

  SelectiveInline(int Threshold)
      : Inliner(ID, Threshold, /*InsertLifetime*/ true), ICA(nullptr) {
  }

  static char ID; // Pass identification, replacement for typeid
  
  InlineCost getInlineCost(CallSite CS) override {
    if ( allArgsConst(CS) ) {
		return InlineCost::getAlways();
	}
	
	return InlineCost::getNever();
  }

  bool runOnSCC(CallGraphSCC &SCC) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  
  bool allArgsConst(CallSite &CS);
};

static int computeThresholdFromOptLevels(unsigned OptLevel,
                                         unsigned SizeOptLevel) {
  if (OptLevel > 2)
    return 275;
  if (SizeOptLevel == 1) // -Os
    return 75;
  if (SizeOptLevel == 2) // -Oz
    return 25;
  return 225;
}

} // end anonymous namespace


char SelectiveInline::ID = 0;
static RegisterPass<SelectiveInline> X("selectiveinline",
			    "Selectively Inling Functions for Partial Program Evaluations", false, true);

Pass *llvm::createFunctionInliningPass() { return new SelectiveInline(); }

Pass *llvm::createFunctionInliningPass(int Threshold) {
  return new SelectiveInline(Threshold);
}

Pass *llvm::createFunctionInliningPass(unsigned OptLevel,
                                       unsigned SizeOptLevel) {
  return new SelectiveInline(
      computeThresholdFromOptLevels(OptLevel, SizeOptLevel));
}

bool SelectiveInline::runOnSCC(CallGraphSCC &SCC) {
  ICA = &getAnalysis<InlineCostAnalysis>();
  return Inliner::runOnSCC(SCC);
}

void SelectiveInline::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<InlineCostAnalysis>();
  Inliner::getAnalysisUsage(AU);
}

bool SelectiveInline::allArgsConst(CallSite &CS) {
	for (CallSite::arg_iterator AI = CS.arg_begin(), CS_end = CS.arg_end(); AI != CS_end; ++AI) {
		if ( isa<Instruction>(AI)) {
			return false;
		}
	}
	return true;
}
