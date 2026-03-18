#pragma once
#include "geometry.hpp"
#include <algorithm>
#include <format>
#include <set>
#include <stdexcept>
#include <vector>

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
        double d = 2 * (a.X() * (b.Y() - c.Y()) + b.X() * (c.Y() - a.Y()) + c.X() * (a.Y() - b.Y()));
        if (std::abs(d) < 1e-10) {
            return {(a.X() + b.X() + c.X()) / 3, (a.Y() + b.Y() + c.Y()) / 3};
        }

        double ux =
            ((a.X() * a.X() + a.Y() * a.Y()) * (b.Y() - c.Y()) + (b.X() * b.X() + b.Y() * b.Y()) * (c.Y() - a.Y()) +
             (c.X() * c.X() + c.Y() * c.Y()) * (a.Y() - b.Y())) /
            d;

        double uy =
            ((a.X() * a.X() + a.Y() * a.Y()) * (c.X() - b.X()) + (b.X() * b.X() + b.Y() * b.Y()) * (a.X() - c.X()) +
             (c.X() * c.X() + c.Y() * c.Y()) * (b.X() - a.X())) /
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
                if (std::abs(p1.X() - p2.X()) < 1e-10 && std::abs(p1.Y() - p2.Y()) < 1e-10) {
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
        if (p1.X() > p2.X() || (p1.X() == p2.X() && p1.Y() > p2.Y())) {
            std::swap(this->p1, this->p2);
        }
    }

    bool operator<(const Edge &other) const {
        if (std::abs(p1.X() - other.p1.X()) > 1e-10)
            return p1.X() < other.p1.X();
        if (std::abs(p1.Y() - other.p1.Y()) > 1e-10)
            return p1.Y() < other.p1.Y();
        if (std::abs(p2.X() - other.p2.X()) > 1e-10)
            return p2.X() < other.p2.X();
        return p2.Y() < other.p2.Y();
    }

    bool operator==(const Edge &other) const {
        return std::abs(p1.X() - other.p1.X()) < 1e-10 && std::abs(p1.Y() - other.p1.Y()) < 1e-10 &&
               std::abs(p2.X() - other.p2.X()) < 1e-10 && std::abs(p2.Y() - other.p2.Y()) < 1e-10;
    }
};

// Ваш код здесь
inline std::vector<DelaunayTriangle> DelaunayTriangulation(std::span<const Point2D> points) {
    if (points.size() < 3) {
        throw std::logic_error("At least three points are required for triangulation.");
    }

    auto [minX, maxX] = std::minmax_element(points.begin(), points.end(),
                                            [](const Point2D &a, const Point2D &b) { return a.X() < b.X(); });
    auto [minY, maxY] = std::minmax_element(points.begin(), points.end(),
                                            [](const Point2D &a, const Point2D &b) { return a.Y() < b.Y(); });

    double dx = maxX->X() - minX->X();
    double dy = maxY->Y() - minY->Y();
    double dmax = std::max(dx, dy);
    Point2D center = {(minX->X() + maxX->X()) / 2, (minY->Y() + maxY->Y()) / 2};

    Point2D super1 = {center.X() - 20 * dmax, center.Y() - dmax};
    Point2D super2 = {center.X(), center.Y() + 20 * dmax};
    Point2D super3 = {center.X() + 20 * dmax, center.Y() - dmax};

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
            return std::find_if(bad_triangles.begin(), bad_triangles.end(), [&t](const DelaunayTriangle &bad) {
                       return std::abs(t.a.X() - bad.a.X()) < 1e-10 && std::abs(t.a.Y() - bad.a.Y()) < 1e-10 &&
                              std::abs(t.b.X() - bad.b.X()) < 1e-10 && std::abs(t.b.Y() - bad.b.Y()) < 1e-10 &&
                              std::abs(t.c.X() - bad.c.X()) < 1e-10 && std::abs(t.c.Y() - bad.c.Y()) < 1e-10;
                   }) != bad_triangles.end();
        });

        for (const Edge &edge : polygon) {
            triangles.emplace_back(edge.p1, edge.p2, point);
        }
    }
    std::erase_if(triangles, [&super1, &super2, &super3](const DelaunayTriangle &t) {
        return (std::abs(t.a.X() - super1.X()) < 1e-10 && std::abs(t.a.Y() - super1.Y()) < 1e-10) ||
               (std::abs(t.a.X() - super2.X()) < 1e-10 && std::abs(t.a.Y() - super2.Y()) < 1e-10) ||
               (std::abs(t.a.X() - super3.X()) < 1e-10 && std::abs(t.a.Y() - super3.Y()) < 1e-10) ||
               (std::abs(t.b.X() - super1.X()) < 1e-10 && std::abs(t.b.Y() - super1.Y()) < 1e-10) ||
               (std::abs(t.b.X() - super2.X()) < 1e-10 && std::abs(t.b.Y() - super2.Y()) < 1e-10) ||
               (std::abs(t.b.X() - super3.X()) < 1e-10 && std::abs(t.b.Y() - super3.Y()) < 1e-10) ||
               (std::abs(t.c.X() - super1.X()) < 1e-10 && std::abs(t.c.Y() - super1.Y()) < 1e-10) ||
               (std::abs(t.c.X() - super2.X()) < 1e-10 && std::abs(t.c.Y() - super2.Y()) < 1e-10) ||
               (std::abs(t.c.X() - super3.X()) < 1e-10 && std::abs(t.c.Y() - super3.Y()) < 1e-10);
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
