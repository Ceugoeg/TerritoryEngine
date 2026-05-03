#pragma once

#include <vector>
#include <mutex>
#include <utility>

namespace Territory {

/**
 * @brief Double-Buffered Intent Queue
 * 采用指针置换技术，将多线程锁竞争开销降至极限的 O(1) 级别。
 * 适用于跨线程的海量指令分发，如粒子生成、网络同步状态合并等。
 */
template<typename T>
class IntentQueue {
public:
    IntentQueue() {
        // 提前预分配内存，防止在极速 Push 时的动态扩容导致系统调用分配内存的开销
        m_WriteBuffer.reserve(10000);
        m_ReadBuffer.reserve(10000);
    }

    ~IntentQueue() = default;

    IntentQueue(const IntentQueue&) = delete;
    IntentQueue& operator=(const IntentQueue&) = delete;

    /**
     * @brief 生产者调用：工作线程将意图压入队列
     * 使用右值引用和完美转发，避免大对象的拷贝开销
     */
    template<typename U>
    void Push(U&& intent) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_WriteBuffer.push_back(std::forward<U>(intent));
    }

    /**
     * @brief 消费者调用（仅限主线程）：瞬间置换读写缓冲区
     * 极度核心：swap() 在 STL 中仅交换底层指针，耗时极短。
     * 置换后立刻清空之前的写缓冲区以供下一帧重复使用。
     */
    void SwapBuffers() {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_ReadBuffer.swap(m_WriteBuffer);
        m_WriteBuffer.clear();
    }

    /**
     * @brief 消费者调用（仅限主线程）：获取当前帧需要处理的所有指令
     * @return 只读的指令集合，此时访问绝对线程安全，完全无锁
     */
    const std::vector<T>& GetReadBuffer() const {
        return m_ReadBuffer;
    }

private:
    std::vector<T> m_WriteBuffer;
    std::vector<T> m_ReadBuffer;
    std::mutex m_Mutex;
};

} // namespace Territory