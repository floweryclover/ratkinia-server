//
// Created by floweryclover on 2025-07-10.
//

#ifndef RATKINIASERVER_PARALLELEXECUTOR_H
#define RATKINIASERVER_PARALLELEXECUTOR_H

#include <functional>
#include <span>
#include <optional>
#include <thread>

namespace Floweryclover
{
    enum class ExecutionPolicy : uint8_t
    {
        TotallyImmutable,
        MutableAxis,
    };

    template<typename TAxis, ExecutionPolicy Policy>
    using TExecutionAxis = std::conditional_t<Policy == ExecutionPolicy::TotallyImmutable, const TAxis&, TAxis&>;

    template<typename TAxis, ExecutionPolicy Policy>
    using TExecutionAxisPointer = std::conditional_t<Policy == ExecutionPolicy::TotallyImmutable, const TAxis*, TAxis*>;

    template<typename TTask, typename TExecutionResult>
    concept IsParallelForWorkerThreadsTask = requires(TTask task)
    {
        { std::invoke(task) } -> std::same_as<std::optional<TExecutionResult>>;
    };

    template<typename TTask, typename TExecutionResult, typename TAxis, ExecutionPolicy Policy>
    concept IsParallelForTask = requires(TTask task, TExecutionAxis<TAxis, Policy> axis)
    {
        { std::invoke(task, axis) } -> std::same_as<std::optional<TExecutionResult>>;
    };

    template<typename TGetAxisFn, typename TAxis, ExecutionPolicy Policy>
    concept IsGetAxisFn = requires(TGetAxisFn getAxisFn, uint32_t index)
    {
        { std::invoke(getAxisFn, index) } -> std::same_as<TExecutionAxisPointer<TAxis, Policy>>;
    } || requires(TGetAxisFn getAxisFn, uint32_t index)
    {
        { std::invoke(getAxisFn, index) } -> std::same_as<std::optional<TAxis>>;
    };

    class alignas(64) ParallelExecutor final
    {
        struct WorkerThreadResult;

    public:
        /**
         * @remarks 반드시 range based for에서만 사용하고, 임의로 반복자를 조작하거나 이용하지 말 것 - 직관적인 방식으로 동작하지 않음.
         * @tparam TResult
         */
        template<typename TResult>
        class ExecutionResults;

        explicit ParallelExecutor(const uint32_t workerThreadCount,
                                  const size_t workerThreadResultPageBaseSize)
            : WorkerThreadResults{ std::make_unique<WorkerThreadResult[]>(workerThreadCount) },
              WorkerThreadContexts{ std::make_unique<WorkerThreadContext[]>(workerThreadCount) },
              WorkerThreadResultPageBaseSize{ workerThreadResultPageBaseSize },
              WorkerThreadCount{ workerThreadCount },
              work_{
                  [](const uint32_t)
                  {
                  }
              }
        {
            for (uint32_t threadIndex = 0; threadIndex < workerThreadCount; ++threadIndex)
            {
                WorkerThreadContexts[threadIndex].WorkingFlag.store(true, std::memory_order_release);
                WorkerThreadContexts[threadIndex].Thread = std::thread(&ParallelExecutor::WorkerThreadBody,
                                                                       this,
                                                                       threadIndex);
                WorkerThreadContexts[threadIndex].WorkingFlag.wait(true, std::memory_order_acquire);
            }
        }

        ~ParallelExecutor();

        ParallelExecutor(const ParallelExecutor&) = delete;

        ParallelExecutor(ParallelExecutor&&) = delete;

        ParallelExecutor& operator=(const ParallelExecutor&) = delete;

        ParallelExecutor& operator=(ParallelExecutor&&) = delete;

        template<typename TAxis,
            typename TExecutionResult,
            ExecutionPolicy Policy = ExecutionPolicy::TotallyImmutable>
        ExecutionResults<TExecutionResult> ParallelFor(auto&& getAxis,
                                                       auto&& task,
                                                       size_t chunkSize)
            requires IsGetAxisFn<decltype(getAxis), TAxis, Policy> &&
                     IsParallelForTask<decltype(task), TExecutionResult, TAxis, Policy>;


        template<typename TExecutionResult>
        ExecutionResults<TExecutionResult> ParallelForWorkerThreads(auto&& task)
            requires IsParallelForWorkerThreadsTask<TExecutionResult, decltype(task)>;

    private:
        struct WorkerThreadContext
        {
            std::thread Thread;
            std::atomic_bool WorkingFlag;
        };

