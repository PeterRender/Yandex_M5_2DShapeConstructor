#include "convex_hull.hpp"
#include <algorithm>
#include <stdexcept>

namespace geometry::convex_hull {

/**
 * @brief Функция-хелпер, вычисляющая векторное произведение (p1 - middle) × (p2 - middle)
 *
 * Используется в алгоритме Грэхема для определения ориентации трех точек.
 *
 * @param p1 первая точка
 * @param middle центральная точка (вершина угла)
 * @param p2 вторая точка
 * @return векторное произведение векторов (p1 - middle) и (p2 - middle):
 *  - "+" значение: поворот от p1 к p2 относительно middle выполняется против часовой стрелки
 *  - "-" значение: поворот по часовой стрелке
 *  - 0: точки коллинеарны
 *
 * @note Функция помечена как noexcept, так как все вызываемые операции (вычитание Point2D, Cross) noexcept
 */
[[nodiscard]] double CrossProduct(const Point2D &p1, const Point2D &middle, const Point2D &p2) noexcept {
    auto new_p1 = p1 - middle;
    auto new_p2 = p2 - middle;
    return new_p1.Cross(new_p2);
}

/**
 * @brief Функция, реализующая построение выпуклой оболочки с помощью алгоритма Грэхема
 *
 * Сложность: O(N log N), где N - количество точек.
 *
 * @param points массив точек (изменяется внутри функции ввиду сортировки)
 * @return std::expected:
 *  - при успехе: вектор точек выпуклой оболочки в порядке обхода против часовой стрелки
 *  - при ошибке: строка с описанием проблемы
 *
 * @note Функция помечена как noexcept - все ошибки обрабатываются через std::expected, исключения не выбрасываются
 */
[[nodiscard]] std::expected<std::vector<Point2D>, std::string> GrahamScan(std::span<Point2D> points) noexcept {
    // Идея алгоритма:
    // 1. Находим точку с min y (и x при равенстве) - начальную точку оболочки
    // 2. Сортируем остальные точки по полярному углу относительно начальной
    // 3. Строим оболочку с помощью стека, удаляя точки, дающие правый поворот

    // Проверка на min допустимое количество точек
    if (points.size() < 3) {
        return std::unexpected("At least three points are required for convex hull.");
    }

    // 1. Находим итератор на самую нижнюю левую точку (точку с min y, а при равенстве - min x)
    // Используем ranges::min_element с кастомным компаратором, учитывающим погрешность double
    auto smallest_it = std::ranges::min_element(points, [](const Point2D &a, const Point2D &b) {
        // Сначала сравниваем y-координаты с учетом погрешности
        if (std::abs(a.Y() - b.Y()) > EPS) {
            return a.Y() < b.Y();  // та, у которой y меньше
        }
        // Если y почти равны, сравниваем x-координаты с учетом погрешности
        if (std::abs(a.X() - b.X()) > EPS) {
            return a.X() < b.X();  // та, у которой x меньше
        }
        // Точки равны в пределах погрешности
        return false;
    });

    // Меняем местами найденную точку с началом массива
    // (теперь points[0] - стартовая точка для алгоритма Грэхема)
    std::swap(points[0], *smallest_it);
    const Point2D &start_point = points[0];

    // 2. Сортируем остальные точки по полярному углу относительно start_point
    // (используем std::sort с кастомным компаратором, который не бросает исключений)
    std::sort(points.begin() + 1, points.end(), [&start_point](const Point2D &p1, const Point2D &p2) noexcept {
        double cross = CrossProduct(p1, start_point, p2);

        // Если коллинеарны, берем ближайшую по радиусу
        if (std::abs(cross) < EPS) {
            return start_point.DistanceTo(p1) < start_point.DistanceTo(p2);
        }
        return cross > 0;  // "+" значение - это поворот против часовой стрелки
    });

    // 3. Строим выпуклую оболочку с помощью стека StackForGrahamScan
    // (метод Push может бросить bad_alloc, перехватываем его и обрабатываем через std::unexpected)
    try {
        StackForGrahamScan hull;

        // Цикл по отсортированному массиву точек
        for (const auto &new_p : points) {
            // Удаляем точки при правом повороте или коллинеарности (cross <= EPS)
            // Оставляем только точки, дающие левый поворот (cross > EPS)
            while (hull.Size() > 1 && CrossProduct(hull.Top(), hull.NextToTop(), new_p) <= EPS) {
                hull.Pop();
            }
            hull.Push(new_p);  // единственное место, где может быть bad_alloc
        }

        return hull.Extract();

    } catch (const std::bad_alloc &) {
        return std::unexpected("Failed to allocate memory during convex hull computation.");
    }
}

}  // namespace geometry::convex_hull