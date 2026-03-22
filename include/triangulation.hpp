#pragma once
#include "geometry.hpp"
#include <algorithm>
#include <expected>
#include <format>
#include <ranges>
#include <set>
#include <stdexcept>
#include <vector>

namespace geometry::triangulation {

namespace rs = std::ranges;  // псевдоним пространства имен диапазонов

// Класс треугольника триангуляции Делоне
class DelaunayTriangle {
public:
    // Параметрический конструктор, инициализирующий вершины треугольника
    constexpr DelaunayTriangle(const Point2D &a, const Point2D &b, const Point2D &c) noexcept
        : a_(a), b_(b), c_(c), circumcenter_(CalcCircumcenter()), circumradius_sq_(CalcCircumradiusSq()) {}

    // === Методы доступа к данным-членам (только чтение) ===
    [[nodiscard]] constexpr const Point2D &A() const noexcept { return a_; }
    [[nodiscard]] constexpr const Point2D &B() const noexcept { return b_; }
    [[nodiscard]] constexpr const Point2D &C() const noexcept { return c_; }
    [[nodiscard]] constexpr const Point2D &Circumcenter() const noexcept { return circumcenter_; }
    [[nodiscard]] constexpr double CircumradiusSq() const noexcept { return circumradius_sq_; }

    // Перегруженный оператор равенства
    [[nodiscard]] constexpr bool operator==(const DelaunayTriangle &other) const noexcept {
        return a_ == other.a_ && b_ == other.b_ && c_ == other.c_;
    }

    // Возвращает все вершины в виде массива для удобной итерации
    [[nodiscard]] constexpr std::array<Point2D, 3> Vertices() const noexcept { return {a_, b_, c_}; }

    // Проверяет, содержит ли описанная окружность треугольника данную точку
    // (использует квадраты расстояний для избежания std::sqrt и повышения точности)
    [[nodiscard]] constexpr bool ContainsPoint(const Point2D &p) const {
        Point2D diff = p - circumcenter_;
        double dist_sq = diff.X() * diff.X() + diff.Y() * diff.Y();
        return dist_sq <= circumradius_sq_ + EPS;
    }

    // Проверяет, имеют ли два треугольника общее ребро
    [[nodiscard]] constexpr bool SharesEdge(const DelaunayTriangle &other) const {
        std::array<Point2D, 3> this_points = {a_, b_, c_};
        std::array<Point2D, 3> other_points = {other.a_, other.b_, other.c_};

        int shared_count = 0;
        for (const Point2D &p1 : this_points) {
            for (const Point2D &p2 : other_points) {
                if (std::abs(p1.X() - p2.X()) < EPS && std::abs(p1.Y() - p2.Y()) < EPS) {
                    shared_count++;
                    break;
                }
            }
        }

        return shared_count == 2;
    }

private:
    // Вычисляет центр описанной окружности треугольника
    // Алгоритм: решение системы уравнений для точек на окружности
    // (x - x0)^2 + (y - y0)^2 = R^2 для трех точек
    [[nodiscard]] constexpr Point2D CalcCircumcenter() const {
        double d = 2.0 * (a_.X() * (b_.Y() - c_.Y()) + b_.X() * (c_.Y() - a_.Y()) + c_.X() * (a_.Y() - b_.Y()));

        // Вырожденный случай - возвращаем центр тяжести
        if (std::abs(d) < EPS) {
            return {(a_.X() + b_.X() + c_.X()) / 3, (a_.Y() + b_.Y() + c_.Y()) / 3.0};
        }

        double ux = ((a_.X() * a_.X() + a_.Y() * a_.Y()) * (b_.Y() - c_.Y()) +
                     (b_.X() * b_.X() + b_.Y() * b_.Y()) * (c_.Y() - a_.Y()) +
                     (c_.X() * c_.X() + c_.Y() * c_.Y()) * (a_.Y() - b_.Y())) /
                    d;

        double uy = ((a_.X() * a_.X() + a_.Y() * a_.Y()) * (c_.X() - b_.X()) +
                     (b_.X() * b_.X() + b_.Y() * b_.Y()) * (a_.X() - c_.X()) +
                     (c_.X() * c_.X() + c_.Y() * c_.Y()) * (b_.X() - a_.X())) /
                    d;

        return {ux, uy};
    }

