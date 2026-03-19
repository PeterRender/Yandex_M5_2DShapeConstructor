#pragma once
#include "geometry.hpp"
#include <cmath>
#include <optional>

namespace geometry::intersections {

// Класс-посетитель для поиска пересечений между двумя фигурами.
// Реализует двойную диспетчеризацию через std::visit.
// Поддерживаются только следующие комбинации:
// - Line & Line (пересечение двух отрезков)
// - Line & Circle (пересечение отрезка с окружностью)
// - Circle & Circle (пересечение двух окружностей)
// Для всех остальных комбинаций выбрасывается std::logic_error с сообщением о неподдерживаемой операции
class IntersectionVisitor {
public:
    // Оператор, реализующий поиск пересечения для Line и Line
    [[nodiscard]] std::optional<Point2D> operator()(const Line &l1, const Line &l2) const {
        // Используем алгоритм пересечения двух отрезков, основанный на решении системы параметрических уравнений:
        // P = l1.start + t * dir1, где dir1 = (l1.end - l1.start), а t из диапазона [0,1]
        // P = l2.start + u * dir2, где dir2 = (l2.end - l2.start), а u из диапазона [0,1]

        Point2D dir1 = l1.Dir();  // направляющий вектор первого отрезка
        Point2D dir2 = l2.Dir();  // направляющий вектор второго отрезка

        double dir1_dir2_cross = dir1.Cross(dir2);  // векторное произведение dir1 x dir2

        // Если v1_v2_cross близко к нулю, отрезки параллельны или коллинеарны
        if (std::abs(dir1_dir2_cross) < EPS) {
            return std::nullopt;  // нет пересечения (или бесконечно много - не рассматриваем)
        }

        // Вычисляем параметры t и u для точки пересечения:
        // l1.start + t * dir1 = l2.start + u * dir2, откуда:
        // t = (diff x dir2) / (dir1 x dir2), u = (diff x dir1) / (dir1 x dir2), где diff = l2.start - l1.start
        Point2D diff = l2.Start() - l1.Start();
        double t = diff.Cross(dir2) / dir1_dir2_cross;
        double u = diff.Cross(dir1) / dir1_dir2_cross;

        // Проверяем, что точка пересечения лежит внутри обоих отрезков
        if (in_range(t) && in_range(u)) {
            return l1.Start() + dir1 * t;  // возвращаем точку пересечения
        }

        return std::nullopt;  // пересечения нет
    }

    // Оператор, реализующий поиск пересечения для Line и Circle
    [[nodiscard]] std::optional<Point2D> operator()(const Line &line, const Circle &circle) const {
        // Решаем квадратное уравнение для пересечения прямой с окружностью
        // Подставляем параметрическое уравнение прямой в уравнение окружности:
        // P = line.start + t*dir, где dir = (line.end - line.start), а t из диапазона [0,1]
        // |P - C|^2 = R^2, где С - центр, R - радиус
        // |line.start + t*dir - C|^2 = R^2
        // |center_to_start + t*dir|^2 = R^2, где center_to_start = line.start - C
        // dot(center_to_start + t*dir, center_to_start + t*dir) = R^2, т.к. |V|^2 = dot(V,V)
        // dot(dir, dir) * t^2 + 2.0 * dot(center_to_start, dir) * t + (dot(center_to_start, center_to_start) - R^2) = 0

        Point2D dir = line.Dir();                                  // направление отрезка
        Point2D center_to_start = line.Start() - circle.Center();  // вектор от центра окружности к началу отрезка

        // Считаем свободный член "с" квадратного уравнения: a*t^2 + b*t + c = 0
        double c = center_to_start.Dot(center_to_start) - circle.Radius() * circle.Radius();

        // Особый случай: отрезок вырожден в точку
        if (dir.IsZero()) {
            // Точка лежит на окружности, если c == 0 (расстояние до центра = радиус)
            return (std::abs(c) < EPS) ? std::make_optional(line.Start()) : std::nullopt;
        }

        // Считаем коэффициенты "a" и "b" квадратного уравнения: a*t^2 + b*t + c = 0
        double a = dir.Dot(dir);  // всегда > 0 для невырожденного отрезка
        double b = 2.0 * center_to_start.Dot(dir);
        double D = b * b - 4.0 * a * c;  // считаем дискриминант

        // Если дискриминант отрицательный, пересечений нет
        if (D < -EPS) {
            return std::nullopt;
        }

        // Если дискриминант близок к нулю, есть одна потенциальная точка касания
        if (std::abs(D) < EPS) {
            double t = -b / (2.0 * a);
            return in_range(t) ? std::make_optional(line.Start() + dir * t) : std::nullopt;
        }

        // Дискриминант положительный - две потенциальные точки пересечения
        double sqrt_disc = std::sqrt(D);
        double t1 = (-b - sqrt_disc) / (2.0 * a);  // находим параметр первой точки
        // Проверяем, что первая точка попадает в отрезок [0,1]
        if (in_range(t1)) {
            return line.Start() + dir * t1;
        }
        // Если первая точка не подходит (когда начало отрезка внутри окружности, t1 < 0), то проверяем вторую
        double t2 = (-b + sqrt_disc) / (2.0 * a);  // находим параметр второй точки
        if (in_range(t2)) {
            return line.Start() + dir * t2;
        }

        return std::nullopt;  // ни одна точка не попала в отрезок
    }

