#include <condition_variable>
#include <mutex>
#include <thread>
#include <iostream>
#include <queue>
#include <chrono>
 
int main()
{
    std::queue<int> produced_nums;
    std::mutex m;
    std::condition_variable cond_var;
    bool done = false;
    bool notified = false;
 
    std::thread producer([&]() {
        for (int i = 0; i < 5; ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::unique_lock<std::mutex> lock(m);
            std::cout << "producing " << i << '\n';
			for(int k=0;k<i+1;k++)
				produced_nums.push(k);
            notified = true;
            cond_var.notify_one();
        }
 
        done = true;
        cond_var.notify_one();
    }); 
 
    std::thread consumer1([&]() {
        std::unique_lock<std::mutex> lock(m);
        while (!done) {
            while (!notified) {  // loop to avoid spurious wakeups
				
                cond_var.wait(lock);
            }   
            while (!produced_nums.empty()) {
                std::cout << "consuming 1 " << produced_nums.front() << '\n';
                produced_nums.pop();
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }   
            notified = false;
        }   
    }); 
    //~ std::thread consumer2([&]() {
        //~ std::unique_lock<std::mutex> lock(m);
        //~ while (!done) {
            //~ while (!notified) {  // loop to avoid spurious wakeups
                //~ cond_var.wait(lock);
            //~ }   
            //~ while (!produced_nums.empty()) {
                //~ std::cout << "consuming 2 " << produced_nums.front() << '\n';
                //~ produced_nums.pop();
                //~ std::this_thread::sleep_for(std::chrono::seconds(1));
            //~ }   
            //~ notified = false;
        //~ }   
    //~ }); 
 
    producer.join();
    consumer1.join();
    //~ consumer2.join();
}
