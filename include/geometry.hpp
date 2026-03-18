#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <format>
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

    // Метод, проверяющий пересечение двух ограничивающих боксов
    [[nodiscard]] constexpr bool Overlaps(const BoundingBox &other) const noexcept {
        // Нет пересечения, если один бокс полностью слева, справа, снизу или сверху
        return !(top_right_.X() < other.bottom_left_.X() || bottom_left_.X() > other.top_right_.X() ||
                 top_right_.Y() < other.bottom_left_.Y() || bottom_left_.Y() > other.top_right_.Y());
    }
    // Методы, возвращающие ширину, длину и центр ограничивающего бокса
    [[nodiscard]] constexpr double Width() const noexcept { return top_right_.X() - bottom_left_.X(); }
    [[nodiscard]] constexpr double Height() const noexcept { return top_right_.Y() - bottom_left_.Y(); }
    [[nodiscard]] constexpr Point2D Center() const noexcept { return (bottom_left_ + top_right_) / 2.0; }

private:
    Point2D bottom_left_;  // левая нижняя точка (минимальные x, y)
    Point2D top_right_;    // правая верхняя точка (максимальные x, y)
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

    // Метод, возвращающий направление отрезка (единичный вектор)
    [[nodiscard]] constexpr Point2D Direction() const noexcept { return (end_ - start_).Normalize(); }

    // Метод, возвращающий ограничивающий бокс отрезка
    [[nodiscard]] constexpr BoundingBox BoundBox() const noexcept {
        return {std::min(start_.X(), end_.X()), std::min(start_.Y(), end_.Y()), std::max(start_.X(), end_.X()),
                std::max(start_.Y(), end_.Y())};
    }

    // Метод, возвращающий наибольшую Y координату отрезка (для сортировки по высоте)
    [[nodiscard]] constexpr double Height() const noexcept { return std::max(start_.Y(), end_.Y()); }

    // Метод, возвращающий центр отрезка
    [[nodiscard]] constexpr Point2D Center() const noexcept { return (start_ + end_) / 2.0; }

    // Метод, возвращающий вершины отрезка
    [[nodiscard]] constexpr std::array<Point2D, 2> Vertices() const noexcept { return {start_, end_}; }

    // Метод, возвращающий отрезок в формате отрисовки библиотеки Matplot++
    [[nodiscard]] constexpr Lines2D<2> Lines() const noexcept {
        return {{start_.X(), end_.X()}, {start_.Y(), end_.Y()}};
    }

private:
    Point2D start_;  // начальная точка отрезка
    Point2D end_;    // конечная точка отрезка
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
        return {std::min({a_.X(), b_.X(), c_.X()}), std::min({a_.Y(), b_.Y(), c_.Y()}),
                std::max({a_.X(), b_.X(), c_.X()}), std::max({a_.Y(), b_.Y(), c_.Y()})};
    }

    // Метод, возвращающий вершины треугольника
    [[nodiscard]] constexpr std::array<Point2D, 3> Vertices() const noexcept { return {a_, b_, c_}; }

    // Метод, возвращающий наибольшую Y координату треугольника (для сортировки по высоте)
    [[nodiscard]] constexpr double Height() const noexcept { return std::max({a_.Y(), b_.Y(), c_.Y()}); }

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

struct Rectangle {
    Point2D bottom_left;
    double width, height;

    constexpr Rectangle(Point2D bottom_left, double width, double height) noexcept
        : bottom_left(bottom_left), width(width), height(height) {}

    [[nodiscard]] constexpr Point2D TopRight() const noexcept {
        return {bottom_left.X() + width, bottom_left.Y() + height};
    }
    [[nodiscard]] constexpr BoundingBox BoundBox() const noexcept {
        return {bottom_left.X(), bottom_left.Y(), bottom_left.X() + width, bottom_left.Y() + height};
    }
    [[nodiscard]] constexpr std::array<Point2D, 4> Vertices() const noexcept {
        return {bottom_left,
                {bottom_left.X() + width, bottom_left.Y()},
                {bottom_left.X() + width, bottom_left.Y() + height},
                {bottom_left.X(), bottom_left.Y() + height}};
    }
    [[nodiscard]] constexpr double Height() const noexcept { return bottom_left.Y() + height; }
    [[nodiscard]] constexpr Point2D Center() noexcept { return bottom_left + (Point2D{width, height} / 2.0); }

    [[nodiscard]] constexpr Lines2D<5> Lines() const noexcept {
        return {
            {bottom_left.X(), bottom_left.X(), bottom_left.X() + width, bottom_left.X() + width, bottom_left.X()},
            {bottom_left.Y(), bottom_left.Y() + height, bottom_left.Y() + height, bottom_left.Y(), bottom_left.Y()}};
    }
};

struct RegularPolygon {
    Point2D center_p;
    double radius;
    int sides;

    constexpr RegularPolygon(Point2D center, double radius, int sides)
        : center_p(center), radius(radius), sides(sides) {}

    std::vector<Point2D> Vertices() const {
        std::vector<Point2D> points;
        points.reserve(sides);

        for (int i = 0; i < sides; ++i) {
            const double angle = 2 * std::numbers::pi * i / sides;
            points.emplace_back(center_p.X() + radius * std::cos(angle), center_p.Y() + radius * std::sin(angle));
        }
        return points;
    }

    [[nodiscard]] constexpr BoundingBox BoundBox() const noexcept {
        return {center_p.X() - radius, center_p.Y() - radius, center_p.X() + radius, center_p.Y() + radius};
    }
    [[nodiscard]] constexpr double Height() const noexcept { return center_p.Y() + radius; }
    [[nodiscard]] constexpr Point2D Center() const noexcept { return center_p; }

