#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <format>
#include <initializer_list>
#include <numbers>
#include <ranges>
#include <variant>
#include <vector>

namespace geometry {

/*
 * Добавьте к методам класса Point2D и Lines2DDyn все необходимые аттрибуты и спецификаторы
 * Важно: Возвращаемый тип и принимаемые аргументы менять не нужно
 */

// В класс 2D-точки добавлены следующие атрибуты и спецификаторы:
// - [[nodiscard]] - метод возвращает значение, которое не следует игнорировать
// (важно для функций, которые возвращают новые значения, не изменяя исходные объекты)
// - constexpr - метод можно вычислить на этапе компиляции
// - const - метод не изменяет объект
// - noexcept - метод гарантированно не бросает исключения (важно для оптимизации операций в STL-контейнерах)

inline constexpr double EPS = 1e-10;  // точность сравнения double-координат

// Класс 2D-точки
class Point2D {
public:
    // Конструктор по умолчанию
    constexpr Point2D() noexcept : x_(0.0), y_(0.0) {}

    // Параметрический конструктор, принимающий координаты точки
    constexpr Point2D(double x, double y) noexcept : x_(x), y_(y) {}

    // Методы доступа (геттеры) к координатам точки
    [[nodiscard]] constexpr double X() const noexcept { return x_; }
    [[nodiscard]] constexpr double Y() const noexcept { return y_; }

    // Сеттеры не нужны - точки неизменяемы (все операции создают новые точки)

    // === Перегруженные операторы сравнения ===
    [[nodiscard]] constexpr bool operator<(const Point2D &other) const noexcept {
        // Сначала сравниваем x с учетом погрешности
        if (std::abs(x_ - other.x_) > EPS) {
            return x_ < other.x_;
        }
        // Если x почти равны, сравниваем y с учетом погрешности
        if (std::abs(y_ - other.y_) > EPS) {
            return y_ < other.y_;
        }
        // Точки равны в пределах погрешности
        return false;
    }
    [[nodiscard]] constexpr bool operator==(const Point2D &other) const noexcept {
        // Сравниваем с учетом погрешности: |x1 - x2| < EPS && |y1 - y2| < EPS
        return (std::abs(x_ - other.x_) < EPS) && (std::abs(y_ - other.y_) < EPS);
    }

    // === Перегруженные математические бинарные операторы ===
    [[nodiscard]] constexpr Point2D operator+(const Point2D &other) const noexcept {
        return {x_ + other.x_, y_ + other.y_};
    }
    [[nodiscard]] constexpr Point2D operator-(const Point2D &other) const noexcept {
        return {x_ - other.x_, y_ - other.y_};
    }
    [[nodiscard]] constexpr Point2D operator*(double value) const noexcept { return {x_ * value, y_ * value}; }
    [[nodiscard]] constexpr Point2D operator/(double value) const noexcept {
        return std::abs(value) >= EPS ? Point2D{x_ / value, y_ / value}
                                      : Point2D{0.0, 0.0};  // защита от деления на ноль
    }

