// Parallel Opimization hw1_thread_pool id:312611101
#include <iostream>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <chrono>
#include <atomic>
#include <random>
#include <iomanip>

class ThreadPool {
private:
    // store jobs
    std::queue<std::function<void(void)>> jobs;

    // store threads
    std::vector<std::thread> threads;

    // Time record for each thread
    std::vector<std::chrono::microseconds> threadRunTimes;

    // ID for each thread - store as std::thread::id instead of int
    std::vector<std::thread::id> threadIDs;

    // Mutex for jobs(protecting the shared resource)
    mutable std::mutex mutex;  // Mark as mutable to allow locking in const methods

    // Condition variable for new jobs to notify threads
    std::condition_variable condVar;

    bool shouldStop;

    // For printing
    std::mutex print_Mutex;
    std::condition_variable print_CondVar;
    std::atomic<int> print_Count;

public:
    // Constructor
    ThreadPool() : shouldStop(false), print_Count(0) {

        const int numThreads = 5;
        threadRunTimes.resize(numThreads);
        threadIDs.resize(numThreads);

        for (int i = 0; i < numThreads; ++i) {
            // Create threads
            threads.emplace_back([this, i] {
                // For each thread, record the start time
                auto startTime = std::chrono::steady_clock::now();
                // Record the thread ID to verify the different threads
                threadIDs[i] = std::this_thread::get_id();

                while (true) {
                    std::function<void()> job;
                    {
                        // 當創建lock時，它會自動嘗試獲取mutex
                        // 如果mutex已被其他thread持有，當前線程會wait直到能獲取到mutex(resource)
                        std::unique_lock<std::mutex> lock(mutex);
                        condVar.wait(lock, [this] { return !jobs.empty() || shouldStop; });  // Fixed lambda return type

                        if (shouldStop && jobs.empty()) {break;}
                        // Get the job from the first element of the queue
                        job = jobs.front();
                        jobs.pop();
                    }
                    // Execute the first job
                    job();
                }
                // Record the end time
                auto endTime = std::chrono::steady_clock::now();
                // Calculate the time duration
                threadRunTimes[i] = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
            });
        }
    }

    // Destructor
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(mutex);
            shouldStop = true;
            // Set the flag to stop the threads(All threads will stop after finishing the current job)
        }
        // notifiy all waiting threads
        condVar.notify_all();

        // Join all threads, wait for all threads to finish
        for (auto& thread : threads) {
            thread.join();
        }

        // Print the thread ID and the time duration
        std::cout << "\nThread Pool destroyed!" << "\nThread run times:" << std::endl;
        for (size_t i = 0; i < threads.size(); ++i) {
            std::cout << "Thread " << threadIDs[i] << ": " << threadRunTimes[i].count() << " us" << std::endl;
        }
    }

    // Enqueue job
    template<typename F>
    void enqueue(F&& job) {
        {
            std::unique_lock<std::mutex> lock(mutex);
            jobs.emplace(std::forward<F>(job));
        }
        condVar.notify_one();
    }

    size_t jobCount() const {
        std::unique_lock<std::mutex> lock(mutex);  // Now works because mutex is mutable
        return jobs.size();
    }

    void increment_printCount() {
        print_Count++;
        // print_CondVar.notify_one();
        print_CondVar.notify_all(); 
        
    }

    int get_printCount() const {
        return print_Count;
    }

    std::mutex& get_printMutex() {
        return print_Mutex;
    }

    std::condition_variable& get_printCondVar() {
        return print_CondVar;
    }
};

void print_1(ThreadPool& pool) {
    // random number generator
    thread_local std::random_device rd;
    thread_local std::mt19937 gen(rd());
    thread_local std::uniform_int_distribution<int> dis(1, 1000);

    int randomNum = dis(gen);

    {
        // mutex for printing
        std::unique_lock<std::mutex> lock(pool.get_printMutex());

        // Print 1 for odd numbers, 0 for even numbers (corrected logic)
        if (randomNum % 2 == 0) {
            std::cout << "0" << std::flush;
        } else {
            std::cout << "1" << std::flush;
        }

        // Notify finishing a print_1 job
        pool.increment_printCount();

        static std::atomic<int> counter(0);
        counter++;
        std::cout << " [" << counter << "] " << std::flush;
    }
}

// Construct functor
struct ADD {
    ThreadPool& pool;
    // static std::atomic<int> print2_executed;
    
    ADD(ThreadPool& p) : pool(p) {}
    
    void operator()() {
        {
            std::unique_lock<std::mutex> lock(pool.get_printMutex());
            
            pool.get_printCondVar().wait(lock, [this] {
                return pool.get_printCount() >= 496;
            });
            
            std::cout << "2" << std::flush;
        }
        // print2_executed++;
    }
};

// std::atomic<int> ADD::print2_executed(0);


int main(void) {
    {
        ThreadPool pool;

        ADD print_2(pool);

        // First send 496 functions
        for (int i = 0; i < 496; i++) {
            pool.enqueue([&pool] {print_1(pool);});
        }

        // then 4 functors into the pool
        for (int i = 0; i < 4; i++) {
            pool.enqueue([&print_2] {
                print_2();
            });
        }

        // // Wait for all jobs to finish
        // while (pool.jobCount() > 0) {
        //     std::this_thread::sleep_for(std::chrono::microseconds(2));
        // }
    }

    std::cout << "\nAll jobs have been processed." << std::endl;

    return 0;
}