    [[nodiscard]] constexpr Lines2DDyn Lines() {
        auto verts = Vertices();
        Lines2DDyn lines;
        lines.Reserve(verts.size() + 1);
        for (const auto &p : verts) {
            lines.PushBack(p);
        }
        lines.PushBack(lines.Front());
        return lines;
    }
};

struct Circle {
    Point2D center_p;
    double radius;

    constexpr Circle(Point2D center, double radius) noexcept : center_p(center), radius(radius) {}

    [[nodiscard]] constexpr BoundingBox BoundBox() const noexcept {
        return {center_p.X() - radius, center_p.Y() - radius, center_p.X() + radius, center_p.Y() + radius};
    }
    [[nodiscard]] constexpr double Height() const noexcept { return center_p.Y() + radius; }
    [[nodiscard]] constexpr Point2D Center() const noexcept { return center_p; }

    [[nodiscard]] constexpr std::vector<Point2D> Vertices(size_t N = 30) const {
        std::vector<Point2D> points;
        points.reserve(N);

        for (auto i : std::ranges::views::iota(0u, N)) {
            const double angle = 2 * std::numbers::pi * i / N;
            points.emplace_back(center_p.X() + radius * std::cos(angle), center_p.Y() + radius * std::sin(angle));
        }
        return points;
    }
    [[nodiscard]] constexpr Lines2DDyn Lines(size_t N = 100) const {
        Lines2DDyn lines;
        lines.Reserve(N + 1);
        for (auto i : std::ranges::views::iota(0u, N)) {
            double angle = 2 * std::numbers::pi * i / N;
            lines.PushBack(center_p.X() + radius * std::cos(angle), center_p.Y() + radius * std::sin(angle));
        }
        lines.PushBack(lines.Front());
        return lines;
    }
};

class Polygon {
public:
    constexpr Polygon(std::vector<Point2D> points) noexcept : points_(std::move(points)) { CalculateBoundBox(); }

    [[nodiscard]] constexpr BoundingBox BoundBox() const noexcept { return bounding_box_; }
    [[nodiscard]] constexpr double Height() const noexcept {
        const auto box = BoundBox();
        return box.Height();
    }

    [[nodiscard]] constexpr Point2D Center() const noexcept {
        const auto box = BoundBox();
        return box.Center();
    }

    [[nodiscard]] constexpr std::span<const Point2D> Vertices() const noexcept { return points_; }
    [[nodiscard]] constexpr Lines2DDyn Lines() const {
        Lines2DDyn lines;
        lines.Reserve(points_.size() + 1);
        for (const auto &p : points_) {
            lines.PushBack(p);
        }
        lines.PushBack(lines.Front());
        return lines;
    }

private:
    void CalculateBoundBox() {
        double min_x = points_[0].X(), max_x = points_[0].X();
        double min_y = points_[0].Y(), max_y = points_[0].Y();

        for (const auto &p : points_) {
            if (p.X() < min_x)
                min_x = p.X();
            if (p.X() > max_x)
                max_x = p.X();
            if (p.Y() < min_y)
                min_y = p.Y();
            if (p.Y() > max_y)
                max_y = p.Y();
        }

        bounding_box_ = BoundingBox{min_x, min_y, max_x, max_y};
    }

    std::vector<Point2D> points_;
    BoundingBox bounding_box_;
};

using Shape = std::variant<Line, Triangle, Rectangle, RegularPolygon, Circle, Polygon>;
}  // namespace geometry

template <>
struct std::formatter<geometry::Point2D> {
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::Point2D &p, FormatContext &ctx) {
        return format_to(ctx.out(), "({:.2f}, {:.2f})", p.X(), p.Y());
    }
};
template <>
struct std::formatter<std::vector<geometry::Point2D>> {
    bool use_new_line = false;

    constexpr auto parse(std::format_parse_context &ctx) {
        auto it = ctx.begin();

        /* ваш код здесь */

        return it;
    }

    template <typename FormatContext>
    auto format(const std::vector<geometry::Point2D> &v, FormatContext &ctx) {

        /* ваш код здесь */
        return ctx.out();
    }
};

template <>
struct std::formatter<geometry::Line> {
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::Line &l, FormatContext &ctx) {
        return std::format_to(ctx.out(), "Line({}, {})", l.Start(), l.End());
    }
};

template <>
struct std::formatter<geometry::Circle> {
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::Circle &c, FormatContext &ctx) {
        return std::format_to(ctx.out(), "Circle(center={}, r={:.2f})", c.center_p, c.radius);
    }
};

template <>
struct std::formatter<geometry::Rectangle> {
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::Rectangle &r, FormatContext &ctx) {
        return std::format_to(ctx.out(), "Rectangle(bottom_left={}, w={:.2f}, h={:.2f})", r.bottom_left, r.width,
                              r.height);
    }
};

template <>
struct std::formatter<geometry::RegularPolygon> {
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::RegularPolygon &p, FormatContext &ctx) {
        return std::format_to(ctx.out(), "RegularPolygon(center={}, r={:.2f}, sides={})", p.center_p, p.radius,
                              p.sides);
    }
};
template <>
struct std::formatter<geometry::Triangle> {
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const geometry::Triangle &t, FormatContext &ctx) {
        return std::format_to(ctx.out(), "Triangle({}, {}, {})", t.A(), t.B(), t.C());
    }
};
template <>
struct std::formatter<geometry::Polygon> {
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

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
