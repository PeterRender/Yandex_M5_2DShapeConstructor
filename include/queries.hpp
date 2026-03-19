#pragma once
#include "geometry.hpp"
#include <algorithm>
#include <optional>
#include <variant>

namespace geometry::queries {

template <class... Ts>
struct Multilambda : Ts... {
    using Ts::operator()...;
};

struct DistanceVisitor {
    Point2D point;

    explicit DistanceVisitor(const Point2D &p) : point(p) {}

    double operator()(const Line &line) const {
        Point2D line_vec = line.End() - line.Start();
        Point2D point_vec = point - line.Start();

        double line_length_sq = line_vec.Dot(line_vec);
        if (line_length_sq == 0) {
            return point.DistanceTo(line.Start());
        }

        double t = std::clamp(point_vec.Dot(line_vec) / line_length_sq, 0.0, 1.0);
        Point2D projection = line.Start() + line_vec * t;

        return point.DistanceTo(projection);
    }

    double operator()(const Triangle &triangle) const {
        auto vertices = triangle.Vertices();
        double min_distance = std::numeric_limits<double>::max();

        for (size_t i = 0; i < vertices.size(); ++i) {
            Line edge{vertices[i], vertices[(i + 1) % vertices.size()]};
            min_distance = std::min(min_distance, (*this)(edge));
        }

        return min_distance;
    }

    double operator()(const Rectangle &rect) const {
        auto vertices = rect.Vertices();
        double min_distance = std::numeric_limits<double>::max();

        for (size_t i = 0; i < vertices.size(); ++i) {
            Line edge{vertices[i], vertices[(i + 1) % vertices.size()]};
            min_distance = std::min(min_distance, (*this)(edge));
        }

        return min_distance;
    }

    double operator()(const RegularPolygon &polygon) const {
        auto vertices = polygon.Vertices();
        double min_distance = std::numeric_limits<double>::max();

        for (size_t i = 0; i < vertices.size(); ++i) {
            Line edge{vertices[i], vertices[(i + 1) % vertices.size()]};
            min_distance = std::min(min_distance, (*this)(edge));
        }

        return min_distance;
    }

    double operator()(const Circle &circle) const {
        double center_distance = point.DistanceTo(circle.Center());
        return std::max(0.0, center_distance - circle.Radius());
    }

    double operator()(const Polygon &polygon) const {
        double min_distance = std::numeric_limits<double>::max();
        for (const auto &p : polygon.Vertices()) {
            min_distance = std::min(min_distance, point.DistanceTo(p));
        }
        return min_distance;
    }
};

struct PointToShapeDistanceVisitor {
    Point2D point;

    explicit PointToShapeDistanceVisitor(const Point2D &p) : point(p) {}

    double operator()(const Line &line) const {
        Point2D line_vec = line.End() - line.Start();
        Point2D point_vec = point - line.Start();

        double line_length_sq = line_vec.Dot(line_vec);
        if (line_length_sq == 0) {
            return point.DistanceTo(line.Start());
        }

        double t = std::clamp(point_vec.Dot(line_vec) / line_length_sq, 0.0, 1.0);
        Point2D projection = line.Start() + line_vec * t;

        return point.DistanceTo(projection);
    }

    double operator()(const Triangle &triangle) const {
        auto vertices = triangle.Vertices();
        double min_distance = std::numeric_limits<double>::max();

        for (size_t i = 0; i < vertices.size(); ++i) {
            Line edge{vertices[i], vertices[(i + 1) % vertices.size()]};
            min_distance = std::min(min_distance, (*this)(edge));
        }

        return min_distance;
    }

    double operator()(const Rectangle &rect) const {
        auto vertices = rect.Vertices();
        double min_distance = std::numeric_limits<double>::max();

        for (size_t i = 0; i < vertices.size(); ++i) {
            Line edge{vertices[i], vertices[(i + 1) % vertices.size()]};
            min_distance = std::min(min_distance, (*this)(edge));
        }

        return min_distance;
    }

    double operator()(const RegularPolygon &polygon) const {
        auto vertices = polygon.Vertices();
        double min_distance = std::numeric_limits<double>::max();

        for (size_t i = 0; i < vertices.size(); ++i) {
            Line edge{vertices[i], vertices[(i + 1) % vertices.size()]};
            min_distance = std::min(min_distance, (*this)(edge));
        }

        return min_distance;
    }

    double operator()(const Circle &circle) const {
        double center_distance = point.DistanceTo(circle.Center());
        return std::max(0.0, center_distance - circle.Radius());
    }

