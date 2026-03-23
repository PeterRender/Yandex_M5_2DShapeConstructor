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
 * @brief Функция, выводящая индексы и высоты всех фигур
 *
 * @param shapes список фигур для анализа
 */
void PrintShapeHeights(std::span<const Shape> shapes) {
    std::println("\n=== Shape Heights ===");

    // Формируем строки пар {индекс фигуры, высота} с помощью композиции отображений
    auto height_lines = shapes | rv::enumerate |                             // добавляем индексы фигур
                        rv::transform([](const auto &pair) -> std::string {  // преобразуем пары в строки
                            const auto &[idx, shape] = pair;
                            return std::format("  Shape {}: height = {:.4f}", idx, queries::GetHeight(shape));
                        });
    // Выводим строки с результатами
    rs::for_each(height_lines, [](const std::string &line) { std::println("{}", line); });
}

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

    // Проверяем, что есть фигуры для теста
    if (others.empty()) {
        std::println("  No shapes to test");
        return;
    }

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

/**
 * @brief Функция, выводящая расстояния от заданной точки до первых 5 фигур
 *
 * @param p точка, от которой вычисляются расстояния
 * @param shapes список фигур для анализа
 *
 * Алгоритм:
 * 1. Берем первые 5 фигур из списка (или меньше, если фигур недостаточно)
 * 2. Для каждой фигуры вычисляем расстояние до точки
 * 3. Выводим результат в формате "Расстояние от точки P до фигуры S равно D"
 */
void PrintDistancesFromPointToShapes(Point2D p, std::span<const Shape> shapes) {
    std::println("\n=== Distance from Point Test ===");

    /*
     * Используйте ranges чтобы выбрать любые 5 фигур из списка.
     * Затем найдите расстояния от заданной точки до всех выбранных фигур.
     * Выведите результат в формате "Расстояние от точки P до фигуры S равно D"
     */

    // Проверяем, что есть фигуры для теста
    if (shapes.empty()) {
        std::println("  No shapes to test");
        return;
    }

    // Берем первые 5 фигур, добавляем индексы (0..4), форматируем строки и выводим
    rs::for_each(shapes | rv::take(5) | rv::enumerate, [&p](const auto &pair) {
        const auto &[idx, shape] = pair;
        double distance = queries::DistanceToPoint(shape, p);
        std::println("  Distance from point {} to shape {} is {:.4f}", p, idx, distance);
    });
}

/**
 * @brief Функция, выполняющая комплексный анализ фигур
 *
 * @param shapes список фигур для анализа
 *
 * Анализ включает:
 * 1. Поиск всех пересекающихся пар фигур методом Bounding Box
 * 2. Поиск самой высокой фигуры
 * 3. Вывод расстояния между первой парой фигур, поддерживающих вычисление расстояния
 */
void PerformShapeAnalysis(std::span<const Shape> shapes) {
    std::println("\n=== Shape Analysis ===");

    /*
     * Используйте ranges и созданные классы чтобы:
     *     - Найти все пересечения между фигурами используя метод Bounding Box
     *     - Найти самую высокую фигуру (чья высота наибольшая)expected
     *     - Вывести расстояние между любыми двумя фигурами, которые поддерживают данную функциональность
     */

    // Проверяем, что есть фигуры для теста
    if (shapes.empty()) {
        std::println("  No shapes to test");
        return;
    }

    // 1. Находим все пересекающиеся пары фигур и выводим их
    std::println("\n--- Collisions ---");
    auto collisions = utils::FindAllCollisions(shapes);

    if (collisions.empty()) {
        std::println("  No collisions found");
    } else {
        // Формируем строки вывода для каждой коллизии с индексом
        auto collision_lines = collisions | rv::enumerate | rv::transform([&shapes](const auto &pair) -> std::string {
                                   const auto &[idx, collision_pair] = pair;
                                   const auto &[s1, s2] = collision_pair;
                                   size_t idx1 = std::distance(shapes.data(), &s1);
                                   size_t idx2 = std::distance(shapes.data(), &s2);
                                   return std::format("  Collision {}: shape {} and shape {}", idx, idx1, idx2);
                               });

        rs::for_each(collision_lines, [](const std::string &line) { std::println("{}", line); });
    }

    // 2. Находим самую высокую фигуру и выводим результат (монадический стиль)
    std::println("\n--- Highest Shape ---");
    std::println("{}", utils::FindHighestShape(shapes)         // ищем фигуру с max высотой
                           .transform([&shapes](size_t idx) {  // если есть, формируем строку с индексом и высотой
                               return std::format("  Highest shape is at index {} with height {:.4f}", idx,
                                                  queries::GetHeight(shapes[idx]));
                           })
                           .value_or(std::string("  No shapes found")));  // иначе - строку об отсутствии фигур

    // 3. Находим первую пару фигур, поддерживающих вычисление расстояния, и выводим результат
    auto all_indices = rv::iota(0u, shapes.size());  // создаем последовательность индексов всех фигур [0, size)

    // Находим первую поддерживаемую пару с помощью композиции отображений (ленивые вычисления)
    auto result =
        rv::cartesian_product(all_indices, all_indices) |  // создаем все пары (i, j) индексов фигур
        rv::filter([&](const auto &idx_pair) {             // отбираем уникальные пары (i < j) индексов фигур
            auto &&[i, j] = idx_pair;
            return (i < j);
        }) |
        rv::transform([&shapes](const auto &pair) {  // преобразуем в пару {(i, j), опц. расстояние}
            const auto &[i, j] = pair;
            return std::make_pair(pair, queries::DistanceBetweenShapes(shapes[i], shapes[j]));
        }) |
        rv::filter([](const auto &item) { return item.second.has_value(); }) |  // отсеиваем неподдерживаемые пары
        rv::take(1);                                                            // берем только первую подходящую пару

    // Выводим результат, если нашли подходящую пару
    if (auto it = rs::begin(result); it != rs::end(result)) {
        const auto &[pair, distance] = *it;
        const auto &[i, j] = pair;
        std::println("  Distance between shape {} and shape {}: {:.4f}", i, j, distance.value());
    } else {
        std::println("  No supported shape pairs found");
    }
}

