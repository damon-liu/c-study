#include "calculator.h"
#include "logger.h"
#include <cmath>
#include <vector>

Calculator::Calculator() : initialized_(true) {
    Logger::getInstance().log("Calculator initialized");
}

Calculator::~Calculator() {
    Logger::getInstance().log("Calculator destroyed");
}

double Calculator::add(double a, double b) {
    double result = a + b;
    Logger::getInstance().log("add(" + std::to_string(a) + ", " + 
                              std::to_string(b) + ") = " + std::to_string(result));
    return result;
}

double Calculator::subtract(double a, double b) {
    double result = a - b;
    Logger::getInstance().log("subtract(" + std::to_string(a) + ", " + 
                              std::to_string(b) + ") = " + std::to_string(result));
    return result;
}

double Calculator::multiply(double a, double b) {
    double result = a * b;
    Logger::getInstance().log("multiply(" + std::to_string(a) + ", " + 
                              std::to_string(b) + ") = " + std::to_string(result));
    return result;
}

double Calculator::divide(double a, double b) {
    if (b == 0) {
        Logger::getInstance().log("ERROR: Division by zero!");
        return 0;
    }
    double result = a / b;
    Logger::getInstance().log("divide(" + std::to_string(a) + ", " + 
                              std::to_string(b) + ") = " + std::to_string(result));
    return result;
}

std::vector<double> Calculator::solveQuadratic(double a, double b, double c) {
    std::vector<double> solutions;
    
    double discriminant = b * b - 4 * a * c;
    
    if (discriminant < 0) {
        Logger::getInstance().log("Quadratic: No real solutions");
        return solutions;
    }
    
    if (discriminant == 0) {
        double x = -b / (2 * a);
        solutions.push_back(x);
        Logger::getInstance().log("Quadratic: One solution x = " + std::to_string(x));
    } else {
        double x1 = (-b + sqrt(discriminant)) / (2 * a);
        double x2 = (-b - sqrt(discriminant)) / (2 * a);
        solutions.push_back(x1);
        solutions.push_back(x2);
        Logger::getInstance().log("Quadratic: Two solutions x1 = " + 
                                  std::to_string(x1) + ", x2 = " + std::to_string(x2));
    }
    
    return solutions;
}