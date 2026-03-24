#include "triangulation.hpp"
#include <gtest/gtest.h>

#include <print>

using namespace geometry;

// Тест триангуляции Делоне для треугольника
TEST(TriangulationTest, ThreePoints) {
    std::vector<Point2D> points = {{0, 0}, {5, 0}, {2.5, 5}};
    auto result = triangulation::DelaunayTriangulation(points);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), 1);
}

// Тест триангуляции Делоне для квадрата
TEST(TriangulationTest, Square) {
    std::vector<Point2D> points = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
    auto result = triangulation::DelaunayTriangulation(points);

    ASSERT_TRUE(result.has_value());
    // Квадрат разбивается на 2 треугольника
    EXPECT_EQ(result->size(), 2);
}

// Тест триангуляции Делоне для правильного многоугольника
TEST(TriangulationTest, RegularPolygon) {
    RegularPolygon polygon(Point2D(0, 0), 10, 5);  // пятиугольник
    auto vertices = polygon.Vertices();
    auto result = triangulation::DelaunayTriangulation(vertices);

    ASSERT_TRUE(result.has_value());
    // Для 5 точек правильного многоугольника должно быть 3 треугольника
    EXPECT_EQ(result->size(), 3);
}

// Тест триангуляции Делоне для точек на окружности
TEST(TriangulationTest, PointsOnCircle) {
    Circle circle(Point2D(0, 0), 10);
    auto vertices = circle.Vertices(6);  // 6 точек на окружности
    auto result = triangulation::DelaunayTriangulation(vertices);

    ASSERT_TRUE(result.has_value());
    // Для 6 точек на окружности должно быть 4 треугольника
    EXPECT_EQ(result->size(), 4);
}

// Тест недостаточного количества точек (< 3)
TEST(TriangulationTest, NotEnoughPoints) {
    std::vector<Point2D> points = {{0, 0}, {1, 0}};
    auto result = triangulation::DelaunayTriangulation(points);

    EXPECT_FALSE(result.has_value());
}

// Тест пустого списка точек
TEST(TriangulationTest, EmptyPoints) {
    std::vector<Point2D> points;
    auto result = triangulation::DelaunayTriangulation(points);

    EXPECT_FALSE(result.has_value());
}

// Тест коллинеарных точек (нельзя построить триангуляцию, возвращается пустой вектор)
TEST(TriangulationTest, CollinearPoints) {
    std::vector<Point2D> points = {Point2D(0, 0), Point2D(1, 0), Point2D(2, 0), Point2D(3, 0)};
    auto result = triangulation::DelaunayTriangulation(points);

    ASSERT_TRUE(result.has_value());  // алгоритм отработал без ошибок
    EXPECT_TRUE(result->empty());     // но треугольников нет (точки коллинеарны)
}
