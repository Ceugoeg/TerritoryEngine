#pragma once
#include "Territory/Pipeline/Pass.hpp"
#include <vector>
#include <memory>

namespace Territory {

    // 顺序执行器
    class Pipeline {
    public:
        void AddPass(std::shared_ptr<Pass> pass) {
            m_Passes.push_back(pass);
        }

        // 引擎的主循环只需要无脑调用这个函数
        void ExecuteAll(float deltaTime) {
            for (auto& pass : m_Passes) {
                pass->Execute(deltaTime);
            }
        }

        void Clear() {
            m_Passes.clear();
        }

    private:
        std::vector<std::shared_ptr<Pass>> m_Passes;
    };

} // namespace Territory