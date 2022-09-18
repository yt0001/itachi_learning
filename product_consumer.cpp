#include <iostream>
#include <mutex>
#include <thread>
#include <queue>
#include <condition_variable>
#include <vector>
#include <memory>
#include <unistd.h>
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
std::queue<Task*> q;
void consumer() {
    Task* task = nullptr;
    while (true) {
        std::unique_lock<std::mutex> ul(m);
        while(q.empty()) {
            cv.wait(ul);
        } 
        task = q.front();
        q.pop();
        ul.unlock();
        if (task == nullptr) {
            continue;
        }
        task->doTask();
        delete task;
        task = nullptr;
    }
}

void productor(int val) {
    for (;;) {
        Task* task = new Task(val);
        std::unique_lock<std::mutex> ul(m);
        q.push(task);
        ul.unlock();
        cv.notify_one();
        val++;
        sleep(1);
    }
}

int main() {
    std::vector<std::shared_ptr<std::thread>> consumer_arr(5);
    for (int i = 0; i < 5; ++i) {
        consumer_arr[i].reset(new std::thread(consumer));
    }
    std::thread productor_thd(productor, 0);
    productor_thd.join();
    for (int i = 0; i < 5; ++i) {
        consumer_arr[i]->join();
    }
    return 0;
}