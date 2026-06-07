#include "calculator.h"
#include "logger.h"
#include <iostream>
#include <thread>
#include <chrono>

int main(int argc, char* argv[]) {
    std::cout << "=== C++ Demo Application ===" << std::endl;
    std::cout << "Compiled for Linux on: " << __DATE__ << " " << __TIME__ << std::endl;
    std::cout << "===========================" << std::endl;
    
    // 设置日志文件
    Logger::getInstance().setLogFile("app.log");
    Logger::getInstance().log("Application started");
    
    // 创建计算器实例
    Calculator calc;
    
    // 执行一些计算
    std::cout << "\n=== Calculations ===" << std::endl;
    std::cout << "10 + 5 = " << calc.add(10, 5) << std::endl;
    std::cout << "10 - 5 = " << calc.subtract(10, 5) << std::endl;
    std::cout << "10 * 5 = " << calc.multiply(10, 5) << std::endl;
    std::cout << "10 / 5 = " << calc.divide(10, 5) << std::endl;
    
    // 解二次方程
    std::cout << "\n=== Solving Quadratic Equation ===" << std::endl;
    auto solutions = calc.solveQuadratic(1, -5, 6);  // x^2 - 5x + 6 = 0
    if (!solutions.empty()) {
        std::cout << "Solutions: ";
        for (double x : solutions) {
            std::cout << x << " ";
        }
        std::cout << std::endl;
    }
    
    // 演示多线程（C++11标准线程）
    std::cout << "\n=== Multi-threading Demo ===" << std::endl;
    std::thread t1([&calc]() {
        Logger::getInstance().log("Thread 1: Calculating 100 + 200");
        auto result = calc.add(100, 200);
        std::cout << "Thread 1 result: " << result << std::endl;
    });
    
    std::thread t2([&calc]() {
        Logger::getInstance().log("Thread 2: Calculating 500 / 25");
        auto result = calc.divide(500, 25);
        std::cout << "Thread 2 result: " << result << std::endl;
    });
    
    t1.join();
    t2.join();
    
    Logger::getInstance().log("Application finished successfully");
    std::cout << "\n=== Demo Completed ===" << std::endl;
    
    return 0;
}