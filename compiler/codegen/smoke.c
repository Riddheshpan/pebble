#include <stdio.h>
#include <stdlib.h>

#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>

int main(void) {
    LLVMModuleRef module =
        LLVMModuleCreateWithName("pebble_smoke");

    LLVMTypeRef i32 = LLVMInt32Type();

    LLVMTypeRef main_type =
        LLVMFunctionType(i32, NULL, 0, 0);

    LLVMValueRef main_function =
        LLVMAddFunction(module, "main", main_type);

    LLVMBasicBlockRef entry =
        LLVMAppendBasicBlock(main_function, "entry");

    LLVMBuilderRef builder =
        LLVMCreateBuilder();

    LLVMPositionBuilderAtEnd(builder, entry);

    LLVMBuildRet(builder, LLVMConstInt(i32, 42, 0));

    /*
     * Initialize LLVM's native target support.
     */
    if (LLVMInitializeNativeTarget() != 0) {
        fprintf(stderr, "error: failed to initialize native target\n");
        return 1;
    }

    if (LLVMInitializeNativeAsmPrinter() != 0) {
        fprintf(stderr, "error: failed to initialize native assembler printer\n");
        return 1;
    }

    /*
     * Determine the host target.
     */
    char *triple = LLVMGetDefaultTargetTriple();

    LLVMTargetRef target = NULL;
    char *error = NULL;

    if (LLVMGetTargetFromTriple(triple, &target, &error) != 0) {
        fprintf(stderr, "error: %s\n", error);
        LLVMDisposeMessage(error);
        LLVMDisposeMessage(triple);
        LLVMDisposeBuilder(builder);
        LLVMDisposeModule(module);
        return 1;
    }

    /*
     * Create a target machine capable of producing native code.
     */
    LLVMTargetMachineRef target_machine =
        LLVMCreateTargetMachine(
            target,
            triple,
            "generic",
            "",
            LLVMCodeGenLevelDefault,
            LLVMRelocDefault,
            LLVMCodeModelDefault
        );

    if (target_machine == NULL) {
        fprintf(stderr, "error: failed to create target machine\n");
        LLVMDisposeMessage(triple);
        LLVMDisposeBuilder(builder);
        LLVMDisposeModule(module);
        return 1;
    }

    /*
     * Emit LLVM IR → native object file.
     */
    if (LLVMTargetMachineEmitToFile(
            target_machine,
            module,
            "pebble-smoke.o",
            LLVMObjectFile,
            &error
        ) != 0) {

        fprintf(stderr, "error: %s\n", error);
        LLVMDisposeMessage(error);
        LLVMDisposeTargetMachine(target_machine);
        LLVMDisposeMessage(triple);
        LLVMDisposeBuilder(builder);
        LLVMDisposeModule(module);
        return 1;
    }

    printf("Generated pebble-smoke.o\n");
    printf("Target: %s\n", triple);

    LLVMDisposeTargetMachine(target_machine);
    LLVMDisposeMessage(triple);
    LLVMDisposeBuilder(builder);
    LLVMDisposeModule(module);

    return 0;
}