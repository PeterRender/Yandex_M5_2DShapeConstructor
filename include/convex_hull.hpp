#pragma once
#include "geometry.hpp"
#include <algorithm>
#include <ranges>
#include <stack>
#include <vector>

namespace geometry::convex_hull {

double CrossProduct(Point2D p1, Point2D middle, Point2D p2);

class StackForGrahamScan {
public:
    void Push(const Point2D &p) { s.push_back(p); }
    void Pop() { s.pop_back(); }

    size_t Size() { return s.size(); }
    Point2D Top() { return s.back(); }
    Point2D NextToTop() { return *std::prev(s.end(), 2); }

    std::vector<Point2D> &&Extract() & { return std::move(s); }

private:
    std::vector<Point2D> s;
};

//Ваш код здесь
std::vector<Point2D> GrahamScan(std::span<Point2D> points);

}  // namespace geometry::convex_hull