#pragma once
#include "geometry.hpp"
#include <expected>
#include <vector>

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
 */
[[nodiscard]] double CrossProduct(const Point2D &p1, const Point2D &middle, const Point2D &p2) noexcept;

// Класс-обертка, реализующий стек для построения выпуклой оболочки с помощью алгоритма Грэхема
class StackForGrahamScan {
public:
    // Конструктор по умолчанию
    StackForGrahamScan() = default;

    // Добавляет точку на вершину стека (может бросить bad_alloc)
    void Push(const Point2D &p) { s.push_back(p); }

    // Удаляет точку с вершины стека
    // (исключение не бросает, но безопасен, только если стек не пуст)
    void Pop() noexcept { s.pop_back(); }

    // Возвращает количество элементов в стеке
    [[nodiscard]] size_t Size() const noexcept { return s.size(); }

    // Возвращает точку на вершине стека (последний добавленный элемент)
    [[nodiscard]] const Point2D &Top() const noexcept { return s.back(); }

    // Возвращает точку, находящуюся под вершиной стека (предпоследний элемент)
    // (используется в алгоритме Грэхема для проверки ориентации трех точек)
    [[nodiscard]] const Point2D &NextToTop() const noexcept { return *std::prev(s.end(), 2); }

    // Извлекает внутренний вектор с перемещением в результат (после вызова стек становится пустым)
    // (используется для возврата результата работы алгоритма Грэхема)
    [[nodiscard]] std::vector<Point2D> &&Extract() noexcept { return std::move(s); }

private:
    std::vector<Point2D> s;  // внутреннее хранилище стека
};

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
[[nodiscard]] std::expected<std::vector<Point2D>, std::string> GrahamScan(std::span<Point2D> points) noexcept;

}  // namespace geometry::convex_hull