/**
 * @brief Функция, выполняющая дополнительный анализ фигур
 *
 * @param shapes список фигур для анализа
 *
 * Анализ включает:
 * 1. Вывод первых 3 фигур, которые находятся выше 50.0
 * 2. Вывод фигур с наименьшей и наибольшей высотами
 */
void PerformExtraShapeAnalysis(std::span<const Shape> shapes) {
    std::println("\n=== Shape Extra Analysis ===");

    /*
     * Используйте ranges и созданные классы чтобы:
     *     - Вывести 3 любые фигуры, которые находятся выше 50.0
     *     - Вывести фигуры с наименьшей и с наибольшей высотами
     */

    // Проверяем, что есть фигуры для теста
    if (shapes.empty()) {
        std::println("  No shapes to test");
        return;
    }

    // Создаем диапазон пар (индекс, высота) для всех фигур
    auto h_pairs = shapes | rv::enumerate |  // добавляем индексы
                   rv::transform([](const auto &pair) -> std::pair<size_t, double> {
                       const auto &[idx, shape] = pair;
                       return {idx, queries::GetHeight(shape)};  // вычисляем высоту один раз
                   }) |
                   rs::to<std::vector>();  // преобразуем отображение в вектор (для min/max нужны все элементы)

    // 1. Выводим первые 3 фигуры, у которых высота > 50.0
    std::println("\n--- Shapes above 50.0 ---");
    constexpr double THRES_H = 50.0;  // пороговая высота фигур

    // Отбираем первые 3 фигуры, у которых h > THRES_H
    auto above_shapes = h_pairs | rv::filter([](const auto &pair) { return pair.second > THRES_H; }) | rv::take(3);

    // Выводим результат
    if (above_shapes.empty()) {
        std::println("  No shapes found above {:.1f}", THRES_H);
    } else {
        for (const auto &[idx, height] : above_shapes) {
            std::println("  Shape {}: height = {:.4f}", idx, height);
        }
    }

    // 2. Находим и выводим фигуры с наименьшей и наибольшей высотами
    std::println("\n--- Min and Max Height Shapes ---");

    // Находим фигуры с min и max высотами (за один проход)
    auto [min_it, max_it] =
        rs::minmax_element(h_pairs, [](const auto &a, const auto &b) { return a.second < b.second; });

    // Выводим результат (итераторы валидны, т.к. shapes и h_pairs не пусты)
    std::println("  Shape with min height: index {} (height = {:.4f})", min_it->first, min_it->second);
    std::println("  Shape with max height: index {} (height = {:.4f})", max_it->first, max_it->second);
}

/**
 * @brief Функция, собирающая все вершины из всех фигур
 *
 * @param shapes список фигур
 * @return вектор всех вершин всех фигур
 */
[[nodiscard]] std::vector<Point2D> CollectAllVertices(std::span<const Shape> shapes) {
    // Первый проход: подсчитываем общее количество вершин
    size_t total_vertices = 0;
    for (const auto &shape : shapes) {
        std::visit([&total_vertices](const auto &s) { total_vertices += s.Count(); }, shape);
    }

    // Второй проход: собираем вершины с предварительным резервированием
    std::vector<Point2D> points;
    points.reserve(total_vertices);  // одно выделение памяти
    for (const auto &shape : shapes) {
        std::visit(
            [&points](const auto &s) {
                auto verts = s.Vertices();  // использует Vertices() без параметров
                points.insert(points.end(), verts.begin(), verts.end());
            },
            shape);
    }

    return points;  // работает оптимизация RVO (без копирования)
}

int main() {
    std::vector<Shape> shapes = utils::ParseShapes("circle 0 0 1.5; line 1 2 3 4; polygon 0 0 2 5; triangle 0 0 1 0 "
                                                   "0.5 1; polygon 0 0 1 2; badshape; circle 0 0 -1");
    std::println("Parsed {} shapes", shapes.size());

    //
    // Выводим индексы и высоты всех фигур
    //
    PrintShapeHeights(shapes);

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
    std::vector<Point2D> points = CollectAllVertices(shapes);

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