#include "convex_hull.hpp"
#include "geometry.hpp"
#include "intersections.hpp"
#include "queries.hpp"
#include "shape_utils.hpp"
#include "triangulation.hpp"
#include "visualization.hpp"

#include <algorithm>
#include <limits>
#include <print>
#include <ranges>

using namespace geometry;

namespace rv = std::ranges::views;  // псевдоним пространства имен отображений
namespace rs = std::ranges;         // псевдоним пространства имен диапазонов

/**
 * @brief Функция, выводящая информацию о пересечениях заданной фигуры с другими фигурами
 *
 * @param shape фигура, для которой проверяются пересечения (индекс 0 в выводе)
 * @param others список других фигур для проверки пересечений
 *
 * Алгоритм:
 * 1. Для каждой фигуры из others пытаемся найти точку пересечения с shape
 * 2. Если комбинация фигур не поддерживается IntersectionVisitor, пропускаем её
 * 3. Результаты (пересечение есть/нет) форматируем в строки
 * 4. Выводим все строки
 */
void PrintAllIntersections(const Shape &shape, std::span<const Shape> others) {
    std::println("\n=== Intersections ===");

    /*
     * Используйте ranges чтобы оставить только фигуры,
     * поддерживающие возможность находить пересечения между собой
     *
     * Затем примените монадический интерфейс для обработки результатов:
     *     - Пересечение найдено в точке A между фигурами B и C
     *     - Фигуры B и C не пересекаются
     */

    // Псевдоним результата обработки комбинации фигур {индекс второй фигуры, результат пересечения}
    using ShapeResult = std::pair<size_t, std::optional<Point2D>>;

    // Маркер для неподдерживаемых комбинаций (индекс вне диапазона фигур)
    constexpr size_t INVALID_INDEX = std::numeric_limits<size_t>::max();

    // Формируем отображение комбинаций фигур в строки результатов
    auto output_strings =
        others | rv::enumerate |  // добавляем индексы фигур
        rv::transform([&](const auto &pair) -> ShapeResult {
            const auto &[idx, other] = pair;
            try {
                auto point_opt = intersections::GetIntersectPoint(shape, other);
                return std::make_pair(idx, point_opt);  // поддерживаемая комбинация
            } catch (const std::logic_error &) {
                return std::make_pair(INVALID_INDEX, std::nullopt);  // неподдерживаемая комбинация
            }
        }) |
        rv::filter([&](const auto &pair) {  // отсеиваем неподдерживаемые комбинации
            return pair.first != INVALID_INDEX;
        }) |
        rv::transform([](const auto &pair) -> std::string {  // преобразуем результаты в строки вывода
            const auto &[idx, point_opt] = pair;
            return point_opt  // формируем строку с результатом с помощью цепочки монад
                .transform([idx](const Point2D &point) {
                    return std::format("  Intersection found at point {} between shape 0 and shape {}", point, idx + 1);
                })
                .value_or(std::format("  Shape 0 and shape {} do not intersect", idx + 1));
        });

    // Выводим строки с результатами
    rs::for_each(output_strings, [](const std::string &line) { std::println("{}", line); });
}

void PrintDistancesFromPointToShapes(Point2D p, std::span<const Shape> shapes) {
    std::println("\n=== Distance from Point Test ===");

    /*
     * Используйте ranges чтобы выбрать любые 5 фигур из списка.
     * Затем найдите расстояния от заданной точки до всех выбранных фигур.
     * Выведите результат в формате "Расстояние от точки P до фигуры S равно D"
     */
}

void PerformShapeAnalysis(std::span<const Shape> shapes) {
    std::println("\n=== Shape Analysis ===");

    /*
     * Используйте ranges и созданные классы чтобы:
     *     - Найти все пересечения между фигурами используя метод Bounding Box
     *     - Найти самую высокую фигуру (чья высота наибольшая)expected
     *     - Вывести расстояние между любыми двумя фигурами, которые поддерживают данную функциональность
     */
}

void PerformExtraShapeAnalysis(std::span<const Shape> shapes) {
    std::println("\n=== Shape Extra Analysis ===");

    /*
     * Используйте ranges и созданные классы чтобы:
     *     - Вывести 3 любые фигуры, которые находятся выше 50.0
     *     - Вывести фигуры с наименьшей и с наибольшей высотами
     */
}

int main() {
    std::vector<Shape> shapes = utils::ParseShapes("circle 0 0 1.5; line 1 2 3 4; polygon 0 0 2 5; triangle 0 0 1 0 "
                                                   "0.5 1; polygon 0 0 1 2; badshape; circle 0 0 -1");
    std::println("Parsed {} shapes", shapes.size());

    // Выведите индекс каждой фигуры и её высоту

    //
    // Вызываем разработанные функции
    //
    PrintAllIntersections(shapes[0], shapes);

    PrintDistancesFromPointToShapes(Point2D{10.0, 10.0}, shapes);

    PerformShapeAnalysis(shapes);

    PerformExtraShapeAnalysis(shapes);

    //
    // Рисуем все фигуры
    //
    // Важно: после изучения графика - нажмите Enter чтобы продолжить выполнение и построить 2ой график
    //
    geometry::visualization::Draw(shapes);

    //
    // Формируем список из вершин всех фигур
    //
    std::vector<Point2D> points;

    /* ваш код здесь */

    //
    // Находим список точек, для построения выпуклой оболочки - convex hull - алгоритмом Грэхема
    // Создаём из них объект класса `Polygon` и добавляем его в список shapes
    // Рисуем все фигуры
    //

    /* ваш код здесь */

    //
    // после изучения графика - нажмите Enter чтобы продолжить выполнение и построить 3ий график
    //

    {
        std::vector<Point2D> points = {{0, 0}, {10, 0}, {5, 8}, {15, 5}, {2, 12}};

        //
        // Используйте список точек points или свой, чтобы
        // выполнить алгоритм триангуляции Делоне алгоритмом Боуэра-Ватсона
        //
        // После успешного завершения алгоритма - выведите результат для проверки
        // используя geometry::visualization::Draw
        //
    }
    return 0;
}