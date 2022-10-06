#include <iostream>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <vector>
#include <thread>
#include <list>

std::mutex m;
std::condition_variable cv;
class Task{
public:
    Task(int taskID) {
        this->taskID = taskID;
    }
    void doTask() {
        std::cout << "do task, taskID:" << taskID << ", threadID:" << std::this_thread::get_id() << std::endl;
    }
private:
    int taskID;
};

class Itachi_Thread_Pool {
public:
    Itachi_Thread_Pool():_is_running(false){}
    void add_task(Task* task) {
        std::shared_ptr<Task> spTask;
        spTask.reset(task);

        std::unique_lock<std::mutex> lk(_mux);
        _task_list.push_back(spTask);
        lk.unlock();
        cv.notify_one();
    }
    
    void destory_thread_pool() {
        std::lock_guard<std::mutex> lk(_mux);
        for (auto& item : _task_list) {
            item.reset();
        }
        _task_list.clear();
    }
    void threadFunc() {
        std::shared_ptr<Task> spTask;
        while (true) {
            std::unique_lock<std::mutex> ul(_mux);
            while(_task_list.empty()) {
                if (!_is_running) {
                    break;
                }
                cv.wait(ul);
            }
            if(!_is_running) {
                break;
            }
            spTask = _task_list.back();
            ul.unlock();
            if (spTask == nullptr) {
                continue;
            }
            spTask->doTask();
            spTask.reset();
        }
    }
    void init(int threadNum = 5) {
        if (threadNum < 5) {
            threadNum = 5;
        }
        for (int i = 0; i< threadNum; ++i) {
            std::shared_ptr<std::thread> thread_ptr;
            thread_ptr.reset(new std::thread(&Itachi_Thread_Pool::threadFunc, this));
            _thread_vec.push_back(thread_ptr);
        }
        _is_running = true;
    }
    void stop() {
        _is_running = false;
        
        for (auto& thread_item : _thread_vec) {
            thread_item->join();
        }
        _cv.notify_all();
    }
private:
    std::vector<std::shared_ptr<std::thread>> _thread_vec;
    std::list<std::shared_ptr<Task>> _task_list;
    std::condition_variable _cv;
    std::mutex _mux;
    int _pool_num;
    bool _is_running;
};

int main() {
    Itachi_Thread_Pool thp;
    thp.init();

    for (int i = 0 ;i < 10; ++i) {
        Task* task = new Task(i);
        thp.add_task(task);
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));
    thp.stop();
    return 0;
}