#pragma once
#include "geometry.hpp"
#include <algorithm>
#include <optional>
#include <variant>

namespace geometry::queries {

// Класс-посетитель для вычисления расстояния от точки до фигуры
// Особенности:
// - для линии вычисляется расстояние до ближайшей точки на отрезке
// - для многоугольников (треугольник, прямоугольник, правильный многоугольник): min расстояние до всех его сторон
// - для окружности: max(0, расстояние до центра - радиус)
// - для произвольного многоугольника: min расстояние до всех вершин (упрощенная реализация)
class PointToShapeDistanceVisitor {
public:
    // Параметрический конструктор, принимающий точку, от которой считается расстояние
    explicit PointToShapeDistanceVisitor(const Point2D &p) : point_(p) {}

    // Оператор, реализующий расчет расстояния от точки до отрезка прямой
    [[nodiscard]] double operator()(const Line &line) const {
        // Алгоритм:
        // 1. Находим проекцию точки на бесконечную прямую
        // 2. Ограничиваем параметр t диапазоном [0,1] (точка на отрезке)
        // 3. Вычисляем расстояние от исходной точки до проекции

        Point2D line_vec = line.Dir();              // направляющий вектор отрезка
        Point2D point_vec = point_ - line.Start();  // вектор от начала отрезка к точке

        // Если отрезок вырожден в точку
        if (line_vec.IsZero()) {
            return point_.DistanceTo(line.Start());
        }

        // Находим параметр t проекции на прямую
        double line_length_sq = line_vec.Dot(line_vec);
        double t = point_vec.Dot(line_vec) / line_length_sq;

        // Ограничиваем t диапазоном отрезка [0,1]
        t = std::clamp(t, 0.0, 1.0);

        // Вычисляем точку проекции на отрезке
        Point2D projection = line.Start() + line_vec * t;

        return point_.DistanceTo(projection);
    }

    // Оператор, реализующий расчет расстояния от точки до треугольника
    [[nodiscard]] double operator()(const Triangle &triangle) const { return DistanceToPolygon(triangle.Vertices()); }

    // Оператор, реализующий расчет расстояния от точки до прямоугольника
    [[nodiscard]] double operator()(const Rectangle &rect) const { return DistanceToPolygon(rect.Vertices()); }

    // Оператор, реализующий расчет расстояния от точки до правильного многоугольника
    [[nodiscard]] double operator()(const RegularPolygon &polygon) const {
        return DistanceToPolygon(polygon.Vertices());
    }

    // Оператор, реализующий расчет расстояния от точки до окружности
    [[nodiscard]] double operator()(const Circle &circle) const {
        // Формула: max(0, расстояние до центра - радиус)
        // Если точка внутри окружности, расстояние = 0
        double center_distance = point_.DistanceTo(circle.Center());
        return std::max(0.0, center_distance - circle.Radius());
    }

    // Оператор, реализующий расчет расстояния от точки до произвольного многоугольника
    [[nodiscard]] double operator()(const Polygon &polygon) const { return DistanceToPolygon(polygon.Vertices()); }

private:
    // Общий метод для вычисления расстояния от точки до многоугольника
    // (минимальное расстояние до всех сторон)
    [[nodiscard]] double DistanceToPolygon(std::span<const Point2D> vertices) const {
        double min_distance = std::numeric_limits<double>::max();

        // Вычисляем расстояние от точки до каждой стороны многоугольника и выбираем наименьшее
        for (size_t i = 0; i < vertices.size(); ++i) {
            Line edge{vertices[i], vertices[(i + 1) % vertices.size()]};  // сторона многоугольника
            min_distance = std::min(min_distance, (*this)(edge));         // оператор расчета расстояния "точка-отрезок"
        }

        return min_distance;
    }
    Point2D point_;  // точка, от которой считается расстояние
};

// Класс-посетитель для проверки принадлежности точки фигуре
// Особенности:
// - для отрезка: точка лежит на прямой и между концами
// - для треугольника: метод barycentric coordinates или проверка знаков
// - для прямоугольника: проверка попадания в диапазон координат
// - для правильного многоугольника: метод ray casting (четность пересечений)
// - для окружности: сравнение расстояния с радиусом
// - для произвольного многоугольника: ray casting
class PointInShapeVisitor {
public:
    // Параметрический конструктор, принимающий точку, для которой выполняется проверка
    explicit PointInShapeVisitor(const Point2D &p) : point_(p) {}

