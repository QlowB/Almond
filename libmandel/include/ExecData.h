#ifndef MANDEL_EXECDATA_H
#define MANDEL_EXECDATA_H

#include <asmjit/asmjit.h>
#include <memory>

namespace mnd
{
    struct ExecData
    {
        std::unique_ptr<asmjit::JitRuntime> jitRuntime;
        std::unique_ptr<asmjit::CodeHolder> code;
        std::unique_ptr<asmjit::x86::Compiler> compiler;
        void* iterationFunc;

        ExecData(void) :
            jitRuntime{ std::make_unique<asmjit::JitRuntime>() },
            code{ std::make_unique<asmjit::CodeHolder>() },
            compiler{ nullptr },
            iterationFunc{ nullptr }
        {
            code->init(jitRuntime->codeInfo());
            compiler = std::make_unique<asmjit::x86::Compiler>(code.get());
        }

        ExecData(ExecData&&) = default;
        ExecData(const ExecData&) = delete;
        ExecData& operator=(ExecData&&) = default;
        ExecData& operator=(const ExecData&) = delete;

        ~ExecData(void) = default;
    };
}

#endif // MANDEL_EXECDATA_H

