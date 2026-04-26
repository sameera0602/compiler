#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/TargetMachine.h>

#include "../include/codegen.h"

static LLVMModuleRef module;
static LLVMBuilderRef builder;
static LLVMContextRef context;

// Simple environment for variables during codegen
typedef struct EnvNode {
    char* name;
    LLVMValueRef val;
    struct EnvNode* next;
} EnvNode;

static EnvNode* env = NULL;

static void env_push(const char* name, LLVMValueRef val) {
    EnvNode* node = malloc(sizeof(EnvNode));
    node->name = strdup(name);
    node->val = val;
    node->next = env;
    env = node;
}

static LLVMValueRef env_lookup(const char* name) {
    EnvNode* current = env;
    while (current) {
        if (strcmp(current->name, name) == 0) return current->val;
        current = current->next;
    }
    return NULL;
}

static void env_clear() {
    EnvNode* current = env;
    while (current) {
        EnvNode* next = current->next;
        free(current->name);
        free(current);
        current = next;
    }
    env = NULL;
}

void codegen_init() {
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();
    LLVMInitializeNativeAsmParser();

    context = LLVMContextCreate();
    module = LLVMModuleCreateWithNameInContext("nova_module", context);
    builder = LLVMCreateBuilderInContext(context);
    
    // Declare printf function for our "print" statement
    LLVMTypeRef printf_args[] = { LLVMPointerType(LLVMInt8TypeInContext(context), 0) };
    LLVMTypeRef printf_type = LLVMFunctionType(LLVMInt32TypeInContext(context), printf_args, 1, 1);
    LLVMAddFunction(module, "printf", printf_type);
}

// Forward declarations
static LLVMValueRef codegen_expr(ASTNode* node);
static void codegen_stmt(ASTNode* node);

static LLVMValueRef codegen_expr(ASTNode* node) {
    if (!node) return NULL;
    
    switch (node->type) {
        case AST_NUMBER:
            return LLVMConstInt(LLVMInt32TypeInContext(context), node->data.number.value, 0);
        case AST_IDENTIFIER: {
            LLVMValueRef ptr = env_lookup(node->data.identifier.name);
            if (!ptr) {
                fprintf(stderr, "Codegen Error: Unknown variable name %s\n", node->data.identifier.name);
                return NULL;
            }
            return LLVMBuildLoad2(builder, LLVMInt32TypeInContext(context), ptr, node->data.identifier.name);
        }
        case AST_BINOP: {
            LLVMValueRef L = codegen_expr(node->data.binop.left);
            LLVMValueRef R = codegen_expr(node->data.binop.right);
            if (!L || !R) return NULL;
            
            switch (node->data.binop.op) {
                case OP_ADD: return LLVMBuildAdd(builder, L, R, "addtmp");
                case OP_SUB: return LLVMBuildSub(builder, L, R, "subtmp");
                case OP_MUL: return LLVMBuildMul(builder, L, R, "multmp");
                case OP_DIV: return LLVMBuildSDiv(builder, L, R, "divtmp");
                case OP_EQ: return LLVMBuildICmp(builder, LLVMIntEQ, L, R, "eqtmp");
                case OP_NEQ: return LLVMBuildICmp(builder, LLVMIntNE, L, R, "neqtmp");
                case OP_LT: return LLVMBuildICmp(builder, LLVMIntSLT, L, R, "lttmp");
                case OP_GT: return LLVMBuildICmp(builder, LLVMIntSGT, L, R, "gttmp");
                case OP_LTE: return LLVMBuildICmp(builder, LLVMIntSLE, L, R, "ltetmp");
                case OP_GTE: return LLVMBuildICmp(builder, LLVMIntSGE, L, R, "gtetmp");
                default: return NULL;
            }
        }
        case AST_CALL: {
            LLVMValueRef callee = LLVMGetNamedFunction(module, node->data.call.callee);
            if (!callee) {
                fprintf(stderr, "Codegen Error: Unknown function referenced\n");
                return NULL;
            }
            
            LLVMValueRef* args = malloc(sizeof(LLVMValueRef) * node->data.call.arg_count);
            for (int i = 0; i < node->data.call.arg_count; ++i) {
                args[i] = codegen_expr(node->data.call.args[i]);
            }
            
            LLVMValueRef call = LLVMBuildCall2(builder, LLVMGlobalGetValueType(callee), callee, args, node->data.call.arg_count, "calltmp");
            free(args);
            return call;
        }
        default:
            return NULL;
    }
}

