//===- inlineTransformation.cpp - Partial Evaluation Optimization --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// THis transformation inherit inliner.cpp from LLVM pass and override the getInlineCost function to determine the what functions we want to turn inline into and then call the Inliner::shouldInline to do the inline transformation
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
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"
#include "llvm/Transforms/Utils/SSAUpdater.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <string>
#include <fstream>
#include <AssumptionCache.h>
#include <clone.h>
#include <DenseMap.h>
#include <Pass.h>


using namespace llvm;
using namespace std;

#define DEBUG_TYPE "NumExpanded"
STATISTIC(NumExpanded,  "Number of aggregate allocas broken up");

static cl::opt<string> InputFilename("inputfile", cl::desc("Specify input filename"), cl::value_desc("filename"));


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
    bool searchForStoreInstruction(BasicBlock *BB, vector<int> argv);
    vector<int> readInputFile();
  };
}

//--------------------------------------------------------------------------//
// Register the pass so it is accessible from the command line,
// via the pass manager.
//--------------------------------------------------------------------------//
char LLPE::ID = 0;
static RegisterPass<LLPE> X("partialevalForInlineTransformation",
			    "Partial Evaluation of Programs for inline transformation", false, true);

//----------------------------------------------------------------------------//
// 
// Function runOnModule:
// Entry point for the inline  transformation PE.
// 
//----------------------------------------------------------------------------//

bool LLPE::runOnModule(Module &M) {
  
	vector<int> argv = readInputFile();

        llvm::CallGraph currentGraph(&M);
	

	for (Module::iterator F = M.begin(), F_end = M.end(); F != F_end; ++F) {

		for (Function::arg_iterator A = F->arg_begin(), A_end = F->arg_end(); A != A_end; ++A) {
		
			if (A->getName() == "argv") {
				
				typedef DenseMap<ArrayType*, std::vector<AllocaInst*> > InlinedArrayAllocasTy; 
				
				int InlineHistory = 0;

				bool InsertLifetime = false;		
		
				char PID = '0';
				
				llvm::PassKind passKind = llvm::PassKind.PT_Function;				

				llvm::Pass::Pass newPass(passKind, &PID);				

				llvm::AssumptionCacheTracker assumptionCacheTracker;
							
				Instruction *instructionCallSite  = dyn_cast<Function>(*A);				

				llvm::CallSite::Callsite currentCallSite(instructionCallSite);

				llvm::InlineFunctionInfo inlinefunctionInfo(currentGraph, assumptionCacheTracker);

				inliner::InlineCallIfPossible(newPass, currentCallSite, inlinefunctionInfo, InlinedArrayAllocasTy, InlineHistory, InsertLifetime);	
			

			}
			
		}

	}
   
}

vector<int> LLPE::readInputFile() {
	ifstream infile(InputFilename);
	vector<int> argv;
	
	int argv_elem;
	while (infile >> argv_elem) {
		argv.push_back(argv_elem);
	}
	return argv;
}
