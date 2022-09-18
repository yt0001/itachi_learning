/*version 1: 无锁
#include <thread>
#include <iostream>
int val = 0;
void* func(int val) {
    for (int i = 0; i < 100000; ++i) {
        val = val + 1;
    }
    return nullptr;
}
int main() {
    std::thread thread_arr[10];
    for (int i = 0; i < 10; ++i) {
        thread_arr[i] = std::move(std::thread(func, 0));
    }
    for (int i = 0; i < 10; ++i) {
        thread_arr[i].join();
    }

    std::cout << val << std::endl;
    return 0;
}*/

#include <thread>
#include <iostream>
#include <mutex>
int val = 0;
std::mutex m;
void* func(int) {
    for (int i = 0; i < 100000; ++i) {
        std::lock_guard<std::mutex> lk(m);
        val = val + 1;
    }
    return nullptr;
}
int main() {
    std::thread thread_arr[10];
    for (int i = 0; i < 10; ++i) {
        thread_arr[i] = std::move(std::thread(func, 0));
    }
    for (int i = 0; i < 10; ++i) {
        thread_arr[i].join();
    }

    std::cout << val << std::endl;
    return 0;
}