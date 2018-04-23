#ifndef CTRPLUGINFRAMEWORKIMPL_SYSTEM_TASK_HPP
#define CTRPLUGINFRAMEWORKIMPL_SYSTEM_TASK_HPP

#include "types.h"
#include "ctrulib/synchronization.h"

namespace CTRPluginFramework
{
    using TaskFunc = s32 (*)(void *);

    struct TaskContext
    {
        int         refcount{0};
        u32         flags{0};
        s32         result{0};
        void *      arg{nullptr};
        TaskFunc    func{nullptr};
        LightEvent  event{};
    };

    struct Task
    {
        enum
        {
            Idle = 0,
            Scheduled = 1,
            Processing = 2,
            Finished = 4
        };

        TaskContext     *context;

        explicit Task(TaskFunc func, void *arg = nullptr);
        Task(const Task &task);
        Task(Task &&task) noexcept;
        ~Task(void);

        Task &operator=(const Task &right) = delete;

        /**
         * \brief Schedule a Task and starts it
         * \return 0 on operation success
         */
        int     Start(void) const;

        /**
         * \brief Wait for the Task to be completed
         * \return The result of the Task
         */
        s32     Wait(void) const;

        /**
         * \brief Get the current status of the Task
         * \return Task status (see enum)
         */
        u32     Status(void) const;
    };
}

#endif