    // Вычисляет квадрат радиуса описанной окружности
    [[nodiscard]] constexpr double CalcCircumradiusSq() const {
        Point2D diff = a_ - circumcenter_;
        return diff.X() * diff.X() + diff.Y() * diff.Y();
    }

    Point2D a_;  // вершины треугольника
    Point2D b_;
    Point2D c_;

    Point2D circumcenter_;    // кэшированный центр описанной окружности
    double circumradius_sq_;  // кэшированный квадрат радиуса описанной окружности
};

// Класс ребра для алгоритма триангуляции
// (используется для хранения уникальных ребер в процессе построения триангуляции)
class Edge {
public:
    // Конструктор, принимающий две вершины ребра
    // (выполняет нормализацию: p1 всегда "меньше" p2 для корректного сравнения)
    constexpr Edge(const Point2D &p1, const Point2D &p2) noexcept : p1_(p1), p2_(p2) {
        // Нормализуем порядок вершин: p1 должна быть меньше p2 (лексикографически)
        if (p2_ < p1_) {  // используем operator< точки
            std::swap(p1_, p2_);
        }
    }

    // === Методы доступа к данным-членам (только чтение) ===
    [[nodiscard]] constexpr const Point2D &P1() const noexcept { return p1_; }
    [[nodiscard]] constexpr const Point2D &P2() const noexcept { return p2_; }

    // Оператор сравнения ребер
    // (ребра сравниваются лексикографически: сначала по p1, затем по p2)
    [[nodiscard]] constexpr bool operator<(const Edge &other) const noexcept {
        if (p1_ < other.p1_)
            return true;
        if (other.p1_ < p1_)
            return false;
        return p2_ < other.p2_;
    }