    // === Методы, реализующие бинарные геометрические операции ===
    // Скалярное произведение
    [[nodiscard]] constexpr double Dot(const Point2D &other) const noexcept { return x_ * other.x_ + y_ * other.y_; }
    // Векторное произведение (в 2D - площадь параллелограмма, "+" значение - поворот против часовой стрелки)
    [[nodiscard]] constexpr double Cross(const Point2D &other) const noexcept { return x_ * other.y_ - y_ * other.x_; }
    // Длина вектора из (0, 0) в точку
    [[nodiscard]] constexpr double Length() const noexcept {
        // Считается по теореме Пифагора (сумма квадратов под корнем всегда >= 0), поэтому не будет бросать исключение
        return std::sqrt(x_ * x_ + y_ * y_);
    }
    // Евклидово расстояние между точками
    [[nodiscard]] constexpr double DistanceTo(const Point2D &other) const noexcept { return (*this - other).Length(); }
    // Нормализация вектора из (0, 0) в точку
    [[nodiscard]] constexpr Point2D Normalize() {
        return *this / Length();  // если Length() вернет 0.0, то оператор деления защитит
    }
    // Метод, сравнивающий координаты точки с (0.0) с учетом погрешности
    [[nodiscard]] constexpr bool IsZero() const noexcept { return (std::abs(x_) < EPS) && (std::abs(y_) < EPS); }

private:
    double x_;  // координаты точки
    double y_;
};

// Шаблонный класс статической ломаной линии (число точек известно на этапе компиляции)
template <size_t N>
struct Lines2D {
    std::array<double, N> x;  // координаты X точек ломаной линии
    std::array<double, N> y;  // координаты Y точек ломаной линии
    // Примечание: используются отдельные array вместо array Point2D для оптимизации передачи в библиотеку Matplot++
};

// Класс динамической ломаной линии (число точек неизвестно на этапе компиляции)
struct Lines2DDyn {
    std::vector<double> x;  // координаты X точек ломаной линии
    std::vector<double> y;  // координаты Y точек ломаной линии
    // Примечание: используются отдельные векторы вместо вектора Point2D для оптимизации передачи в библиотеку Matplot++

    // Метод, резервирующий память под точки ломаной линии
    void Reserve(size_t n) {
        x.reserve(n);
        y.reserve(n);
    }
    // Метод, добавляющий точку в ломаную линию
    void PushBack(Point2D p) {
        x.push_back(p.X());
        y.push_back(p.Y());
    }
    // Перегруженный метод, добавляющий координаты точки в ломаную линию
    void PushBack(double px, double py) {
        x.push_back(px);
        y.push_back(py);
    }
    // Метод, возвращающий первую точку ломаной линии
    [[nodiscard]] Point2D Front() const { return {x.front(), y.front()}; }
};

// Класс ограничивающего бокса (прямоугольника со сторонами, параллельными осям координат)
class BoundingBox {
public:
    // Конструктор по умолчанию
    constexpr BoundingBox() noexcept : bottom_left_(0.0, 0.0), top_right_(0.0, 0.0) {}

    // Параметрический конструктор, принимающий границы ограничивающего бокса
    constexpr BoundingBox(double min_x, double min_y, double max_x, double max_y) noexcept
        : bottom_left_(min_x, min_y), top_right_(max_x, max_y) {}

    // Параметрический конструктор, принимающий диагональные вершины ограничивающего бокса
    constexpr BoundingBox(const Point2D &bottom_left, const Point2D &top_right) noexcept
        : bottom_left_(bottom_left.X(), bottom_left.Y()), top_right_(top_right.X(), top_right.Y()) {}

    // Методы доступа к диагональным вершинам ограничивающего бокса
    [[nodiscard]] constexpr Point2D BottomLeft() const noexcept { return bottom_left_; }
    [[nodiscard]] constexpr Point2D TopRight() const noexcept { return top_right_; }

    // Метод, проверяющий пересечение двух ограничивающих боксов
    [[nodiscard]] constexpr bool Overlaps(const BoundingBox &other) const noexcept {
        // Нет пересечения, если один бокс полностью слева, справа, снизу или сверху
        return !(top_right_.X() < other.bottom_left_.X() || bottom_left_.X() > other.top_right_.X() ||
                 top_right_.Y() < other.bottom_left_.Y() || bottom_left_.Y() > other.top_right_.Y());
    }

    // Метод, возвращающий ширину ограничивающего бокса
    [[nodiscard]] constexpr double Width() const noexcept { return top_right_.X() - bottom_left_.X(); }

    // Метод, возвращающий высоту ограничивающего бокса
    [[nodiscard]] constexpr double Height() const noexcept { return top_right_.Y() - bottom_left_.Y(); }