        const std::unique_ptr<WorkerThreadResult[]> WorkerThreadResults;
        const std::unique_ptr<WorkerThreadContext[]> WorkerThreadContexts;
        const size_t WorkerThreadResultPageBaseSize;
        const uint32_t WorkerThreadCount;

        std::function<void(uint32_t)> work_;

        alignas(64) std::atomic_bool shouldStop_;
        alignas(64) std::atomic_uint32_t workIndex_;

        void WorkerThreadBody(uint32_t threadIndex);

        void ExtendPageAtLeast(WorkerThreadResult& threadResult, size_t atLeast);
    };

    struct ParallelExecutor::WorkerThreadResult final
    {
        size_t PageSize = 0;
        size_t ElementCount = 0;
        std::unique_ptr<char[]> Page;
    };

    template<typename TResult>
    class ParallelExecutor::ExecutionResults final
    {
    public:
        class Iterator final
        {
        public:
            using value_type = TResult;
            using reference = const value_type&;
            using iterator_category = std::input_iterator_tag;

            explicit Iterator(const std::span<WorkerThreadResult> threadContexts,
                              const uint32_t currentThreadResultIndex,
                              const uint32_t currentResultElementIndex)
                : threadResults_{ threadContexts },
                  currentThreadResultIndex_{ currentThreadResultIndex },
                  currentResultElementIndex_{ currentResultElementIndex }
            {
            }

            reference operator*() const
            {
                return *reinterpret_cast<value_type*>(
                    threadResults_[currentThreadResultIndex_].Page.get() + currentResultElementIndex_ * sizeof(
                        TResult));
            }

            Iterator& operator++()
            {
                currentResultElementIndex_ += 1;
                while (currentThreadResultIndex_ < threadResults_.size() &&
                       currentResultElementIndex_ >= threadResults_[currentThreadResultIndex_].
                       ElementCount)
                {
                    currentResultElementIndex_ = 0;
                    currentThreadResultIndex_ += 1;
                }

                return *this;
            }

            /**
             * end()와의 비교 상황만 가정하여, == end()인 상황, 즉 out of range 상황을 감지함.
             * @return
             */
            bool operator==(const Iterator&) const
            {
                return currentThreadResultIndex_ >= threadResults_.size();
            }

            /**
             * end()와의 비교 상황만 가정하여, != end()인 상황, 즉 in range 상황을 감지함.
             * @return
             */
            bool operator!=(const Iterator&) const
            {
                return currentThreadResultIndex_ < threadResults_.size();
            }

        private:
            std::span<WorkerThreadResult> threadResults_;
            uint32_t currentThreadResultIndex_;
            uint32_t currentResultElementIndex_;
        };

        explicit ExecutionResults(std::span<WorkerThreadResult> threadContexts)
            : workerThreadResults_{ threadContexts }
        {
        }

        ~ExecutionResults() = default;

        ExecutionResults(const ExecutionResults&) = delete;

        ExecutionResults(ExecutionResults&&) = delete;

        ExecutionResults& operator=(const ExecutionResults&) = delete;

        ExecutionResults& operator=(ExecutionResults&&) = delete;

        Iterator begin() const
        {
            uint32_t nonEmptyThreadIndex = 0;
            for (nonEmptyThreadIndex = 0; nonEmptyThreadIndex < workerThreadResults_.size(); ++nonEmptyThreadIndex)
            {
                if (workerThreadResults_[nonEmptyThreadIndex].ElementCount > 0)
                {
                    break;
                }
            }

            return Iterator{ workerThreadResults_, nonEmptyThreadIndex, 0 };
        }

        Iterator end() const
        {
            return Iterator{ workerThreadResults_, static_cast<uint32_t>(workerThreadResults_.size()), 0 };
        }

    private:
        std::span<WorkerThreadResult> workerThreadResults_;
    };