    // Оператор равенства
    [[nodiscard]] constexpr bool operator==(const Edge &other) const noexcept {
        return p1_ == other.p1_ && p2_ == other.p2_;
    }

private:
    Point2D p1_;  // первая вершина ребра (гарантированно "меньше" p2_)
    Point2D p2_;  // вторая вершина ребра
};

/**
 * @brief Функция, реализующая построение триангуляции Делоне
 *
 * Сложность: O(N^2) в худшем случае, O(N log N) в среднем.
 *
 * @param points массив точек для триангуляции
 * @return std::expected:
 *  - при успехе: вектор треугольников Делоне
 *  - при ошибке: строка с описанием проблемы
 *
 * @note Функция помечена как noexcept - все ошибки обрабатываются через std::expected, исключения не выбрасываются
 */
[[nodiscard]] inline std::expected<std::vector<DelaunayTriangle>, std::string>
DelaunayTriangulation(std::span<const Point2D> points) noexcept {
    // Проверка на минимальное количество точек
    if (points.size() < 3) {
        return std::unexpected("At least three points are required for triangulation.");
    }

    // Находим границы всех точек за один проход
    Point2D min_p = points[0];
    Point2D max_p = points[0];
    std::ranges::for_each(points, [&](const Point2D &p) {
        min_p = Point2D::Min(min_p, p);  // работает copy elision, прямая инициализация
        max_p = Point2D::Max(max_p, p);
    });

    // Вычисляем размер и центр ограничивающего квадрата, в котором лежат все точки
    double dmax = std::max(max_p.X() - min_p.X(), max_p.Y() - min_p.Y());
    Point2D center = {(min_p.X() + max_p.X()) * 0.5, (min_p.Y() + max_p.Y()) * 0.5};

    // Создаем супер-треугольник, охватывающий все точки (равносторонний треугольник, описанный вокруг квадрата)
    // Размер выбираем достаточно большим, чтобы все точки были внутри
    const double SUPER_SIZE = 20.0;  // эмпирич. коэфф-т для гарантированного охвата (тр-к в 20 раз > области с точками)
    const std::array<Point2D, 3> super_vertices = {
        Point2D{center.X() - SUPER_SIZE * dmax, center.Y() - dmax},   // левая нижняя (ниже и левее)
        Point2D{center.X(), center.Y() + SUPER_SIZE * dmax},          // верхняя (выше)
        Point2D{center.X() + SUPER_SIZE * dmax, center.Y() - dmax}};  // правая нижняя (ниже и правее)

    // Пытаемся построить триангуляцию Делоне
    // (может выброситься bad_alloc, перехватываем его и обрабатываем через std::unexpected)
    try {
        std::vector<DelaunayTriangle> triangles;  // хранилище треугольников Делоне
        // Примечание: алгоритм в процессе работы создает примерно в 2 раза больше треугольников, чем исходных точек
        triangles.reserve(points.size() * 2);  // предварительное выделение памяти
        triangles.emplace_back(super_vertices[0], super_vertices[1],
                               super_vertices[2]);  // создаем сразу в памяти вектора

        // Основной цикл алгоритма построения триангуляции
        for (const Point2D &point : points) {
            std::vector<const DelaunayTriangle *> bad_triangles;
            std::set<Edge> polygon;

            // Находим все треугольники, описанная окружность которых содержит новую точку (это bad_triangles)
            // - Эти треугольники образуют "дырку" в триангуляции
            // - Нам нужно найти границу этой дырки - это будет полигон из ребер
            for (const auto &triangle : triangles) {
                if (triangle.ContainsPoint(point)) {
                    bad_triangles.push_back(&triangle);

                    // Обрабатываем три ребра "плохого" треугольника:
                    // - Пытаемся добавить ребро в полигон с помощью emplace (создание на месте)
                    // - Если ребро уже было в полигоне (значит, это общее ребро между двумя плохими треугольниками),
                    // тогда удаляем его - это внутреннее ребро, оно не должно быть частью границы дырки
                    auto [it1, inserted1] = polygon.emplace(triangle.A(), triangle.B());
                    if (!inserted1)
                        polygon.erase(it1);
                    auto [it2, inserted2] = polygon.emplace(triangle.B(), triangle.C());
                    if (!inserted2)
                        polygon.erase(it2);
                    auto [it3, inserted3] = polygon.emplace(triangle.C(), triangle.A());
                    if (!inserted3)
                        polygon.erase(it3);
                }
            }

            // Если нет плохих треугольников, то точка уже находится внутри триангуляции и не требует изменений -
            // переходим к следующей точке
            if (bad_triangles.empty()) {
                continue;
            }

            // Удаляем из общего списка triangles все треугольники, которые есть в списке bad_triangles
            std::erase_if(triangles, [&bad_triangles](const DelaunayTriangle &t) {
                return rs::any_of(bad_triangles, [&t](const DelaunayTriangle *bad) { return t == *bad; });
            });

            // Создаем новые треугольники из ребер полигона и новой точки
            for (const Edge &edge : polygon) {
                triangles.emplace_back(edge.P1(), edge.P2(), point);
            }
        }

        // Лямбда, проверяющая, является ли вершина частью супер-треугольника
        auto is_super_vertex = [&super_vertices](const Point2D &p) -> bool {
            return std::ranges::any_of(super_vertices, [&p](const Point2D &v) {
                return p == v;  // используем operator== для Point2D (с учетом EPS)
            });
        };

        // Удаляем треугольники, содержащие любую вершину супер-треугольника
        std::erase_if(triangles, [&is_super_vertex](const DelaunayTriangle &t) {
            return is_super_vertex(t.A()) || is_super_vertex(t.B()) || is_super_vertex(t.C());
        });

        triangles.shrink_to_fit();  // уменьшаем capacity до реального размера

        return triangles;
    } catch (const std::bad_alloc &) {
        return std::unexpected("Failed to allocate memory during triangulation.");
    }
}

}  // namespace geometry::triangulation

// Специализация шаблона std::formatter для типа DelaunayTriangle
// (чтобы использовать его в std::format и std::print)
template <>
struct std::formatter<geometry::triangulation::DelaunayTriangle> {
    // Метод, выполняющий парсинг спецификаторов формата (значения в скобках {})
    constexpr auto parse(std::format_parse_context &ctx) {
        return ctx.begin();  // игнорируем спецификаторы и возвращаем начало строки формата
    }

    // Шаблонный метод-инструкция, отвечающий за преобразование значения DelaunayTriangle в строку
    // (параметр FormatContext задает, куда выводить результат)
    template <typename FormatContext>
    auto format(const geometry::triangulation::DelaunayTriangle &t, FormatContext &ctx) const {
        return std::format_to(ctx.out(), "DelaunayTriangle({}, {}, {})", t.A(), t.B(), t.C());
    }
};
