/*
    Copyright 2021 XITRIX

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include <borealis/core/thread.hpp>
#include <libretro-common/retro_timers.h>

#if defined(__SWITCH__)
#include <switch.h>
#endif

namespace brls {

#ifdef __SWITCH__
    static Thread task_loop_thread;
#endif

Threading::Threading()
{
    start_task_loop();
}

void sync(const std::function<void()> &func)
{
    Threading::sync(func);
}

void async(const std::function<void()> &task)
{
    Threading::async(task);
}

void Threading::sync(const std::function<void()> &func) {
    std::lock_guard<std::mutex> guard(m_sync_mutex);
    m_sync_functions.push_back(func);
}

void Threading::async(const std::function<void()> &task) {
    std::lock_guard<std::mutex> guard(m_async_mutex);
    m_async_tasks.push_back(task);
}

void Threading::performSyncTasks()
{
    m_sync_mutex.lock();
    auto local = m_sync_functions;
    m_sync_functions.clear();
    m_sync_mutex.unlock();
    
    for (auto &f : local)
        f();
}

void Threading::start() {
    start_task_loop();
}

void Threading::stop() {
    task_loop_active = false;
    
    #ifdef __SWITCH__
    threadWaitForExit(&task_loop_thread);
    threadClose(&task_loop_thread);
    #endif
}

void Threading::task_loop() {
    while (task_loop_active) {
        std::vector<std::function<void()>> m_tasks_copy; {
            std::lock_guard<std::mutex> guard(m_async_mutex);
            m_tasks_copy = m_async_tasks;
            m_async_tasks.clear();
        }
        
        for (auto task: m_tasks_copy) {
            task();
        }
        
        retro_sleep(500);
    }
}

void Threading::start_task_loop() {
#ifdef __SWITCH__
    threadCreate(
        &task_loop_thread,
        [](void* a) {
            task_loop();
        },
        NULL,
        NULL,
        0x10000,
        0x2C,
        -2
    );
    threadStart(&task_loop_thread);
#else
    task_loop_active = true;
    auto thread = std::thread([](){
        task_loop();
    });
    thread.detach();
#endif
}

} // namespace brls
