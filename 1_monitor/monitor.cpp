#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
using namespace std::chrono_literals;

std::mutex m;
std::condition_variable cv;
bool ready = false;
std::vector<int> v = { 3, 1, 4 };

void provide() {
    while (true) {
        std::unique_lock<std::mutex> lk(m);
        if (ready) {
            lk.unlock();
            continue;
        }

        // Add a new random element (between 0 and 10) to the vector
        int newElement = rand() % 11;
        v.push_back(newElement);
        std::cout << "Added element: " << newElement << "\n";
        std::cout << "Provided\n";
        std::this_thread::sleep_for(1000ms);

        ready = true;
        cv.notify_one();
        lk.unlock();
    }
}

void consume() {
    while (true) {
        std::unique_lock<std::mutex> lk(m);
        while (!ready) {
            cv.wait(lk, [] {return ready; });
            std::cout << "awoke\n";
        }
        std::cout << "Consumed\n";
        for (auto item : v) {
            std::cout << item << " ";
        }

        // Remove a random element from the vector
        if (!v.empty()) {
            int randomIndex = rand() % v.size();
            int deletedElement = v[randomIndex];
            auto it = std::next(v.begin(), randomIndex);
            v.erase(it);
            std::cout << "\nDeleted element: " << deletedElement << "\n";
        }

        std::cout << "\n\n";

        ready = false;
        lk.unlock();
    }
}

int main() {
    std::srand(std::time(0));

    std::thread provider(provide);
    std::thread consumer(consume);

    provider.join();
    consumer.join();
}
