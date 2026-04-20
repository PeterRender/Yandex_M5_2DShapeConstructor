#include "intersections.hpp"
#include <gtest/gtest.h>

using namespace geometry;

// Тест пересечения двух отрезков, пересекающихся в центре
TEST(IntersectionsTest, LineLineIntersect) {
    Line l1(Point2D(0, 0), Point2D(10, 10));
    Line l2(Point2D(0, 10), Point2D(10, 0));

    auto point = intersections::GetIntersectPoint(l1, l2);
    ASSERT_TRUE(point.has_value());
    EXPECT_NEAR(point->X(), 5.0, EPS);
    EXPECT_NEAR(point->Y(), 5.0, EPS);
}

// Тест параллельных отрезков, не имеющих пересечения
TEST(IntersectionsTest, LineLineNoIntersect) {
    Line l1(Point2D(0, 0), Point2D(10, 0));
    Line l2(Point2D(0, 10), Point2D(10, 10));

    auto point = intersections::GetIntersectPoint(l1, l2);
    EXPECT_FALSE(point.has_value());
}

// Тест пересечения отрезка с окружностью (должно быть 2 точки, возвращаем первую по лучу)
TEST(IntersectionsTest, LineCircleIntersect) {
    Line line(Point2D(-5, 0), Point2D(5, 0));
    Circle circle(Point2D(0, 0), 3);

    auto point = intersections::GetIntersectPoint(line, circle);
    ASSERT_TRUE(point.has_value());
    // Первая точка пересечения по направлению от start к end
    EXPECT_NEAR(point->X(), -3.0, EPS);
    EXPECT_NEAR(point->Y(), 0.0, EPS);
}

// Тест пересечения двух окружностей (должно быть 2 точки, возвращаем первую)
TEST(IntersectionsTest, CircleCircleIntersect) {
    Circle c1(Point2D(0, 0), 5);
    Circle c2(Point2D(8, 0), 5);

    auto point = intersections::GetIntersectPoint(c1, c2);
    ASSERT_TRUE(point.has_value());
    EXPECT_NEAR(point->X(), 4.0, EPS);
    EXPECT_NEAR(point->Y(), 3.0, EPS);
}
