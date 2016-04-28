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
#include "llvm/Analysis/Loads.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Support/CallSite.h"
#include "llvm/IR/Constants.h"
#include "llvm/DIBuilder.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/DebugInfo.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/Analysis/Dominators.h"
#include "llvm/IR/Function.h"
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

using namespace llvm;
using namespace std;


namespace {
  
  struct LLPE : public ModulePass {
    static char ID;             // Pass identification
    
    LLPE() : ModulePass(ID) { }

    // Entry point for the partial evaluation pass
    bool runOnModule(Module &M);

    // getAnalysisUsage - This pass does not require any passes, but we know it
    // will not alter the CFG, so say so.
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();
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
static RegisterPass<LLPE> X("partialeval",
			    "Partial Evaluation of Programs", false, true);

//----------------------------------------------------------------------------//
// 
// Function runOnModule:
// Entry point for the overall LLPE module pass.
// 
//----------------------------------------------------------------------------//

bool LLPE::runOnModule(Module &M) {
  
	vector<int> argv = readInputFile();

	for (Module::iterator F = M.begin(), F_end = M.end(); F != F_end; ++F) {

		for (Function::arg_iterator A = F->arg_begin(), A_end = F->arg_end(); A != A_end; ++A) {
		
			//Search for variables referencing argv
			if (A->getName() == "argv") {
				
				//Iterate through uses of argv
				for (Value::use_iterator U = A->use_begin(), U_end = A->use_end(); U != U_end; ++U) {
					Instruction *User = dyn_cast<Instruction>(*U);
					
					StoreInst *SI = dyn_cast<StoreInst>(User);
					AllocaInst *OrigAlloca = dyn_cast<AllocaInst>(SI->getOperand(1));
					
					for (Value::use_iterator U2 = OrigAlloca->use_begin(), U2_end = OrigAlloca->use_end(); U2 != U2_end; ++U2) {
						Instruction *User2 = dyn_cast<Instruction>(*U2);

						for (Value::use_iterator U3 = User2->use_begin(), U3_end = OrigAlloca->use_end(); U3 != U3_end; ++U3) {
							searchForStoreInstruction(dyn_cast<Instruction>(*U3)->getParent(), argv);
						}
					}

				}
			}
			
		}

	}
   
	return true;
}

bool LLPE::searchForStoreInstruction(BasicBlock *BB, vector<int> argv) {
	bool GEPIFlag = false;
	
	//Iterate through each instruction in the basic block
	//Search for GEPI. The next store after a GEPI, indicates the location for inserting a new alloca instruction.
	for(BasicBlock::iterator BI = BB->begin(), BI_end = BB->end(); BI != BI_end; ++BI) {
		if ( isa<GetElementPtrInst>(BI) ) {
			GEPIFlag = true;
		}
		if (GEPIFlag) {
			if ( StoreInst *SI = dyn_cast<StoreInst>(BI)) {
				AllocaInst *constAlloca = dyn_cast<AllocaInst>(SI->getOperand(1));
				constAlloca->print(errs());
				StoreInst *constSI = new StoreInst(ConstantInt::get(Type::getInt32Ty(SI->getContext()), argv.at(0)), constAlloca, SI);
				SI->eraseFromParent();
				return true;
			}
		}
	} //End BasicBlock loop
	
	return false;
}

//Read the program input from an input text file
vector<int> LLPE::readInputFile() {
	ifstream infile("input.txt", ios::in);
	vector<int> argv;
	
	int argv_elem;
	while (infile >> argv_elem) {
		argv.push_back(argv_elem);
	}
	return argv;
}
