#include "convex_hull.hpp"
#include <algorithm>
#include <stdexcept>

namespace geometry::convex_hull {

double CrossProduct(Point2D p1, Point2D middle, Point2D p2) {
    auto new_p1 = p1 - middle;
    auto new_p2 = p2 - middle;
    return new_p1.Cross(new_p2);
}

std::vector<Point2D> GrahamScan(std::span<Point2D> points) {
    if (points.size() < 3) {
        throw std::logic_error("At least three points are required for convex hull.");
    }

    auto smallest = *std::min_element(points.begin(), points.end());

    std::sort(points.begin() + 1, points.end(), [&smallest](const Point2D &p1, const Point2D &p2) {
        static const auto precision = 1e-10;

        double cross = CrossProduct(p1, smallest, p2);
        if (std::abs(cross) < precision) {
            return smallest.DistanceTo(p1) < smallest.DistanceTo(p2);
        }
        return cross > 0;
    });

    StackForGrahamScan hull;
    for (const auto &new_p : points) {
        while (hull.Size() > 1 && CrossProduct(hull.NextToTop(), hull.Top(), new_p) > 0.0) {
            hull.Pop();
        }
        hull.Push(new_p);
    }

    return std::vector{hull.Extract()};}

}  // namespace geometry::convex_hull