    inline ParallelExecutor::~ParallelExecutor()
    {
        if (shouldStop_.load(std::memory_order_relaxed))
        {
            return;
        }

        shouldStop_.store(true, std::memory_order_relaxed);
        work_ = [](const uint32_t)
        {
        };

        for (int threadId = 0; threadId < WorkerThreadCount; ++threadId)
        {
            WorkerThreadContexts[threadId].WorkingFlag.store(true, std::memory_order_release);
            WorkerThreadContexts[threadId].WorkingFlag.notify_one();
            WorkerThreadContexts[threadId].Thread.join();
        }
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    inline void ParallelExecutor::ExtendPageAtLeast(WorkerThreadResult& threadResult, const size_t atLeast)
    {
        while (threadResult.PageSize < atLeast)
        {
            const size_t oldSize = threadResult.PageSize;
            const size_t newSize = threadResult.PageSize == 0
                                       ? WorkerThreadResultPageBaseSize
                                       : threadResult.PageSize * 2;
            auto newBlock = std::make_unique<char[]>(newSize);
            memcpy(newBlock.get(), threadResult.Page.get(), oldSize);
            threadResult.Page = std::move(newBlock);
            threadResult.PageSize = newSize;
        }
    }

    template<typename TAxis, typename TExecutionResult, ExecutionPolicy Policy> ParallelExecutor::ExecutionResults<
        TExecutionResult> ParallelExecutor::ParallelFor(auto&& getAxis, auto&& task, size_t chunkSize) requires
        IsGetAxisFn<decltype
            (getAxis), TAxis, Policy> && IsParallelForTask<decltype(task), TExecutionResult, TAxis, Policy>
    {
        work_ = [this, chunkSize, &task, &getAxis](const uint32_t threadIndex)
        {
            auto& threadResult = WorkerThreadResults[threadIndex];
            while (true)
            {
                const auto workEnd = workIndex_.fetch_add(chunkSize, std::memory_order_relaxed);
                const auto workBegin = workEnd - chunkSize;

                for (uint32_t i = static_cast<uint32_t>(workBegin); i < workEnd; ++i)
                {
                    const auto axis = getAxis(i);
                    if (!axis)
                    {
                        return;
                    }

                    const auto result = task(*axis);
                    if (!result)
                    {
                        continue;
                    }

                    ExtendPageAtLeast(threadResult, sizeof(TExecutionResult) * (threadResult.ElementCount + 1));
                    memcpy(
                        threadResult.Page.get() + sizeof(TExecutionResult) * threadResult.ElementCount,
                        &*result,
                        sizeof(TExecutionResult));
                    threadResult.ElementCount += 1;
                }
            }
        };

        workIndex_.store(0, std::memory_order_relaxed);

        for (int i = 0; i < WorkerThreadCount; ++i)
        {
            WorkerThreadResults[i].ElementCount = 0;
            WorkerThreadContexts[i].WorkingFlag.store(true, std::memory_order_release);
            WorkerThreadContexts[i].WorkingFlag.notify_one();
        }
        for (int i = 0; i < WorkerThreadCount; ++i)
        {
            WorkerThreadContexts[i].WorkingFlag.wait(true, std::memory_order_acquire);
        }

        return ExecutionResults<TExecutionResult>{ std::span{ WorkerThreadResults.get(), WorkerThreadCount } };
    }

    template<typename TExecutionResult> ParallelExecutor::ExecutionResults<TExecutionResult> ParallelExecutor::
    ParallelForWorkerThreads(auto&& task) requires IsParallelForWorkerThreadsTask<TExecutionResult, decltype(task)>
    {
        work_ = [this, &task](const uint32_t threadIndex)
        {
            auto& threadResult = WorkerThreadResults[threadIndex];
            const auto result = task();
            if (!result)
            {
                return;
            }

            ExtendPageAtLeast(threadResult, sizeof(TExecutionResult));
            memcpy(
                threadResult.Page.get() + sizeof(TExecutionResult),
                &*result,
                sizeof(TExecutionResult));
            threadResult.ElementCount = 1;
        };

        for (int threadId = 1; threadId <= WorkerThreadCount; ++threadId)
        {
            WorkerThreadResults[threadId - 1].ElementCount = 0;
            WorkerThreadContexts[threadId - 1].WorkingFlag.store(true, std::memory_order_release);
            WorkerThreadContexts[threadId - 1].WorkingFlag.notify_one();
        }
        for (int threadId = 1; threadId <= WorkerThreadCount; ++threadId)
        {
            WorkerThreadContexts[threadId - 1].WorkingFlag.wait(true, std::memory_order_acquire);
        }

        return ExecutionResults<TExecutionResult>{ std::span{ WorkerThreadResults.get(), WorkerThreadCount } };
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    inline void ParallelExecutor::WorkerThreadBody(const uint32_t threadId)
    {
        auto& threadContext = WorkerThreadContexts[threadId];
        while (!shouldStop_.load(std::memory_order_relaxed))
        {
            threadContext.WorkingFlag.wait(false, std::memory_order_acquire);
            if (shouldStop_.load(std::memory_order_relaxed))
            {
                break;
            }

            work_(threadId);
            threadContext.WorkingFlag.store(false, std::memory_order_release);
            threadContext.WorkingFlag.notify_one();
        }

        threadContext.WorkingFlag.store(false, std::memory_order_release);
        threadContext.WorkingFlag.notify_one();
    }
}

#endif // RATKINIASERVER_PARALLELEXECUTOR_H