    // Метод, возвращающий центр ограничивающего бокса
    [[nodiscard]] constexpr Point2D Center() const noexcept { return (bottom_left_ + top_right_) / 2.0; }

private:
    Point2D bottom_left_;  // левая нижняя вершина (минимальные x, y)
    Point2D top_right_;    // правая верхняя вершина (максимальные x, y)
};

// Класс отрезка прямой линии
class Line {
public:
    // Конструктор по умолчанию отсутствует намеренно (предотвращение создания невалидных объектов)

    // Параметрический конструктор, принимающий начало и конец отрезка
    constexpr Line(Point2D start, Point2D end) noexcept : start_(start), end_(end) {}

    // Методы доступа к начальной и конечной точкам отрезка
    [[nodiscard]] constexpr Point2D Start() const noexcept { return start_; }
    [[nodiscard]] constexpr Point2D End() const noexcept { return end_; }

    // Метод, возвращающий длину отрезка
    [[nodiscard]] constexpr double Length() const noexcept { return start_.DistanceTo(end_); }

    // Метод, возвращающий направление отрезка (не единичный вектор)
    [[nodiscard]] constexpr Point2D Dir() const noexcept { return (end_ - start_); }

    // Метод, возвращающий направление отрезка (единичный вектор)
    [[nodiscard]] constexpr Point2D DirNorm() const noexcept { return Dir().Normalize(); }

    // Метод, возвращающий ограничивающий бокс отрезка
    [[nodiscard]] constexpr BoundingBox BoundBox() const noexcept {
        return {{std::min(start_.X(), end_.X()), std::min(start_.Y(), end_.Y())},   // левая нижняя вершина бокса
                {std::max(start_.X(), end_.X()), std::max(start_.Y(), end_.Y())}};  // правая верхняя вершина бокса
    }

    // Метод, возвращающий наибольшую Y координату отрезка
    [[nodiscard]] constexpr double MaxY() const noexcept { return std::max(start_.Y(), end_.Y()); }

    // Метод, возвращающий центр отрезка
    [[nodiscard]] constexpr Point2D Center() const noexcept { return (start_ + end_) / 2.0; }

    // Метод, возвращающий вершины отрезка
    [[nodiscard]] constexpr std::array<Point2D, 2> Vertices() const noexcept { return {start_, end_}; }

    // Метод, возвращающий отрезок в формате отрисовки библиотеки Matplot++
    [[nodiscard]] constexpr Lines2D<2> Lines() const noexcept {
        return {{start_.X(), end_.X()}, {start_.Y(), end_.Y()}};
    }

private:
    Point2D start_;  // начальная вершина отрезка
    Point2D end_;    // конечная вершина отрезка
};

// Класс треугольника
class Triangle {
public:
    // Конструктор по умолчанию отсутствует намеренно (предотвращение создания невалидных объектов)

    // Параметрический конструктор, принимающий три точки треугольника
    constexpr Triangle(Point2D a, Point2D b, Point2D c) noexcept : a_(a), b_(b), c_(c) {}

    // Методы доступа к вершинам треугольника
    [[nodiscard]] constexpr Point2D A() const noexcept { return a_; }
    [[nodiscard]] constexpr Point2D B() const noexcept { return b_; }
    [[nodiscard]] constexpr Point2D C() const noexcept { return c_; }

    // Метод, возвращающий площадь треугольника
    [[nodiscard]] constexpr double Area() const noexcept { return std::abs((b_ - a_).Cross(c_ - a_)) / 2.0; }

    // Метод, возвращающий ограничивающий бокс треугольника
    [[nodiscard]] constexpr BoundingBox BoundBox() const noexcept {
        std::initializer_list<double> x_coords = {a_.X(), b_.X(), c_.X()};
        std::initializer_list<double> y_coords = {a_.Y(), b_.Y(), c_.Y()};
        return {{std::min(x_coords), std::min(y_coords)},   // левая нижняя вершина бокса
                {std::max(x_coords), std::max(y_coords)}};  // правая верхняя вершина бокса
    }

