//===- PartialEval.cpp - Partial Evaluation Optimization --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This transformation implements the well known scalar replacement of
// aggregates transformation.  This xform breaks up alloca instructions of
// structure type into individual alloca instructions for
// each member, when legal.  Then, if legal, it transforms the individual
// alloca instructions into nice clean scalar SSA form.
//
// This combines a simple SRoA algorithm with Mem2Reg because they
// often interact, especially for C++ programs.  As such, iterating between
// SRoA and Mem2Reg until we run out of things to promote works well.
//
//===----------------------------------------------------------------------===//


#include "llvm/Analysis/AliasSetTracker.h"
#include "llvm/Analysis/AliasAnalysis.h"

#include "llvm/Transforms/Scalar.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/Loads.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GetElementPtrTypeIterator.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"
#include "llvm/Transforms/Utils/SSAUpdater.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <string>

using namespace llvm;
using namespace std;

#define DEBUG_TYPE "NumExpanded"
STATISTIC(NumExpanded,  "Number of aggregate allocas broken up");


namespace {
  
  struct LLPE : public ModulePass {
    static char ID;             // Pass identification
    
    LLPE() : ModulePass(ID) { }

    // Entry point for the partial evaluation pass
    bool runOnModule(Module &M);

    // getAnalysisUsage - This pass does not require any passes, but we know it
    // will not alter the CFG, so say so.
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<AssumptionCacheTracker>();
      AU.addRequired<DominatorTreeWrapperPass>();
      AU.setPreservesCFG();
    }

  private:
    // Add fields and helper functions for this pass here.
  };
}

//--------------------------------------------------------------------------//
// Register the pass so it is accessible from the command line,
// via the pass manager.
//--------------------------------------------------------------------------//
char LLPE::ID = 0;
static RegisterPass<LLPE> X("partialeval",
			    "Partial Evaluation of Programs", false, true);

//----------------------------------------------------------------------------//
// 
// Function runOnFunction:
// Entry point for the overall ScalarReplAggregates function pass.
// This function is provided to you.
// 
//----------------------------------------------------------------------------//

bool LLPE::runOnModule(Module &M) {
	
	//int input = 6;
  

	for (Module::iterator F = M.begin(), F_end = M.end(); F != F_end; ++F) {
		errs() << F->getName() << "\n";
		for (Function::arg_iterator A = F->arg_begin(), A_end = F->arg_end(); A != A_end; ++A) {
			errs() << A->getName() << "\n";
		
			if (A->getName() == "argv") {
				for (User::user_iterator U = A->user_begin(), U_end = A->user_end(); U != U_end; ++U) {
					Instruction *User = cast<Instruction>(*U);
					errs() << "Success" << "\n";
					//StoreInst *SI = new StoreInst(6);
				}
			}
			
		}
		errs() << "\n";
	}
   
	return true;
}

