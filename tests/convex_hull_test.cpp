#include "convex_hull.hpp"
#include <gtest/gtest.h>

using namespace geometry;

// Тест построения выпуклой оболочки для квадрата
TEST(ConvexHullTest, SimpleSquare) {
    std::vector<Point2D> points = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};

    auto result = convex_hull::GrahamScan(points);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), 4);
}

// Тест построения выпуклой оболочки для треугольника
TEST(ConvexHullTest, Triangle) {
    std::vector<Point2D> points = {{0, 0}, {5, 0}, {2.5, 5}};

    auto result = convex_hull::GrahamScan(points);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), 3);
}

// Тест построения выпуклой оболочки для точек на окружности
TEST(ConvexHullTest, PointsOnCircle) {
    Circle circle(Point2D(0, 0), 5);
    const int NUM_VERTICES = 8;
    auto vertices = circle.Vertices(NUM_VERTICES);  // NUM_VERTICES точек на окружности
    auto result = convex_hull::GrahamScan(vertices);

    ASSERT_TRUE(result.has_value());
    // Все NUM_VERTICES точек должны быть в оболочке
    EXPECT_EQ(result->size(), NUM_VERTICES);
}

// Тест построения выпуклой оболочки для правильного многоугольника
TEST(ConvexHullTest, RegularPolygon) {
    const int NUM_VERTICES = 6;  // шестиугольник
    RegularPolygon polygon(Point2D(0, 0), 5, NUM_VERTICES);
    auto vertices = polygon.Vertices();
    auto result = convex_hull::GrahamScan(vertices);

    ASSERT_TRUE(result.has_value());
    // Все NUM_VERTICES вершин уже образуют выпуклую оболочку
    EXPECT_EQ(result->size(), NUM_VERTICES);
}

// Тест недостаточного количества точек (< 3)
TEST(ConvexHullTest, NotEnoughPoints) {
    std::vector<Point2D> points = {{0, 0}, {1, 0}};
    auto result = convex_hull::GrahamScan(points);

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "At least three points are required for convex hull.");
}

// Тест пустого списка точек
TEST(ConvexHullTest, EmptyPoints) {
    std::vector<Point2D> points;

    auto result = convex_hull::GrahamScan(points);
    EXPECT_FALSE(result.has_value());
}

// Тест коллинеарных точек (все на одной линии)
TEST(ConvexHullTest, CollinearPoints) {
    // Создаем множество коллинеарных точек
    std::vector<Point2D> points = {Point2D(0, 0), Point2D(1, 0), Point2D(2, 0), Point2D(3, 0), Point2D(4, 0)};

    auto result = convex_hull::GrahamScan(points);
    ASSERT_TRUE(result.has_value());
    // Для коллинеарных точек оболочка состоит из двух крайних точек
    EXPECT_EQ(result->size(), 2);
}
