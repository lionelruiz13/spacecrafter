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
    static void DrawTraced(std::unique_ptr<Tracer> trace) {
        std::cout << name;
        move_in_line()
        DrawValue();
        std::cout << "\n ";
    };
    static void DrawTrace() {
        std::cout << "\ec\e[0m";
        draw_cadre(40, 4 + traced.size());
        for_each(traced.begin(), traced.end(), DrawTraced);
    };
    virtual void DrawValue() = 0;
private:
    char color;
    std::string name;
    std::weak_ptr<T> value;
    static std::list<std::unique_ptr<Tracer>> traced;
};

template <typename T>
class TraceValue : public Tracer<T>
{
    virtual void DrawValue() {
        std::cout << value;
    };
};
