#pragma once

#include <list>
#include <iostream>
#include <memory>

template <typename T>
class Tracer
{
public:
    Tracer(std::weak_ptr<T> val) : value(val) {

    };
    static void DrawTrace() {};
    static void DrawTraced() {
        std::cout << name << "(" << value;
        DrawValue();
        std::cout << "\n ";
    };
    virtual void DrawValue() = 0;
private:
    char color;
    std::string name;
    std::weak_ptr T value;
    static std::list<Tracer*> traced;
};

template <typename T>
class TraceValue : public Tracer<T>
{
    virtual void DrawValue() {
        std::cout << value;
    };
};