    // Метод, возвращающий вершины треугольника
    [[nodiscard]] constexpr std::array<Point2D, 3> Vertices() const noexcept { return {a_, b_, c_}; }

    // Метод, возвращающий наибольшую Y координату треугольника
    [[nodiscard]] constexpr double MaxY() const noexcept { return std::max({a_.Y(), b_.Y(), c_.Y()}); }

    // Метод, возвращающий центр тяжести треугольника (центроид)
    [[nodiscard]] constexpr Point2D Center() const noexcept { return (a_ + b_ + c_) / 3.0; }

    // Метод, возвращающий треугольник в формате отрисовки библиотеки Matplot++
    // (замкнутая ломаная из 4 точек: a -> b -> c -> a)
    [[nodiscard]] constexpr Lines2D<4> Lines() const noexcept {
        return {{a_.X(), b_.X(), c_.X(), a_.X()}, {a_.Y(), b_.Y(), c_.Y(), a_.Y()}};
    }

private:
    Point2D a_;  // первая вершина треугольника
    Point2D b_;  // вторая вершина треугольника
    Point2D c_;  // третья вершина треугольника
};

// Класс прямоугольника со сторонами, параллельными осям координат
class Rectangle {
public:
    // Конструктор по умолчанию отсутствует (предотвращение создания невалидных объектов)

    // Параметрический конструктор, принимающий левый нижний угол, ширину и высоту
    constexpr Rectangle(Point2D bottom_left, double width, double height) noexcept
        : bottom_left_(bottom_left), width_(width), height_(height) {}

    // Методы доступа к данным-членам
    [[nodiscard]] constexpr Point2D BottomLeft() const noexcept { return bottom_left_; }
    [[nodiscard]] constexpr double Width() const noexcept { return width_; }
    [[nodiscard]] constexpr double Height() const noexcept { return height_; }

    // Метод, возвращающий правую верхнюю вершину прямоугольника
    [[nodiscard]] constexpr Point2D TopRight() const noexcept {
        return {bottom_left_.X() + width_, bottom_left_.Y() + height_};
    }

    // Метод, возвращающий ограничивающий бокс (совпадает с самим прямоугольником)
    [[nodiscard]] constexpr BoundingBox BoundBox() const noexcept { return {bottom_left_, TopRight()}; }

    // Метод, возвращающий все четыре вершины прямоугольника
    [[nodiscard]] constexpr std::array<Point2D, 4> Vertices() const noexcept {
        auto top_right = TopRight();  // получим правую верхнюю вершину прямоугольника
        return {
            bottom_left_,                       // левая нижняя
            {top_right.X(), bottom_left_.Y()},  // правая нижняя
            top_right,                          // правая верхняя
            {bottom_left_.X(), top_right.Y()}   // левая верхняя
        };
    }

    // Метод, возвращающий наибольшую Y координату прямоугольника
    [[nodiscard]] constexpr double MaxY() const noexcept { return bottom_left_.Y() + height_; }

    // Метод, возвращающий центр прямоугольника
    [[nodiscard]] constexpr Point2D Center() const noexcept { return bottom_left_ + Point2D{width_, height_} / 2.0; }

    // Метод, возвращающий прямоугольник в формате отрисовки библиотеки Matplot++
    // (замкнутая ломаная из 5 точек: левая нижняя -> левая верхняя -> правая верхняя -> правая нижняя -> левая нижняя)
    [[nodiscard]] constexpr Lines2D<5> Lines() const noexcept {
        auto top_right = TopRight();  // получим правую верхнюю вершину прямоугольника
        return {{bottom_left_.X(), bottom_left_.X(), top_right.X(), top_right.X(), bottom_left_.X()},
                {bottom_left_.Y(), top_right.Y(), top_right.Y(), bottom_left_.Y(), bottom_left_.Y()}};
    }

private:
    Point2D bottom_left_;  // левая нижняя вершина
    double width_;         // ширина прямоугольника
    double height_;        // высота прямоугольника
};

// Класс правильного многоугольника (все стороны и углы равны)
class RegularPolygon {
public:
    // Конструктор по умолчанию отсутствует (предотвращение создания невалидных объектов)

