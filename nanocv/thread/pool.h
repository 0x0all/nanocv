#pragma once

#include "thread.h"
#include "nanocv/arch.h"
#include "nanocv/noncopyable.hpp"
#include <deque>
#include <vector>
#include <condition_variable>

namespace ncv
{
        ///
        /// \brief asynchronously runs multiple workers/jobs/threads
        /// by enqueing and distribute them on all available threads
        ///
        /// NB: this is heavily copied/inspired by http://progsch.net/wordpress/?p=81
        ///
        class NANOCV_PUBLIC thread_pool_t : private noncopyable_t
        {
        public:

                typedef std::function<void()>           task_t;
                typedef std::thread                     thread_t;
                typedef std::mutex                      mutex_t;
                typedef std::unique_lock<mutex_t>       lock_t;
                typedef std::condition_variable         condition_t;

                ///
                /// \brief constructor
                ///
                explicit thread_pool_t(size_t nthreads = 0);

                ///
                /// \brief destructor
                ///
                ~thread_pool_t();

                ///
                /// \brief enqueue a new task to execute
                ///
                template<class F>
                void enqueue(F f)
                {
                        _enqueue(f);
                }

                ///
                /// \brief wait for all workers to finish running the tasks
                ///
                void wait();

                // access functions
                size_t n_workers() const { return m_workers.size(); }
                size_t n_jobs() const { return m_data.m_tasks.size(); }

        private:

                ///
                /// \brief collect the tasks to run
                ///
                struct data_t
                {
                        ///
                        /// \brief constructor
                        ///
                        data_t() :      m_running(0),
                                        m_stop(false)
                        {
                        }

                        // attributes
                        std::deque<task_t>      m_tasks;                ///< tasks (functors) to execute
                        size_t                  m_running;              ///< #running threads
                        mutex_t                 m_mutex;                ///< synchronize task access
                        condition_t             m_condition;            ///< signaling
                        bool                    m_stop;                 ///< stop requested
                };

                ///
                /// \brief worker unit (to execute tasks)
                ///
                struct worker_t
                {
                        ///
                        /// \brief constructor
                        ///
                        worker_t(data_t& data) : m_data(data)
                        {
                        }

                        ///
                        /// \brief execute tasks when available
                        ///
                        void operator()();

                        // attributes
                        data_t&                 m_data;                 ///< Tasks
                };

                ///
                /// \brief add a new task to execute (implementation)
                ///
                template<class F>
                void _enqueue(F f)
                {
                        {
                                const lock_t lock(m_data.m_mutex);
                                
                                m_data.m_tasks.push_back(task_t(f));
                        }
                        m_data.m_condition.notify_one();
                }

        private:

                // attributes
                std::vector<thread_t>           m_workers;              ///< worker threads
                data_t                          m_data;                 ///< tasks to execute + synchronization
        };
}