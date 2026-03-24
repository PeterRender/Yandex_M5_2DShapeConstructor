#include "shape_utils.hpp"
#include <gtest/gtest.h>

using namespace geometry;

// Параметрический тест фабричных функций фигур
// Параметры: тип фигуры, вектор параметров, ожидаемый успех
class ShapeFactoryTest : public ::testing::TestWithParam<std::tuple<std::string, std::vector<double>, bool>> {};

TEST_P(ShapeFactoryTest, CreateShape) {
    auto [type, params, should_succeed] = GetParam();
    std::optional<Shape> result;

    if (type == "circle") {
        result = utils::MakeCircle(params);
    } else if (type == "line") {
        result = utils::MakeLine(params);
    } else if (type == "triangle") {
        result = utils::MakeTriangle(params);
    } else if (type == "rectangle") {
        result = utils::MakeRectangle(params);
    } else if (type == "polygon") {
        result = utils::MakePolygon(params);
    }

    EXPECT_EQ(result.has_value(), should_succeed);

    // Дополнительная проверка для успешных случаев
    if (should_succeed && result.has_value()) {
        if (type == "circle") {
            auto circle = std::get<Circle>(*result);
            EXPECT_DOUBLE_EQ(circle.Radius(), params[2]);
        } else if (type == "line") {
            auto line = std::get<Line>(*result);
            EXPECT_DOUBLE_EQ(line.Length(), std::hypot(params[2] - params[0], params[3] - params[1]));
        } else if (type == "rectangle") {
            auto rect = std::get<Rectangle>(*result);
            EXPECT_DOUBLE_EQ(rect.Width(), params[2]);
            EXPECT_DOUBLE_EQ(rect.Height(), params[3]);
        } else if (type == "polygon") {
            auto poly = std::get<RegularPolygon>(*result);
            EXPECT_DOUBLE_EQ(poly.Radius(), params[2]);
            EXPECT_EQ(poly.Sides(), static_cast<int>(params[3]));
        }
    }
}

// Набор тестов для фабричных функций
INSTANTIATE_TEST_SUITE_P(ShapeFactoryTestSuite, ShapeFactoryTest,
                         ::testing::Values(
                             // Круг: центр (0,0), радиус 5 - успех
                             std::make_tuple("circle", std::vector<double>{0, 0, 5}, true),
                             // Круг: отрицательный радиус - ошибка
                             std::make_tuple("circle", std::vector<double>{0, 0, -5}, false),
                             // Круг: недостаточно параметров - ошибка
                             std::make_tuple("circle", std::vector<double>{0, 0}, false),

                             // Линия: (0,0) - (3,4) - успех
                             std::make_tuple("line", std::vector<double>{0, 0, 3, 4}, true),
                             // Линия: недостаточно параметров - ошибка
                             std::make_tuple("line", std::vector<double>{0, 0, 3}, false),

                             // Треугольник: (0,0), (1,0), (0,1) - успех
                             std::make_tuple("triangle", std::vector<double>{0, 0, 1, 0, 0, 1}, true),
                             // Треугольник: недостаточно параметров - ошибка
                             std::make_tuple("triangle", std::vector<double>{0, 0, 1, 0, 0}, false),

                             // Прямоугольник: (0,0), ширина 5, высота 10 - успех
                             std::make_tuple("rectangle", std::vector<double>{0, 0, 5, 10}, true),
                             // Прямоугольник: отрицательная ширина - ошибка
                             std::make_tuple("rectangle", std::vector<double>{0, 0, -5, 10}, false),
                             // Прямоугольник: отрицательная высота - ошибка
                             std::make_tuple("rectangle", std::vector<double>{0, 0, 5, -10}, false),

                             // Правильный многоугольник: центр (0,0), радиус 5, 6 сторон - успех
                             std::make_tuple("polygon", std::vector<double>{0, 0, 5, 6}, true),
                             // Многоугольник: отрицательный радиус - ошибка
                             std::make_tuple("polygon", std::vector<double>{0, 0, -5, 6}, false),
                             // Многоугольник: количество сторон < 3 - ошибка
                             std::make_tuple("polygon", std::vector<double>{0, 0, 5, 2}, false),
                             // Многоугольник: количество сторон не целое - ошибка
                             std::make_tuple("polygon", std::vector<double>{0, 0, 5, 3.5}, false)));

// Тест парсинга строки с фигурами
TEST(ShapeUtilsTest, ParseShapes) {
    auto shapes = utils::ParseShapes("circle 0 0 1; line 0 0 1 1; triangle 0 0 1 0 0 1");
    EXPECT_EQ(shapes.size(), 3);
}

// Тест парсинга строки с некорректными фигурами (должны быть отфильтрованы)
TEST(ShapeUtilsTest, ParseShapesWithInvalid) {
    auto shapes = utils::ParseShapes("circle 0 0 1; badshape; line 0 0 1 1; circle 0 0 -1");
    EXPECT_EQ(shapes.size(), 2);  // только circle и line
}

// Тест парсинга пустой строки
TEST(ShapeUtilsTest, ParseEmptyString) {
    auto shapes = utils::ParseShapes("");
    EXPECT_TRUE(shapes.empty());
}
