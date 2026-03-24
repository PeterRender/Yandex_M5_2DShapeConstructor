#include "queries.hpp"
#include <gtest/gtest.h>

using namespace geometry;

// Тест расстояния от точки до отрезка (точка над отрезком)
TEST(QueriesTest, DistanceToPointLine) {
    Line line(Point2D(0, 0), Point2D(10, 0));
    Point2D p(5, 3);

    double dist = queries::DistanceToPoint(line, p);
    EXPECT_DOUBLE_EQ(dist, 3.0);
}

// Тест расстояния от точки до окружности (точка снаружи)
TEST(QueriesTest, DistanceToPointCircle) {
    Circle circle(Point2D(0, 0), 5);
    Point2D p(8, 0);

    double dist = queries::DistanceToPoint(circle, p);
    EXPECT_DOUBLE_EQ(dist, 3.0);
}

// Тест принадлежности точки окружности (точка внутри)
TEST(QueriesTest, PointInCircle) {
    Circle circle(Point2D(0, 0), 5);
    Point2D p(3, 4);

    // Оборачиваем фигуру в Shape (variant)
    Shape shape = circle;
    bool inside = std::visit(queries::PointInShapeVisitor(p), shape);
    EXPECT_TRUE(inside);
}

// Тест принадлежности точки треугольнику (точка внутри)
TEST(QueriesTest, PointInTriangle) {
    Triangle tri(Point2D(0, 0), Point2D(4, 0), Point2D(0, 4));
    Point2D p(1, 1);

    Shape shape = tri;
    bool inside = std::visit(queries::PointInShapeVisitor(p), shape);
    EXPECT_TRUE(inside);
}

// Тест принадлежности точки треугольнику (точка снаружи)
TEST(QueriesTest, PointOutsideTriangle) {
    Triangle tri(Point2D(0, 0), Point2D(4, 0), Point2D(0, 4));
    Point2D p(5, 5);

    Shape shape = tri;
    bool inside = std::visit(queries::PointInShapeVisitor(p), shape);
    EXPECT_FALSE(inside);
}

// Тест получения ограничивающего бокса фигуры
TEST(QueriesTest, GetBoundBox) {
    Circle circle(Point2D(2, 3), 4);
    Shape shape = circle;
    BoundingBox box = queries::GetBoundBox(shape);
    EXPECT_DOUBLE_EQ(box.Width(), 8.0);
    EXPECT_DOUBLE_EQ(box.Height(), 8.0);
}

// Тест получения высоты фигуры
TEST(QueriesTest, GetHeight) {
    Circle circle(Point2D(2, 3), 4);
    Shape shape = circle;
    double height = queries::GetHeight(shape);
    EXPECT_DOUBLE_EQ(height, 7.0);
}

// Тест пересечения ограничивающих боксов
TEST(QueriesTest, BoundingBoxesOverlap) {
    Circle c1(Point2D(0, 0), 5);
    Circle c2(Point2D(8, 0), 5);
    Shape shape1 = c1;
    Shape shape2 = c2;

    bool overlap = queries::BoundingBoxesOverlap(shape1, shape2);
    EXPECT_TRUE(overlap);  // боксы пересекаются
}

// Тест расстояния между двумя окружностями
TEST(QueriesTest, DistanceBetweenCircles) {
    Circle c1(Point2D(0, 0), 5);
    Circle c2(Point2D(15, 0), 5);
    Shape shape1 = c1;
    Shape shape2 = c2;

    auto dist = queries::DistanceBetweenShapes(shape1, shape2);
    ASSERT_TRUE(dist.has_value());
    EXPECT_DOUBLE_EQ(dist.value(), 5.0);  // 15 - 5 - 5 = 5
}
