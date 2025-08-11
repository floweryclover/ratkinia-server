// //
// // Created by floweryclover on 2025-08-08.
// //
//
// #ifndef EXECUTOR_H
// #define EXECUTOR_H
//
// #include <atomic>
//
// class Executor final
// {
//     struct ThreadContext
//     {
//         std::atomic_bool WorkingFlag;
//         std::unique_ptr<char[]> MemoryBlock;
//         size_t MemoryBlockSize{ 0 };
//         size_t ResultElementCount{};
//     };
//
//     static constexpr size_t BaseThreadMemorySize = 1024;
//
//     const uint32_t WorkerThreadCount;
//     const std::vector<F_SystemData> Systems;
//     const F_MutableContext* multiThreadUpdateContext_;
//     const std::unique_ptr<ThreadContext[]> threadContexts_;
//     std::function<void(uint32_t)> work_;
//
//     alignas(U_Concurrency::CacheLineSize) std::atomic_bool shouldStop_; // 메인 스레드에서 설정하는, 워커 스레드들의 완전한 종료 명령 상태.
//     alignas(U_Concurrency::CacheLineSize) std::atomic_uint32_t multiThreadWorkIndex_;
//     // Component인 경우 dense Index, Event인 경우 EventQueue에서의 Index.
//
//     void WorkerThreadBody(int threadId);
// };
//
// template<IsEventOrComponent TAxis, IsTriviallyCopyable TExecutionResult, E_Execution Execution>
// std::span<TExecutionResult> F_SystemManager::Execute(const F_MutableContext& context, auto&& fn, const size_t chunkSize)
//     requires (IsEvent<TAxis> && IsHandlerForEventExecutor<TAxis, TExecutionResult, decltype(fn), Execution>) ||
//              (IsComponent<TAxis> && IsHandlerForComponentExecutor<TAxis, TExecutionResult, decltype(fn), Execution>)
// {
//     struct WorkerContext
//     {
//         const F_MutableContext& Mutable;
//         const F_ImmutableContext& Immutable;
//         const size_t ChunkSize;
//     };
//
//     const auto workerContext = WorkerContext { context, static_cast<F_ImmutableContext>(context), chunkSize };
//
//     work_ = [this, &fn, &workerContext](const uint32_t threadId)
//     {
//         auto& threadContext = threadContexts_[threadId];
//         while (true)
//         {
//             const auto workEnd = multiThreadWorkIndex_.fetch_add(workerContext.ChunkSize, std::memory_order_relaxed);
//             const auto workBegin = workEnd - workerContext.ChunkSize;
//
//             for (uint32_t i = static_cast<uint32_t>(workBegin); i < workEnd; ++i)
//             {
//                 std::optional<TExecutionResult> result;
//                 if constexpr (IsComponent<TAxis>)
//                 {
//                     const auto [entity, component] = [&]() -> std::pair<F_Entity, std::conditional_t<Execution == E_Execution::TotallyImmutable, const TAxis*, TAxis*>>
//                     {
//                         if constexpr (Execution == E_Execution::TotallyImmutable)
//                         {
//                             return workerContext.Immutable.EntityManager.template GetComponentFromDenseIndex<TAxis>(i);
//                         }
//                         else
//                         {
//                             return workerContext.Mutable.EntityManager.template GetComponentFromDenseIndex<TAxis>(i);
//                         }
//                     }();
//                     if (!component)
//                     {
//                         return;
//                     }
//                     result = fn(entity, *component, workerContext.Immutable);
//                 }
//                 else // 이벤트
//                 {
//                     const auto event = workerContext.Immutable.EventManager.template GetEventFromIndex<TAxis>(i);
//                     if (!event)
//                     {
//                         return;
//                     }
//                     result = fn(*event, workerContext.Immutable);
//                 }
//
//                 if (!result)
//                 {
//                     continue;
//                 }
//
//                 while (threadContext.MemoryBlockSize < sizeof(TExecutionResult) * (threadContext.ResultElementCount + 1))
//                 {
//                     const size_t oldSize = threadContext.MemoryBlockSize;
//                     const size_t newSize = threadContext.MemoryBlockSize == 0 ? BaseThreadMemorySize : threadContext.MemoryBlockSize * 2;
//                     auto newBlock = std::make_unique<char[]>(newSize);
//                     memcpy(newBlock.get(), threadContext.MemoryBlock.get(), oldSize);
//                     threadContext.MemoryBlock = std::move(newBlock);
//                     threadContext.MemoryBlockSize = newSize;
//                 }
//                 memcpy(
//                     threadContext.MemoryBlock.get() + sizeof(TExecutionResult) * threadContext.
//                     ResultElementCount,
//                     &*result,
//                     sizeof(TExecutionResult));
//                 threadContext.ResultElementCount += 1;
//             }
//         }
//     };
//
//     multiThreadUpdateContext_ = &context;
//     multiThreadWorkIndex_.store(0, std::memory_order_relaxed);
//
//     for (int threadId = 1; threadId <= WorkerThreadCount; ++threadId)
//     {
//         threadContexts_[threadId].ResultElementCount = 0;
//         threadContexts_[threadId].WorkingFlag.store(true, std::memory_order_release);
//         threadContexts_[threadId].WorkingFlag.notify_one();
//     }
//     for (int threadId = 1; threadId <= WorkerThreadCount; ++threadId)
//     {
//         threadContexts_[threadId].WorkingFlag.wait(true, std::memory_order_acquire);
//     }
//
//     // 이제 메인 스레드가 취합함.
//     auto& mainThreadContext = threadContexts_[0];
//     mainThreadContext.ResultElementCount = 0;
//
//     const size_t totalResultElementCount = [&]
//     {
//         size_t sum = 0;
//         for (int threadId = 1; threadId <= WorkerThreadCount; ++threadId)
//         {
//             sum += threadContexts_[threadId].ResultElementCount;
//         }
//         return sum;
//     }();
//
//     // 메모리 부족하면 재할당
//     while (mainThreadContext.MemoryBlockSize < sizeof(TExecutionResult) * totalResultElementCount)
//     {
//         // 일단 이전 사이즈의 2배
//         const size_t newSize = mainThreadContext.MemoryBlockSize == 0 ? BaseThreadMemorySize : mainThreadContext.MemoryBlockSize * 2;
//         auto newBlock = std::make_unique<char[]>(newSize);
//         mainThreadContext.MemoryBlockSize = newSize;
//         mainThreadContext.MemoryBlock = std::move(newBlock);
//     }
//
//     for (int threadId = 1; threadId <= WorkerThreadCount; ++threadId)
//     {
//         const auto& workerThreadContext = threadContexts_[threadId];
//
//         memcpy(mainThreadContext.MemoryBlock.get() + mainThreadContext.ResultElementCount * sizeof(TExecutionResult),
//             workerThreadContext.MemoryBlock.get(),
//             sizeof(TExecutionResult) * workerThreadContext.ResultElementCount);
//         mainThreadContext.ResultElementCount += workerThreadContext.ResultElementCount;
//     }
//
//     return std::span{
//         reinterpret_cast<TExecutionResult*>(threadContexts_[0].MemoryBlock.get()), threadContexts_[0].ResultElementCount
//     };
// };
//
// #endif //EXECUTOR_H