static void codegen_stmt(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case AST_LET: {
            LLVMValueRef init_val = codegen_expr(node->data.let_stmt.initializer);
            if (!init_val) return;
            
            // Allocate space on stack
            LLVMValueRef alloca = LLVMBuildAlloca(builder, LLVMInt32TypeInContext(context), node->data.let_stmt.name);
            LLVMBuildStore(builder, init_val, alloca);
            
            // Add to env
            env_push(node->data.let_stmt.name, alloca);
            break;
        }
        case AST_RETURN: {
            LLVMValueRef ret_val = codegen_expr(node->data.return_stmt.value);
            LLVMBuildRet(builder, ret_val);
            break;
        }
        case AST_EXPR_STMT:
            codegen_expr(node->data.expr_stmt.expr);
            break;
        case AST_PRINT: {
            LLVMValueRef expr_val = codegen_expr(node->data.print_stmt.expr);
            LLVMValueRef printf_func = LLVMGetNamedFunction(module, "printf");
            
            LLVMValueRef format_str = LLVMBuildGlobalStringPtr(builder, "%d\n", "fmt");
            LLVMValueRef args[] = { format_str, expr_val };
            
            LLVMTypeRef printf_args[] = { LLVMPointerType(LLVMInt8TypeInContext(context), 0) };
            LLVMTypeRef printf_type = LLVMFunctionType(LLVMInt32TypeInContext(context), printf_args, 1, 1);
            
            LLVMBuildCall2(builder, printf_type, printf_func, args, 2, "printf_call");
            break;
        }
        case AST_IF: {
            LLVMValueRef cond = codegen_expr(node->data.if_stmt.condition);
            
            LLVMValueRef zero = LLVMConstInt(LLVMInt32TypeInContext(context), 0, 0);
            // If condition is already i1 (from a comparison), we might not need this, but for ints we do.
            // Let's assume comparisons return i1, but variables might be i32.
            // To be safe, if cond type is i32, cmp with 0.
            if (LLVMGetTypeKind(LLVMTypeOf(cond)) == LLVMIntegerTypeKind && LLVMGetIntTypeWidth(LLVMTypeOf(cond)) != 1) {
                cond = LLVMBuildICmp(builder, LLVMIntNE, cond, zero, "ifcond");
            }

            LLVMValueRef function = LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder));
            LLVMBasicBlockRef then_bb = LLVMAppendBasicBlockInContext(context, function, "then");
            LLVMBasicBlockRef else_bb = LLVMAppendBasicBlockInContext(context, function, "else");
            LLVMBasicBlockRef merge_bb = LLVMAppendBasicBlockInContext(context, function, "ifcont");

            LLVMBuildCondBr(builder, cond, then_bb, node->data.if_stmt.else_branch ? else_bb : merge_bb);

            // Then block
            LLVMPositionBuilderAtEnd(builder, then_bb);
            codegen_stmt(node->data.if_stmt.then_branch);
            // Don't branch if block already has a terminator (like return)
            if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder))) {
                LLVMBuildBr(builder, merge_bb);
            }

            // Else block
            LLVMPositionBuilderAtEnd(builder, else_bb);
            if (node->data.if_stmt.else_branch) {
                codegen_stmt(node->data.if_stmt.else_branch);
            }
            if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder))) {
                LLVMBuildBr(builder, merge_bb);
            }

            LLVMPositionBuilderAtEnd(builder, merge_bb);
            break;
        }
        case AST_WHILE: {
            LLVMValueRef function = LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder));
            LLVMBasicBlockRef cond_bb = LLVMAppendBasicBlockInContext(context, function, "cond");
            LLVMBasicBlockRef loop_bb = LLVMAppendBasicBlockInContext(context, function, "loop");
            LLVMBasicBlockRef after_bb = LLVMAppendBasicBlockInContext(context, function, "afterloop");
            
            LLVMBuildBr(builder, cond_bb);
            LLVMPositionBuilderAtEnd(builder, cond_bb);
            
            LLVMValueRef cond = codegen_expr(node->data.while_stmt.condition);
            if (LLVMGetTypeKind(LLVMTypeOf(cond)) == LLVMIntegerTypeKind && LLVMGetIntTypeWidth(LLVMTypeOf(cond)) != 1) {
                LLVMValueRef zero = LLVMConstInt(LLVMInt32TypeInContext(context), 0, 0);
                cond = LLVMBuildICmp(builder, LLVMIntNE, cond, zero, "whilecond");
            }
            
            LLVMBuildCondBr(builder, cond, loop_bb, after_bb);
            
            LLVMPositionBuilderAtEnd(builder, loop_bb);
            codegen_stmt(node->data.while_stmt.body);
            if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder))) {
                LLVMBuildBr(builder, cond_bb);
            }
            
            LLVMPositionBuilderAtEnd(builder, after_bb);
            break;
        }
        case AST_BLOCK: {
            // New scope, technically we should save env here, but let's keep it simple
            for (int i = 0; i < node->data.block.stmt_count; i++) {
                codegen_stmt(node->data.block.statements[i]);
            }
            break;
        }
        default:
            break;
    }
}

