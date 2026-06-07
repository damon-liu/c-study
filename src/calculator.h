#pragma once
#include <string>
#include <vector>

class Calculator {
public:
    Calculator();
    ~Calculator();
    
    double add(double a, double b);
    double subtract(double a, double b);
    double multiply(double a, double b);
    double divide(double a, double b);
    
    std::vector<double> solveQuadratic(double a, double b, double c);
    
private:
    bool initialized_;
};