    // Параметрический конструктор, принимающий центр, радиус и количество сторон многоугольника
    constexpr RegularPolygon(Point2D center, double radius, int sides) noexcept
        : center_(center), radius_(radius), sides_(sides) {}

    // Методы доступа к данным-членам
    [[nodiscard]] constexpr Point2D Center() const noexcept { return center_; }
    [[nodiscard]] constexpr double Radius() const noexcept { return radius_; }
    [[nodiscard]] constexpr int Sides() const noexcept { return sides_; }

    // Метод, возвращающий все вершины многоугольника (std::vector может быть constexpr в C++20 и выше)
    [[nodiscard]] constexpr std::vector<Point2D> Vertices() const {
        std::vector<Point2D> points;
        points.reserve(sides_);  // предварительное выделение памяти для эффективности

        for (int i = 0; i < sides_; ++i) {
            // Равномерно распределяем вершины по окружности
            const double angle = 2.0 * std::numbers::pi * i / sides_;
            points.emplace_back(center_.X() + radius_ * std::cos(angle), center_.Y() + radius_ * std::sin(angle));
        }
        return points;
    }

    // Метод, возвращающий ограничивающий бокс (квадрат, описанный вокруг окружности)
    [[nodiscard]] constexpr BoundingBox BoundBox() const noexcept {
        auto shift = Point2D{radius_, radius_};
        return {center_ - shift, center_ + shift};
    }

    // Метод, возвращающий наибольшую Y координату (верхнюю точку описанной окружности)
    [[nodiscard]] constexpr double MaxY() const noexcept { return center_.Y() + radius_; }

    // Метод, возвращающий многоугольник в формате отрисовки библиотеки Matplot++
    // (замкнутая ломаная из всех вершин + первая вершина в конце)
    [[nodiscard]] constexpr Lines2DDyn Lines() const {
        auto verts = Vertices();  // получаем вершины
        Lines2DDyn lines;
        lines.Reserve(verts.size() + 1);  // +1 для замыкания

        for (const auto &p : verts) {
            lines.PushBack(p);
        }
        lines.PushBack(lines.Front());  // замыкаем многоугольник

        return lines;
    }

private:
    Point2D center_;  // центр описанной окружности
    double radius_;   // радиус описанной окружности (расстояние от центра до вершин)
    int sides_;       // количество сторон (>= 3 для многоугольника)
};

// Класс окружности
class Circle {
public:
    // Конструктор по умолчанию отсутствует (предотвращение создания невалидных объектов)

    // Параметрический конструктор, принимающий центр и радиус
    constexpr Circle(Point2D center, double radius) noexcept : center_(center), radius_(radius) {}

    // Методы доступа к данным-членам
    [[nodiscard]] constexpr Point2D Center() const noexcept { return center_; }
    [[nodiscard]] constexpr double Radius() const noexcept { return radius_; }

    // Метод, возвращающий ограничивающий бокс (квадрат, описанный вокруг окружности)
    [[nodiscard]] constexpr BoundingBox BoundBox() const noexcept {
        auto shift = Point2D{radius_, radius_};
        return {center_ - shift, center_ + shift};
    }

    // Метод, возвращающий наибольшую Y координату (верхнюю точку окружности)
    [[nodiscard]] constexpr double MaxY() const noexcept { return center_.Y() + radius_; }

    // Метод, возвращающий аппроксимацию окружности многоугольником с N вершинами
    [[nodiscard]] constexpr std::vector<Point2D> Vertices(size_t N = 30) const {
        std::vector<Point2D> points;
        points.reserve(N);  // предварительное выделение памяти для эффективности

        for (auto i : std::ranges::views::iota(0u, N)) {
            const double angle = 2.0 * std::numbers::pi * i / N;
            points.emplace_back(center_.X() + radius_ * std::cos(angle), center_.Y() + radius_ * std::sin(angle));
        }
        return points;
    }

