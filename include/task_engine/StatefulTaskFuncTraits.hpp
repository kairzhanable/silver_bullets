#pragma once

#include "types.hpp"
#include "TaskFuncRegistry.hpp"

#include <functional>

namespace silver_bullets {
namespace task_engine {

template<class TaskFunc>
struct StatefulTaskFuncTraits
{
    using ThreadLocalData = boost::any;
    using ReadOnlySharedData = boost::any;
};

template<class TaskFunc>
class StatefulThreadedTaskExecutorInit
{
public:
    template<class F>
    explicit StatefulThreadedTaskExecutorInit(
            const TaskFuncRegistry<TaskFunc> *taskFuncRegistry,
            const F& init) :
        taskFuncRegistry(taskFuncRegistry),
        m_init(init)
    {}
    typename TaskFuncTraits<TaskFunc>::ThreadLocalData initThreadLocalData() const {
        return m_init();
    }
    const TaskFuncRegistry<TaskFunc> *taskFuncRegistry;
private:
    std::function<boost::any()> m_init;
};

} // namespace task_engine
} // namespace silver_bullets