void codegen_program(ASTNode* program) {
    if (!program || program->type != AST_PROGRAM) return;

    // First pass: declare all functions
    for (int i = 0; i < program->data.program.func_count; i++) {
        ASTNode* func = program->data.program.functions[i];
        
        LLVMTypeRef* param_types = malloc(sizeof(LLVMTypeRef) * func->data.function.param_count);
        for (int p = 0; p < func->data.function.param_count; p++) {
            param_types[p] = LLVMInt32TypeInContext(context); // only supporting int
        }
        
        LLVMTypeRef ret_type = func->data.function.return_type == TYPE_INT ? 
                               LLVMInt32TypeInContext(context) : LLVMVoidTypeInContext(context);
                               
        LLVMTypeRef func_type = LLVMFunctionType(ret_type, param_types, func->data.function.param_count, 0);
        LLVMAddFunction(module, func->data.function.name, func_type);
        free(param_types);
    }

    // Second pass: define functions
    for (int i = 0; i < program->data.program.func_count; i++) {
        env_clear();
        ASTNode* func = program->data.program.functions[i];
        
        LLVMValueRef llvm_func = LLVMGetNamedFunction(module, func->data.function.name);
        LLVMBasicBlockRef bb = LLVMAppendBasicBlockInContext(context, llvm_func, "entry");
        LLVMPositionBuilderAtEnd(builder, bb);
        
        // Setup arguments in environment
        for (int p = 0; p < func->data.function.param_count; p++) {
            LLVMValueRef arg = LLVMGetParam(llvm_func, p);
            const char* arg_name = func->data.function.params[p]->data.param.name;
            LLVMSetValueName2(arg, arg_name, strlen(arg_name));
            
            // Allocate space for argument
            LLVMValueRef alloca = LLVMBuildAlloca(builder, LLVMInt32TypeInContext(context), arg_name);
            LLVMBuildStore(builder, arg, alloca);
            env_push(arg_name, alloca);
        }
        
        // Generate body
        codegen_stmt(func->data.function.body);
        
        // Add default return if no return was hit
        if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(builder))) {
            if (func->data.function.return_type == TYPE_VOID) {
                LLVMBuildRetVoid(builder);
            } else {
                // To satisfy LLVM IR structural rules, provide a default return 0 for ints
                LLVMBuildRet(builder, LLVMConstInt(LLVMInt32TypeInContext(context), 0, 0));
            }
        }
        
        // Verify function
        LLVMVerifyFunction(llvm_func, LLVMPrintMessageAction);
    }
}

void codegen_dump_ir() {
    LLVMDumpModule(module);
}

void codegen_emit_object(const char* filename) {
    LLVMInitializeAllTargetInfos();
    LLVMInitializeAllTargets();
    LLVMInitializeAllTargetMCs();
    LLVMInitializeAllAsmParsers();
    LLVMInitializeAllAsmPrinters();

    char* target_triple = LLVMGetDefaultTargetTriple();
    LLVMSetTarget(module, target_triple);

    char* error = NULL;
    LLVMTargetRef target;
    if (LLVMGetTargetFromTriple(target_triple, &target, &error)) {
        fprintf(stderr, "Error getting target: %s\n", error);
        LLVMDisposeMessage(error);
        return;
    }

    LLVMTargetMachineRef machine = LLVMCreateTargetMachine(
        target, target_triple, "generic", "",
        LLVMCodeGenLevelDefault, LLVMRelocDefault, LLVMCodeModelDefault
    );

    LLVMSetModuleDataLayout(module, LLVMCreateTargetDataLayout(machine));

    if (LLVMTargetMachineEmitToFile(machine, module, (char*)filename, LLVMObjectFile, &error)) {
        fprintf(stderr, "Error emitting object file: %s\n", error);
        LLVMDisposeMessage(error);
    } else {
        printf("Object file written to %s\n", filename);
    }

    LLVMDisposeMessage(target_triple);
    LLVMDisposeTargetMachine(machine);
}

void codegen_cleanup() {
    LLVMDisposeBuilder(builder);
    LLVMDisposeModule(module);
    LLVMContextDispose(context);
    env_clear();
}