    // Метод, возвращающий окружность в формате отрисовки библиотеки Matplot++
    // (замкнутая ломаная из N+1 точек, аппроксимирующая окружность)
    [[nodiscard]] constexpr Lines2DDyn Lines(size_t N = 100) const {
        Lines2DDyn lines;
        lines.Reserve(N + 1);  // +1 для замыкания

        for (auto i : std::ranges::views::iota(0u, N)) {
            double angle = 2 * std::numbers::pi * i / N;
            lines.PushBack(center_.X() + radius_ * std::cos(angle), center_.Y() + radius_ * std::sin(angle));
        }
        lines.PushBack(lines.Front());  // замыкаем окружность

        return lines;
    }

private:
    Point2D center_;  // центр окружности
    double radius_;   // радиус окружности (> 0)
};

// Класс произвольного многоугольника (заданного массивом вершин)
class Polygon {
public:
    // Конструктор по умолчанию отсутствует (предотвращение создания невалидных объектов)

    // Параметрический конструктор, принимающий вектор вершин
    constexpr Polygon(std::vector<Point2D> points) noexcept : points_(std::move(points)) {
        CalculateBoundBox();  // вычисляем ограничивающий бокс при создании
    }
    // Метод, возвращающий ограничивающий бокс многоугольника (кешированное значение)
    [[nodiscard]] constexpr BoundingBox BoundBox() const noexcept { return bounding_box_; }

    // // Метод, возвращающий высоту многоугольника
    // [[nodiscard]] constexpr double Height() const noexcept { return bounding_box_.Height(); }

    // Метод, возвращающий наибольшую Y координату (верхнюю границу) многоугольника
    [[nodiscard]] constexpr double MaxY() const noexcept { return bounding_box_.TopRight().Y(); }

    // Метод, возвращающий центр многоугольника (центр ограничивающего бокса)
    [[nodiscard]] constexpr Point2D Center() const noexcept { return bounding_box_.Center(); }

    // Метод, возвращающий вершины многоугольника (только для чтения)
    [[nodiscard]] constexpr std::span<const Point2D> Vertices() const noexcept { return points_; }

    // Метод, возвращающий многоугольник в формате отрисовки библиотеки Matplot++
    [[nodiscard]] constexpr Lines2DDyn Lines() const {
        Lines2DDyn lines;
        lines.Reserve(points_.size() + 1);  // +1 для замыкания

        for (const auto &p : points_) {
            lines.PushBack(p);
        }
        lines.PushBack(lines.Front());  // замыкаем многоугольник

        return lines;
    }

private:
    // Метод для вычисления ограничивающего бокса по вершинам
    void CalculateBoundBox() {
        double min_x = points_[0].X();
        double max_x = points_[0].X();
        double min_y = points_[0].Y();
        double max_y = points_[0].Y();

        for (const auto &p : points_) {
            min_x = std::min(min_x, p.X());
            max_x = std::max(max_x, p.X());
            min_y = std::min(min_y, p.Y());
            max_y = std::max(max_y, p.Y());
        }

        bounding_box_ = BoundingBox{min_x, min_y, max_x, max_y};
    }

    std::vector<Point2D> points_;  // вершины многоугольника
    BoundingBox bounding_box_;     // кешированный ограничивающий бокс
};

// Тип-сумма для хранения любой фигуры (полиморфизм без виртуальных функций).
// Все операции над фигурами реализуются через паттерн "посетитель" (std::visit).
using Shape = std::variant<Line, Triangle, Rectangle, RegularPolygon, Circle, Polygon>;
}  // namespace geometry

