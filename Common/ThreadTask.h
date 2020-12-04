#include <functional>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <memory>
#include <cassert>
#include <queue>
#include "base.h"
#include <atomic>

class ThreadTask
{
public:
    ThreadTask(const ThreadTask &other) = delete;
    ThreadTask &operator=(const ThreadTask &other) = delete;

    ThreadTask()
        : m_running(false)
    {
    }

    ~ThreadTask()
    {
        if (m_running)
        {
            stop();
        }
    }

    void Start()
    {
        m_running = true;
        m_thread.reset(new std::thread(std::bind(&ThreadTask::process, this)));
    }

    void stop()
    {
        {
            std::lock_guard<std::mutex> lk(m_mtx);
            m_running = false;
            m_not_empty_cond.notify_all();
        }
        m_thread->join();
    }

    void submit(Functor task)
    {
        m_count++;
        std::unique_lock<std::mutex> lk(m_mtx);
        m_queue.push(std::move(task));
        m_not_empty_cond.notify_one();
    }

private:
    void process()
    {
        printf("**********************   计算线程开始运行    **********************\n");
        while (m_running)
        {
            Functor task;
            {
                std::unique_lock<std::mutex> lk(m_mtx);
                while (m_queue.empty())
                {
                    m_not_empty_cond.wait(lk);
                }
                if (!m_running)
                    break;

                if (!m_queue.empty())
                {
                    task = m_queue.front();
                    m_queue.pop();
                }
            }
            std::cout << "********************   计算线程中的任务数量= " << m_count << "    *******************************\n";
            m_count--;
            task();
        }
    }

    mutable std::mutex m_mtx;
    std::condition_variable m_not_empty_cond;
    std::unique_ptr<std::thread> m_thread;
    std::queue<Functor> m_queue;
    std::atomic<int> m_count;
    bool m_running;
};