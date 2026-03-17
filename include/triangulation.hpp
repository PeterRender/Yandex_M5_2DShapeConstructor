#pragma once
#include "geometry.hpp"
#include <algorithm>
#include <format>
#include <set>
#include <vector>
#include <stdexcept>

namespace geometry::triangulation {

struct DelaunayTriangle {
    Point2D a, b, c;

    DelaunayTriangle(Point2D a, Point2D b, Point2D c) : a(a), b(b), c(c) {}

    bool ContainsPoint(const Point2D &p) const {
        Point2D center = Circumcenter();
        double radius = Circumradius();
        return center.DistanceTo(p) <= radius + 1e-10;
    }

    Point2D Circumcenter() const {
        double d = 2 * (a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y));
        if (std::abs(d) < 1e-10) {
            return {(a.x + b.x + c.x) / 3, (a.y + b.y + c.y) / 3};
        }

        double ux = ((a.x * a.x + a.y * a.y) * (b.y - c.y) + (b.x * b.x + b.y * b.y) * (c.y - a.y) +
                     (c.x * c.x + c.y * c.y) * (a.y - b.y)) /
                    d;

        double uy = ((a.x * a.x + a.y * a.y) * (c.x - b.x) + (b.x * b.x + b.y * b.y) * (a.x - c.x) +
                     (c.x * c.x + c.y * c.y) * (b.x - a.x)) /
                    d;

        return {ux, uy};
    }

    double Circumradius() const {
        Point2D center = Circumcenter();
        return center.DistanceTo(a);
    }

    bool SharesEdge(const DelaunayTriangle &other) const {
        std::vector<Point2D> this_points = {a, b, c};
        std::vector<Point2D> other_points = {other.a, other.b, other.c};

        int shared_count = 0;
        for (const Point2D &p1 : this_points) {
            for (const Point2D &p2 : other_points) {
                if (std::abs(p1.x - p2.x) < 1e-10 && std::abs(p1.y - p2.y) < 1e-10) {
                    shared_count++;
                    break;
                }
            }
        }

        return shared_count == 2;
    }

    std::vector<Point2D> vertices() const { return {a, b, c}; }
};

struct Edge {
    Point2D p1, p2;

    Edge(Point2D p1, Point2D p2) : p1(p1), p2(p2) {
        if (p1.x > p2.x || (p1.x == p2.x && p1.y > p2.y)) {
            std::swap(this->p1, this->p2);
        }
    }

    bool operator<(const Edge &other) const {
        if (std::abs(p1.x - other.p1.x) > 1e-10)
            return p1.x < other.p1.x;
        if (std::abs(p1.y - other.p1.y) > 1e-10)
            return p1.y < other.p1.y;
        if (std::abs(p2.x - other.p2.x) > 1e-10)
            return p2.x < other.p2.x;
        return p2.y < other.p2.y;
    }

    bool operator==(const Edge &other) const {
        return std::abs(p1.x - other.p1.x) < 1e-10 && std::abs(p1.y - other.p1.y) < 1e-10 &&
               std::abs(p2.x - other.p2.x) < 1e-10 && std::abs(p2.y - other.p2.y) < 1e-10;
    }
};

//Ваш код здесь
inline std::vector<DelaunayTriangle> DelaunayTriangulation(std::span<const Point2D> points) {
   if (points.size() < 3) {
        throw std::logic_error("At least three points are required for triangulation.");
    }

    auto [minX, maxX] =
        std::minmax_element(points.begin(), points.end(), [](const Point2D &a, const Point2D &b) { return a.x < b.x; });
    auto [minY, maxY] =
        std::minmax_element(points.begin(), points.end(), [](const Point2D &a, const Point2D &b) { return a.y < b.y; });

    double dx = maxX->x - minX->x;
    double dy = maxY->y - minY->y;
    double dmax = std::max(dx, dy);
    Point2D center = {(minX->x + maxX->x) / 2, (minY->y + maxY->y) / 2};

    Point2D super1 = {center.x - 20 * dmax, center.y - dmax};
    Point2D super2 = {center.x, center.y + 20 * dmax};
    Point2D super3 = {center.x + 20 * dmax, center.y - dmax};

    std::vector<DelaunayTriangle> triangles;
    triangles.emplace_back(super1, super2, super3);

    for (const Point2D &point : points) {
        std::vector<DelaunayTriangle> bad_triangles;
        std::set<Edge> polygon;

        for (const auto &triangle : triangles) {
            if (triangle.ContainsPoint(point)) {
                bad_triangles.push_back(triangle);

                Edge e1{triangle.a, triangle.b};
                Edge e2{triangle.b, triangle.c};
                Edge e3{triangle.c, triangle.a};

                if (!polygon.erase(e1))
                    polygon.insert(e1);
                if (!polygon.erase(e2))
                    polygon.insert(e2);
                if (!polygon.erase(e3))
                    polygon.insert(e3);
            }
        }

        std::erase_if(triangles, [&bad_triangles](const DelaunayTriangle &t) {
                                           return std::find_if(bad_triangles.begin(), bad_triangles.end(),
                                                               [&t](const DelaunayTriangle &bad) {
                                                                   return std::abs(t.a.x - bad.a.x) < 1e-10 &&
                                                                          std::abs(t.a.y - bad.a.y) < 1e-10 &&
                                                                          std::abs(t.b.x - bad.b.x) < 1e-10 &&
                                                                          std::abs(t.b.y - bad.b.y) < 1e-10 &&
                                                                          std::abs(t.c.x - bad.c.x) < 1e-10 &&
                                                                          std::abs(t.c.y - bad.c.y) < 1e-10;
                                                               }) != bad_triangles.end();
                                       });

        for (const Edge &edge : polygon) {
            triangles.emplace_back(edge.p1, edge.p2, point);
        }
    }
    std::erase_if(triangles, [&super1, &super2, &super3](const DelaunayTriangle &t) {
                           return (std::abs(t.a.x - super1.x) < 1e-10 && std::abs(t.a.y - super1.y) < 1e-10) ||
                                  (std::abs(t.a.x - super2.x) < 1e-10 && std::abs(t.a.y - super2.y) < 1e-10) ||
                                  (std::abs(t.a.x - super3.x) < 1e-10 && std::abs(t.a.y - super3.y) < 1e-10) ||
                                  (std::abs(t.b.x - super1.x) < 1e-10 && std::abs(t.b.y - super1.y) < 1e-10) ||
                                  (std::abs(t.b.x - super2.x) < 1e-10 && std::abs(t.b.y - super2.y) < 1e-10) ||
                                  (std::abs(t.b.x - super3.x) < 1e-10 && std::abs(t.b.y - super3.y) < 1e-10) ||
                                  (std::abs(t.c.x - super1.x) < 1e-10 && std::abs(t.c.y - super1.y) < 1e-10) ||
                                  (std::abs(t.c.x - super2.x) < 1e-10 && std::abs(t.c.y - super2.y) < 1e-10) ||
                                  (std::abs(t.c.x - super3.x) < 1e-10 && std::abs(t.c.y - super3.y) < 1e-10);
                       });

    return triangles;
}
}  // namespace geometry::triangulation

template <>
struct std::formatter<geometry::triangulation::DelaunayTriangle> {
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::triangulation::DelaunayTriangle &t, FormatContext &ctx) const {
        return std::format_to(ctx.out(), "DelaunayTriangle({}, {}, {})", t.a, t.b, t.c);
    }
};