// Специализация шаблона std::formatter для типа Point2D
// (чтобы использовать его в std::format и std::print)
template <>
struct std::formatter<geometry::Point2D> {
    // Метод, выполняющий парсинг спецификаторов формата (значения в скобках {})
    constexpr auto parse(std::format_parse_context &ctx) {
        return ctx.begin();  // игнорируем спецификаторы и возвращаем начало строки формата
    }

    // Шаблонный метод-инструкция, отвечающий за преобразование значения Point2D в строку
    // (параметр FormatContext задает, куда выводить результат)
    template <typename FormatContext>
    auto format(const geometry::Point2D &p, FormatContext &ctx) {
        return format_to(ctx.out(), "({:.2f}, {:.2f})", p.X(), p.Y());
    }
};

// Специализация шаблона std::formatter для типа std::vector<Point2D>
// (чтобы использовать его в std::format и std::print)
template <>
struct std::formatter<std::vector<geometry::Point2D>> {
    bool use_new_line = false;  // флаг для режима вывода: true - каждая точка на новой строке

    // Метод, выполняющий парсинг спецификаторов формата (значения в скобках {})
    constexpr auto parse(std::format_parse_context &ctx) {
        auto it = ctx.begin();  // итератор на начало спецификатора

        // Если спецификатор пустой ({}) - возвращаем начало
        if (it == ctx.end())
            return it;

        // Создаем string_view для удобной проверки спецификатора
        std::string_view spec{it, ctx.end()};

        // Проверяем, является ли спецификатор "new_line"
        if (spec.starts_with("new_line")) {
            use_new_line = true;  // поднимаем флаг режима вывода "каждая точка на новой строке"
            // Возвращаем итератор на конец обработанной части (после "new_line" могут быть другие спецификаторы)
            return ctx.begin() + std::string_view("new_line").size();
        }

        // Если спецификатор не распознан, возвращаем начало (пустой спецификатор допустим)
        return it;
    }

    // Шаблонный метод-инструкция, отвечающий за преобразование значения std::vector<Point2D> в строку
    // (параметр FormatContext задает, куда выводить результат)
    template <typename FormatContext>
    auto format(const std::vector<geometry::Point2D> &v, FormatContext &ctx) {
        auto out = ctx.out();  // итератор вывода

        // Создаем отображение из отформатированных точек
        auto point_views = v | std::views::transform([](const auto &p) { return std::format("{}", p); });

        // Если включен режим "с новой строки"
        if (use_new_line) {
            // Добавляем табуляцию перед каждой точкой
            auto with_tabs = point_views | std::views::transform([](std::string s) { return "\t" + s; });

            // Вставляем между точками символ переноса строки и выводим
            out = std::format_to(out, "{}", with_tabs | std::views::join_with('\n'));
        }
        // В противном случае работает режим "в одну строку"
        else {
            out = std::format_to(out, "[{}]", point_views | std::views::join_with(", "));
        }
        return out;  // возвращаем итератор вывода
    }
};

// Специализация шаблона std::formatter для типа Line
// (чтобы использовать его в std::format и std::print)
template <>
struct std::formatter<geometry::Line> {
    // Метод, выполняющий парсинг спецификаторов формата (значения в скобках {})
    constexpr auto parse(std::format_parse_context &ctx) {
        return ctx.begin();  // игнорируем спецификаторы и возвращаем начало строки формата
    }

    // Шаблонный метод-инструкция, отвечающий за преобразование значения Line в строку
    // (параметр FormatContext задает, куда выводить результат)
    template <typename FormatContext>
    auto format(const geometry::Line &l, FormatContext &ctx) {
        return std::format_to(ctx.out(), "Line({}, {})", l.Start(), l.End());
    }
};

// Специализация шаблона std::formatter для типа Circle
// (чтобы использовать его в std::format и std::print)
template <>
struct std::formatter<geometry::Circle> {
    // Метод, выполняющий парсинг спецификаторов формата (значения в скобках {})
    constexpr auto parse(std::format_parse_context &ctx) {
        return ctx.begin();  // игнорируем спецификаторы и возвращаем начало строки формата
    }

