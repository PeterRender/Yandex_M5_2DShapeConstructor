#include "geometry.hpp"
#include <gtest/gtest.h>

using namespace geometry;

// Тест создания точки по умолчанию и с параметрами
TEST(Point2DTest, Construction) {
    Point2D p1;
    EXPECT_DOUBLE_EQ(p1.X(), 0.0);
    EXPECT_DOUBLE_EQ(p1.Y(), 0.0);

    Point2D p2(3.0, 4.0);
    EXPECT_DOUBLE_EQ(p2.X(), 3.0);
    EXPECT_DOUBLE_EQ(p2.Y(), 4.0);
}

// Тест вычисления евклидова расстояния между двумя точками
TEST(Point2DTest, DistanceTo) {
    Point2D p1(0, 0);
    Point2D p2(3, 4);
    EXPECT_DOUBLE_EQ(p1.DistanceTo(p2), 5.0);
}

// Тест скалярного произведения двух векторов
TEST(Point2DTest, Dot) {
    Point2D p1(1, 2);
    Point2D p2(3, 4);
    EXPECT_DOUBLE_EQ(p1.Dot(p2), 11.0);
}

// Тест векторного произведения (площадь параллелограмма)
TEST(Point2DTest, Cross) {
    Point2D p1(1, 2);
    Point2D p2(3, 4);
    EXPECT_DOUBLE_EQ(p1.Cross(p2), -2.0);
}

// Тест ограничивающего бокса отрезка
TEST(LineTest, BoundBox) {
    Line line(Point2D(0, 0), Point2D(5, 10));
    BoundingBox box = line.BoundBox();
    EXPECT_DOUBLE_EQ(box.Width(), 5.0);
    EXPECT_DOUBLE_EQ(box.Height(), 10.0);
}

// Тест получения максимальной Y-координаты отрезка
TEST(LineTest, MaxY) {
    Line line(Point2D(0, 0), Point2D(5, 10));
    EXPECT_DOUBLE_EQ(line.MaxY(), 10.0);
}

// Тест вычисления площади треугольника
TEST(TriangleTest, Area) {
    Triangle tri(Point2D(0, 0), Point2D(4, 0), Point2D(0, 3));
    EXPECT_DOUBLE_EQ(tri.Area(), 6.0);
}

// Тест ограничивающего бокса прямоугольника
TEST(RectangleTest, BoundBox) {
    Rectangle rect(Point2D(0, 0), 5, 10);
    BoundingBox box = rect.BoundBox();
    EXPECT_DOUBLE_EQ(box.Width(), 5.0);
    EXPECT_DOUBLE_EQ(box.Height(), 10.0);
}

// Тест ограничивающего бокса окружности
TEST(CircleTest, BoundBox) {
    Circle circle(Point2D(2, 3), 4);
    BoundingBox box = circle.BoundBox();
    Point2D botLeft = box.BottomLeft();
    Point2D topRight = box.TopRight();
    EXPECT_DOUBLE_EQ(botLeft.X(), -2.0);
    EXPECT_DOUBLE_EQ(botLeft.Y(), -1.0);
    EXPECT_DOUBLE_EQ(topRight.X(), 6.0);
    EXPECT_DOUBLE_EQ(topRight.Y(), 7.0);
}

// Тест количества вершин правильного многоугольника
TEST(RegularPolygonTest, VerticesCount) {
    RegularPolygon poly(Point2D(0, 0), 5, 6);
    auto vertices = poly.Vertices();
    EXPECT_EQ(vertices.size(), 6);
}