    // Оператор, реализующий проверку принадлежности точки отрезку
    [[nodiscard]] bool operator()(const Line &line) const {
        // Условия принадлежности:
        // 1. (point - start) X (end - start) = 0 (коллинеарность)
        // 2. dot(point - start, end - start) принадлежит диапазону [0, |end-start|^2]
        Point2D line_vec = line.Dir();              // направляющий вектор отрезка
        Point2D point_vec = point_ - line.Start();  // вектор от начала отрезка к точке

        double cross = point_vec.Cross(line_vec);
        if (std::abs(cross) > EPS) {
            return false;  // точка не лежит на прямой
        }

        double dot = point_vec.Dot(line_vec);
        double line_length_sq = line_vec.Dot(line_vec);

        return dot >= 0.0 && dot <= line_length_sq;
    }

    // Оператор, реализующий проверку, находится ли точка внутри треугольника
    [[nodiscard]] bool operator()(const Triangle &triangle) const {
        // Используем метод проверки знаков: точка внутри, если она с одной стороны от всех сторон.
        // Для каждой стороны (a,b) проверяем знак векторного произведения (b-a) x (point-a).
        // Если все знаки одинаковы (или нули) - точка внутри.
        Point2D a = triangle.A();
        Point2D b = triangle.B();
        Point2D c = triangle.C();

        double sign1 = (point_ - a).Cross(b - a);
        double sign2 = (point_ - b).Cross(c - b);
        double sign3 = (point_ - c).Cross(a - c);

        bool has_neg = (sign1 < 0.0) || (sign2 < 0.0) || (sign3 < 0.0);
        bool has_pos = (sign1 > 0.0) || (sign2 > 0.0) || (sign3 > 0.0);

        return !(has_neg && has_pos);  // если есть знаки и +, и -, то точка снаружи
    }

    // Оператор, реализующий проверку, находится ли точка внутри прямоугольника
    [[nodiscard]] bool operator()(const Rectangle &rect) const {
        // Проверка: x - в диапазоне [bottom_left.x, top_right.x], y - в диапазоне [bottom_left.y, top_right.y]
        Point2D bl = rect.BottomLeft();
        Point2D tr = rect.TopRight();
        return point_.X() >= bl.X() && point_.X() <= tr.X() && point_.Y() >= bl.Y() && point_.Y() <= tr.Y();
    }

    // Оператор, реализующий проверку, находится ли точка внутри правильного многоугольника
    [[nodiscard]] bool operator()(const RegularPolygon &polygon) const {
        // Используем метод "бросания лучей" (ray casting)
        return point_in_polygon_ray_casting(point_, polygon.Vertices());
    }

    // Оператор, реализующий проверку, находится ли точка внутри окружности
    [[nodiscard]] bool operator()(const Circle &circle) const {
        // Используем квадраты расстояний, чтобы избежать дорогостоящего sqrt
        // Точка внутри окружности, если (x - cx)^2 + (y - cy)^2 <= R^2
        Point2D delta = point_ - circle.Center();

        // Сравниваем квадраты расстояния и радиуса с учетом погрешности
        return delta.Dot(delta) <= circle.Radius() * circle.Radius() + EPS;
    }

    // Оператор, реализующий проверку, находится ли точка внутри произвольного многоугольника
    [[nodiscard]] bool operator()(const Polygon &polygon) const {
        // Используем метод "бросания лучей" (ray casting)
        return point_in_polygon_ray_casting(point_, polygon.Vertices());
    }

private:
    // Метод проверки принадлежности точки многоугольнику (на основе "бросания лучей")
    // (использует std::span для эффективной передачи вершин без копирования)
    bool point_in_polygon_ray_casting(const Point2D &p, std::span<const Point2D> vertices) const {
        // Алгоритм: пускаем луч вправо из точки и считаем количество пересечений с ребрами многоугольника.
        // Если количество пересечений нечетное - точка внутри.
        int intersections = 0;       // число пересечений луча с ребрами
        size_t n = vertices.size();  // количество ребер (вершин) многоугольника

        // Цикл по ребрам многоугольника
        for (size_t i = 0; i < n; ++i) {
            // Вершины ребра многоугольника
            const Point2D &v1 = vertices[i];
            const Point2D &v2 = vertices[(i + 1) % n];

            // Проверяем, лежит ли точка на ребре
            if ((*this)(Line{v1, v2})) {  // используем оператор() для Line
                return true;              // точка на границе считается внутри
            }

            // Пропускаем горизонтальные ребра (они не пересекают горизонтальный луч)
            if (std::abs(v1.Y() - v2.Y()) < EPS) {
                continue;
            }

            // Проверяем, пересекает ли горизонтальный луч ребро многоугольника
            // Условие: y-координата точки строго между y1 и y2
            if ((v1.Y() > p.Y() + EPS) != (v2.Y() > p.Y() + EPS)) {
                // Вычисляем x-координату пересечения горизонтали с ребром
                // Используем линейную интерполяцию: x = x1 + (y - y1) * (x2 - x1) / (y2 - y1)
                double x_intersect = v1.X() + (p.Y() - v1.Y()) * (v2.X() - v1.X()) / (v2.Y() - v1.Y());

                // Если пересечение правее точки, считаем его
                if (x_intersect > p.X() + EPS) {  // добавляем EPS для корректной обработки точек на границе
                    intersections++;
                }
            }
        }

        // Точка внутри (true), если количество пересечений нечетное
        return (intersections % 2) == 1;
    }