    // Оператор, реализующий поиск пересечения для Circle и Circle
    [[nodiscard]] std::optional<Point2D> operator()(const Circle &c1, const Circle &c2) const {
        // Работаем с квадратами расстояний, чтобы избежать дорогостоящего sqrt
        Point2D delta = c2.Center() - c1.Center();  // вектор направления из c1 в c2
        double dist_sq = delta.Dot(delta);          // квадрат расстояния между центрами c1 и c2
        double r1 = c1.Radius();
        double r2 = c2.Radius();

        double r_sum = r1 + r2;              // сумма радиусов
        double r_sum_sq = r_sum * r_sum;     // квадрат суммы радиусов
        double r_diff = std::abs(r1 - r2);   // модуль разности радиусов
        double r_diff_sq = r_diff * r_diff;  // квадрат модуля разности радиусов

        // Быстрая проверка на особые случаи (без вычисления корня)
        if ((dist_sq < EPS) ||             // окружности концентричны (бесконечно много точек пересечения или их нет)
            (dist_sq > r_sum_sq + EPS) ||  // окружности слишком далеки друг от друга
            (dist_sq < r_diff_sq - EPS))   // одна окружность внутри другой без касания
        {
            return std::nullopt;  // нет пересечения
        }

        // Флаг касания окружностей (внешнего или внутреннего), вычисляется без sqrt - более точно
        bool is_tangent = (std::abs(dist_sq - r_sum_sq) < EPS) || (std::abs(dist_sq - r_diff_sq) < EPS);

        // Пересечение есть, теперь вычисляем расстояние между центрами
        double dist = std::sqrt(dist_sq);
        Point2D v = delta / dist;  // единичный вектор направления из c1 в c2

        // Проверка на касание окружностей (делается без sqrt - более точно)
        if ((std::abs(dist_sq - r_sum_sq) < EPS) ||  // внешнее касание
            (std::abs(dist_sq - r_diff_sq) < EPS))   // внутреннее касание
        {
            return c1.Center() + v * r1;  // точка касания лежит на линии центров
        }

        // Полное пересечение окружностей - 2 точки.
        // В каждой точке пересечения P выполняются два уравнения: |P-O1|^2 = R1^2, |P-O2|^2 = R2^2
        // Проекция точки P на O2O1: Ppr = O1 + d1*u, где u = (O2-O1)/d - единичный вектор, а d = |O2-O1|
        // По т. Пифагора для каждой окружности: R1^2 = d1^2 + h^2, R2^2 = (d-d1)^2 + h^2
        // Вычитаем и решаем относительно d1: d1 = (R1^2 - R2^2 + d^2) / (2d)
        double d1 = (r1 * r1 - r2 * r2 + dist_sq) / (2.0 * dist);
        double h = std::sqrt(r1 * r1 - d1 * d1);  // половина хорды пересечения c1 и c2
        Point2D perp = Point2D{-v.Y(), v.X()};    // единичный вектор, перпендикулярный линии центров

        // Возвращаем точку "выше" линии центров
        return c1.Center() + v * d1 + perp * h;
    }

    // Оператор для любых других комбинаций типов
    template <typename T, typename U>
    [[nodiscard]] std::optional<Point2D> operator()(const T &, const U &) const {
        throw std::logic_error("Intersection between " + std::string(typeid(T).name()) + " and " +
                               std::string(typeid(U).name()) + " is not supported");
    }

private:
    // Статический вспомогательный метод для проверки принадлежности отрезку [0,1]
    static constexpr bool in_range(double value) noexcept { return value >= -EPS && value <= 1.0 + EPS; }
};

// Вспомогательная функция для поиска точки пересечения двух фигур
// Для поддерживаемых комбинаций фигур возвращает точку пересечения или std::nullopt, если фигуры не пересекаются
// Для неподдерживаемых комбинаций фигур выбрасывает std::logic_error с сообщением о неподдерживаемой операции
inline std::optional<Point2D> GetIntersectPoint(const Shape &shape1, const Shape &shape2) {
    // Для поиска пересечений используется паттерн "посетителя" с двойной диспетчеризацией
    return std::visit(IntersectionVisitor{}, shape1, shape2);
}

}  // namespace geometry::intersections