    // Шаблонный метод-инструкция, отвечающий за преобразование значения Circle в строку
    // (параметр FormatContext задает, куда выводить результат)
    template <typename FormatContext>
    auto format(const geometry::Circle &c, FormatContext &ctx) {
        return std::format_to(ctx.out(), "Circle(center={}, r={:.2f})", c.Center(), c.Radius());
    }
};

// Специализация шаблона std::formatter для типа Rectangle
// (чтобы использовать его в std::format и std::print)
template <>
struct std::formatter<geometry::Rectangle> {
    // Метод, выполняющий парсинг спецификаторов формата (значения в скобках {})
    constexpr auto parse(std::format_parse_context &ctx) {
        return ctx.begin();  // игнорируем спецификаторы и возвращаем начало строки формата
    }

    // Шаблонный метод-инструкция, отвечающий за преобразование значения Rectangle в строку
    // (параметр FormatContext задает, куда выводить результат)
    template <typename FormatContext>
    auto format(const geometry::Rectangle &r, FormatContext &ctx) {
        return std::format_to(ctx.out(), "Rectangle(bottom_left={}, w={:.2f}, h={:.2f})", r.BottomLeft(), r.Width(),
                              r.Height());
    }
};

// Специализация шаблона std::formatter для типа RegularPolygon
// (чтобы использовать его в std::format и std::print)
template <>
struct std::formatter<geometry::RegularPolygon> {
    // Метод, выполняющий парсинг спецификаторов формата (значения в скобках {})
    constexpr auto parse(std::format_parse_context &ctx) {
        return ctx.begin();  // игнорируем спецификаторы и возвращаем начало строки формата
    }

    // Шаблонный метод-инструкция, отвечающий за преобразование значения RegularPolygon в строку
    // (параметр FormatContext задает, куда выводить результат)
    template <typename FormatContext>
    auto format(const geometry::RegularPolygon &p, FormatContext &ctx) {
        return std::format_to(ctx.out(), "RegularPolygon(center={}, r={:.2f}, sides={})", p.Center(), p.Radius(),
                              p.Sides());
    }
};

// Специализация шаблона std::formatter для типа Triangle
// (чтобы использовать его в std::format и std::print)
template <>
struct std::formatter<geometry::Triangle> {
    // Метод, выполняющий парсинг спецификаторов формата (значения в скобках {})
    constexpr auto parse(std::format_parse_context &ctx) {
        return ctx.begin();  // игнорируем спецификаторы и возвращаем начало строки формата
    }

    // Шаблонный метод-инструкция, отвечающий за преобразование значения Triangle в строку
    // (параметр FormatContext задает, куда выводить результат)
    template <typename FormatContext>
    auto format(const geometry::Triangle &t, FormatContext &ctx) {
        return std::format_to(ctx.out(), "Triangle({}, {}, {})", t.A(), t.B(), t.C());
    }
};

// Специализация шаблона std::formatter для типа Polygon
// (чтобы использовать его в std::format и std::print)
template <>
struct std::formatter<geometry::Polygon> {
    // Метод, выполняющий парсинг спецификаторов формата (значения в скобках {})
    constexpr auto parse(std::format_parse_context &ctx) {
        return ctx.begin();  // игнорируем спецификаторы и возвращаем начало строки формата
    }

    // Шаблонный метод-инструкция, отвечающий за преобразование значения Polygon в строку
    // (параметр FormatContext задает, куда выводить результат)
    template <typename FormatContext>
    auto format(const geometry::Polygon &poly, FormatContext &ctx) {
        auto out = ctx.out();
        out = std::format_to(out, "Polygon[{} points]: [", poly.Vertices().size());

        for (const auto &p : poly.Vertices()) {
            out = std::format_to(out, "{} ", p);
        }

        return std::format_to(out, "]");
    }
};