    Point2D point_;  // точка, для которой выполняется проверка
};

// Класс-посетитель для вычисления расстояния между двумя фигурами
// Поддерживает комбинации:
// - Circle & Circle
// - Line & Line
// - Для всех остальных комбинаций возвращает std::nullopt
class ShapeToShapeDistanceVisitor {
public:
    // Оператор, реализующий вычисление расстояния между двумя окружностями
    [[nodiscard]] std::optional<double> operator()(const Circle &c1, const Circle &c2) const {
        double centerDistance = c1.Center().DistanceTo(c2.Center());
        return std::max(0.0, centerDistance - c1.Radius() - c2.Radius());
    }

    // Оператор, реализующий вычисление расстояния между двумя отрезками
    // (возвращает min расстояние между всеми парами точек)
    [[nodiscard]] std::optional<double> operator()(const Line &l1, const Line &l2) const {
        using PSDist = queries::PointToShapeDistanceVisitor;
        std::vector<double> distances = {
            PSDist{l1.Start()}(l2), PSDist{l1.End()}(l2),   // вычисляем расстояния от концов первого отрезка до второго
            PSDist{l2.Start()}(l1), PSDist{l2.End()}(l1)};  // вычисляем расстояния от концов второго отрезка до первого
        return *std::ranges::min_element(distances);
    }

    // Шаблонный оператор для всех неподдерживаемых комбинаций фигур
    // Возвращает std::nullopt, так как расстояние не может быть вычислено
    // (не выбрасывает исключение, т.к. отсутствие расстояния - это допустимая ситуация)
    template <typename T, typename U>
    [[nodiscard]] std::optional<double> operator()(const T &, const U &) const {
        return std::nullopt;
    }
};

// === Функции-помощники ===

// Вычисляет расстояние от точки до фигуры
inline double DistanceToPoint(const Shape &shape, const Point2D &point) {
    return std::visit(PointToShapeDistanceVisitor{point}, shape);
}

// Возвращает ограничивающий бокс (bounding box) фигуры
// (каждая фигура имеет метод BoundBox(), возвращающий ограничивающий бокс)
inline BoundingBox GetBoundBox(const Shape &shape) {
    // std::visit с мультилямбдой, вызывающей метод BoundBox() для каждой фигуры
    return std::visit([](const auto &s) { return s.BoundBox(); }, shape);
}

// Возвращает высоту (max Y-координату) фигуры
inline double GetHeight(const Shape &shape) {
    // std::visit с мультилямбдой, вызывающей метод MaxY() для каждой фигуры
    return std::visit([](const auto &s) { return s.MaxY(); }, shape);
}

// Проверяет, пересекаются ли ограничивающие боксы двух фигур
// (быстрая предварительная проверка перед точным вычислением пересечения)
inline bool BoundingBoxesOverlap(const Shape &shape1, const Shape &shape2) {
    BoundingBox bb1 = GetBoundBox(shape1);
    BoundingBox bb2 = GetBoundBox(shape2);
    return bb1.Overlaps(bb2);
}

// Вычисляет расстояние между двумя фигурами
// (если расстояние не может быть вычислено, то возвращает std::nullopt)
inline std::optional<double> DistanceBetweenShapes(const Shape &shape1, const Shape &shape2) {
    return std::visit(ShapeToShapeDistanceVisitor{}, shape1, shape2);
}

}  // namespace geometry::queries