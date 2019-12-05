//=============================================================================
// FILE:
//    HelloWorld.cpp
//
// DESCRIPTION:
//    Visits all functions in a module, prints their names and the number of
//    arguments via stderr. Strictly speaking, this is an analysis pass (i.e.
//    the functions are not modified). However, in order to keep things simple
//    there's no 'print' method here, which every analysis pass should
//    implement.
//
// USAGE:
//    1. Legacy pass manager
//      # Request `HelloWorld` via a dedicated flag:
//      opt -load libHelloWorld.dylib -legacy-hello-world -disable-output <input-llvm-file>
//      # `HelloWorld` will be executed as part of the optimisation pipelines
//      opt -load libHelloWorld.dylib -O{0|1|2|3} -disable-output <input-llvm-file>
//    2. New pass manager
//      # Define your pass pipeline via the '-passes' flag
//      opt -load-pass-plugin=libHelloWorld.dylib -passes="hello-world" -disable-output <input-llvm-file>
//
//
// License: MIT
//=============================================================================
#include "llvm/IR/Function.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;

//-----------------------------------------------------------------------------
// HelloWorld implementation
//-----------------------------------------------------------------------------
// No need to expose the internals of the pass to the outside world - keep
// everything in an anonymous namespace.
namespace {

// This method implements what the pass does
void visitor(Function &F) {
    errs() << "Visiting: ";
    errs() << F.getName() << " (takes ";
    errs() << F.arg_size() << " args)\n";
}

// New PM implementation
struct HelloWorld : PassInfoMixin<HelloWorld> {
  // Main entry point, takes IR unit to run the pass on (&F) and the
  // corresponding pass manager (to be queried if need be)
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
      visitor(F);
      // Initialize the worklist to all of the instructions ready to process...
      SmallPtrSet<Instruction *, 16> WorkList;
      // The SmallVector of WorkList ensures that we do iteration at stable order.
      // We use two containers rather than one SetVector, since remove is
      // linear-time, and we don't care enough to remove from Vec.
      SmallVector<Instruction *, 16> WorkListVec;
      for (Instruction &I : instructions(&F)) {
//          errs() << I << "\n";

          WorkList.insert(&I);
          WorkListVec.push_back(&I);
      }

      while (!WorkList.empty()) {
          for (auto *I : WorkListVec) {
              errs() << "Instruction I " << *I << "\n";
              /*
              for (const Use &OpU : I->operands()) {
                  // Fold the Instruction's operands.
                  errs() << *OpU << "\n";
              }
               */

//          for(auto op= I->op_begin(); op != I->op_end(); op++){
//              Value* v = op->get();
//              errs() << v->getName() << "\n";
//          }
            if (auto *PN = dyn_cast<PHINode>(I)) {
                errs() << "phi node" << "\n";
//                for (Value *Incoming : PN->incoming_values()) {
//                    // If the incoming value is undef then skip it.  Note that while we could
//                    // skip the value if it is equal to the phi node itself we choose not to
//                    // because that would break the rule that constant folding only applies if
//                    // all operands are constants.
//                    errs() << *Incoming << "\n";
//                }
            }


              WorkList.erase(I); // Remove element from the worklist...

              // Add all of the users of this instruction to the worklist, they might
              // be constant propagatable now...
//              for (User *U : I->users()) {
//                  errs() << "all uses" << "\n";
//                  errs() << *U << "\n";
//                  // If user not in the set, then add it to the vector.
//              }



          }
      }
    return PreservedAnalyses::all();
  }
};


//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
llvm::PassPluginLibraryInfo getHelloWorldPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "HelloWorld", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "hello-world") {
                    FPM.addPass(HelloWorld());
                    return true;
                  }
                  return false;
                });
          }};
}

// This is the core interface for pass plugins - with this 'opt' will be able
// to recognize HelloWorld when added to the pass pipeline on the command line,
// i.e. via '-passes=hello-world'
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getHelloWorldPluginInfo();
}
}