    double operator()(const Polygon &polygon) const {
        double min_distance = std::numeric_limits<double>::max();
        for (const auto &p : polygon.Vertices()) {
            min_distance = std::min(min_distance, point.DistanceTo(p));
        }
        return min_distance;
    }
};

struct PointInShapeVisitor {
    Point2D point;

    explicit PointInShapeVisitor(const Point2D &p) : point(p) {}

    bool operator()(const Line &line) const {
        Point2D line_vec = line.End() - line.Start();
        Point2D point_vec = point - line.Start();

        double cross = point_vec.Cross(line_vec);
        if (std::abs(cross) > 1e-10) {
            return false;
        }

        double dot = point_vec.Dot(line_vec);
        double line_length_sq = line_vec.Dot(line_vec);

        return dot >= 0 && dot <= line_length_sq;
    }

    bool operator()(const Triangle &triangle) const {
        Point2D a = triangle.A();
        Point2D b = triangle.B();
        Point2D c = triangle.C();

        double sign1 = (point - a).Cross(b - a);
        double sign2 = (point - b).Cross(c - b);
        double sign3 = (point - c).Cross(a - c);

        bool has_neg = (sign1 < 0) || (sign2 < 0) || (sign3 < 0);
        bool has_pos = (sign1 > 0) || (sign2 > 0) || (sign3 > 0);

        return !(has_neg && has_pos);
    }

    bool operator()(const Rectangle &rect) const {
        return point.X() >= rect.BottomLeft().X() && point.X() <= rect.BottomLeft().X() + rect.Width() &&
               point.Y() >= rect.BottomLeft().Y() && point.Y() <= rect.BottomLeft().Y() + rect.Height();
    }

    bool operator()(const RegularPolygon &polygon) const {
        std::vector<Point2D> vertices = polygon.Vertices();
        return point_in_polygon_ray_casting(point, vertices);
    }

    bool operator()(const Circle &circle) const { return point.DistanceTo(circle.Center()) <= circle.Radius(); }

private:
    bool point_in_polygon_ray_casting(const Point2D &p, const std::vector<Point2D> &vertices) const {
        int intersections = 0;
        size_t n = vertices.size();

        for (size_t i = 0; i < n; ++i) {
            Point2D v1 = vertices[i];
            Point2D v2 = vertices[(i + 1) % n];

            if (((v1.Y() > p.Y()) != (v2.Y() > p.Y())) &&
                (p.X() < (v2.X() - v1.X()) * (p.Y() - v1.Y()) / (v2.Y() - v1.Y()) + v1.X())) {
                intersections++;
            }
        }

        return (intersections % 2) == 1;
    }
};

struct ShapeToShapeDistanceVisitor {
    std::optional<double> operator()(const Circle &c1, const Circle &c2) const {
        double centerDistance = c1.Center().DistanceTo(c2.Center());
        return std::max(0.0, centerDistance - c1.Radius() - c2.Radius());
    }

    std::optional<double> operator()(const Line &l1, const Line &l2) const {
        std::vector<double> distances = {
            queries::DistanceVisitor{l1.Start()}(l2), queries::DistanceVisitor{l1.End()}(l2),
            queries::DistanceVisitor{l2.Start()}(l1), queries::DistanceVisitor{l2.End()}(l1)};
        return *std::ranges::min_element(distances);
    }

    // fallback for all unsupported combinations
    template <typename T, typename U>
    std::optional<double> operator()(const T &, const U &) const {
        return std::nullopt;
    }
};

/*
 * Функции-помощники
 */
inline double DistanceToPoint(const Shape &shape, const Point2D &point) {

    /* ваш код с PointToShapeDistanceVisitor здесь*/
    return 0.0;
}

inline BoundingBox GetBoundBox(const Shape &shape) {

    /* ваш код с использованием метода BoundBox() здесь */
    return {};
}

inline double GetHeight(const Shape &shape) {

    /* ваш код с использованием метода Height() здесь */
    return 0.0;
}

inline bool BoundingBoxesOverlap(const Shape &shape1, const Shape &shape2) {
    BoundingBox bb1 = GetBoundBox(shape1);
    BoundingBox bb2 = GetBoundBox(shape2);
    return bb1.Overlaps(bb2);
}

std::optional<double> DistanceBetweenShapes(const Shape &shape1, const Shape &shape2) {

    /* ваш код с ShapeToShapeDistanceVisitor здесь*/
    return std::nullopt;
}

}  // namespace geometry::queries