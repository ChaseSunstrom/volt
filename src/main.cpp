#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"

int main()
{
    llvm::LLVMContext context;
    auto              module = std::make_unique<llvm::Module>("test", context);
    module->print(llvm::outs(), nullptr);
    return 0;
}
