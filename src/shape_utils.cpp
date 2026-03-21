#include "shape_utils.hpp"
#include <functional>

#include "intersections.hpp"
#include "queries.hpp"

namespace geometry::utils {

namespace rv = std::ranges::views;  // псевдоним пространства имен отображений
namespace rs = std::ranges;         // псевдоним пространства имен диапазонов

// Разбивает строку на слова (по пробелам), игнорируя лишние пробелы
std::vector<std::string_view> SplitIntoWords(std::string_view s) {
    std::vector<std::string_view> words;
    size_t start = 0;
    size_t end = 0;

    while (start < s.size()) {
        // Пропускаем пробелы
        while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start])))
            ++start;
        if (start >= s.size())
            break;
        end = start;
        while (end < s.size() && !std::isspace(static_cast<unsigned char>(s[end])))
            ++end;
        words.push_back(s.substr(start, end - start));
        start = end;
    }
    return words;
}

// Безопасный парсинг строки в double (без исключений)
std::optional<double> ParseDouble(std::string_view s) {
    double value = 0.0;
    auto result = std::from_chars(s.data(), s.data() + s.size(), value);
    if (result.ec == std::errc{} && result.ptr == s.data() + s.size()) {
        return value;
    }
    return std::nullopt;
}

// Парсит строку в вектор double
std::optional<std::vector<double>> ParseDoubles(std::string_view s) {
    if (s.empty())
        return std::nullopt;
    auto tokens = SplitIntoWords(s);
    if (tokens.empty())
        return std::nullopt;

    std::vector<double> result;
    result.reserve(tokens.size());

    for (auto token : tokens) {
        auto num = ParseDouble(token);
        if (!num.has_value()) {
            return std::nullopt;
        }
        result.push_back(*num);
    }
    return result;
}

// Проверяет размер вектора и возвращает его, если совпадает
std::optional<std::vector<double>> RequireSize(const std::vector<double> &v, size_t expected) {
    return (v.size() == expected) ? std::make_optional(v) : std::nullopt;
}

// Проверяет, что значение > 0
std::optional<double> RequirePositive(double x) { return (x > 0) ? std::make_optional(x) : std::nullopt; }

// Проверяет, что double представляет целое число >= min_value
std::optional<int> RequireIntegerAtLeast(double d, int min_value) {
    int i = static_cast<int>(d);
    if (static_cast<double>(i) == d && i >= min_value) {
        return i;
    }
    return std::nullopt;
}

// === Конструкторы фигур (с использованием монадического стиля) ===

// Создает круг из вектора параметров
std::optional<Shape> MakeCircle(const std::vector<double> &v) {
    // 1. Проверяем, что есть 3 параметра: x, y, radius
    // 2. Проверяем, что радиус > 0
    // 3. Создаем окружность
    return RequireSize(v, 3).and_then([&v](auto) { return RequirePositive(v[2]); }).transform([&v](auto) -> Shape {
        return Circle{{v[0], v[1]}, v[2]};
    });
}

// Создает линию из вектора параметров
std::optional<Shape> MakeLine(const std::vector<double> &v) {
    // 1. Проверяем, что есть 4 параметра: x1, y1, x2, y2
    // 2. Создаем линию (может быть вырожденной)
    return RequireSize(v, 4).transform([&v](auto) -> Shape { return Line{{v[0], v[1]}, {v[2], v[3]}}; });
}

// Создает треугольник из вектора параметров
std::optional<Shape> MakeTriangle(const std::vector<double> &v) {
    // 1. Проверяем, что есть 6 параметров: x1, y1, x2, y2, x3, y3
    // 2. Создаем треугольник (может быть вырожденным)
    return RequireSize(v, 6).transform(
        [&v](auto) -> Shape { return Triangle{{v[0], v[1]}, {v[2], v[3]}, {v[4], v[5]}}; });
}

// Создает прямоугольник из вектора параметров
std::optional<Shape> MakeRectangle(const std::vector<double> &v) {
    // 1. Проверяем, что есть 4 параметра: x, y (левый нижний угол), ширина, высота
    // 2. Проверяем, что ширина > 0 и высота > 0
    // 3. Создаем прямоугольник
    return RequireSize(v, 4)
        .and_then([&v](auto) { return RequirePositive(v[2]); })
        .and_then([&v](auto) { return RequirePositive(v[3]); })
        .transform([&v](auto) -> Shape { return Rectangle{{v[0], v[1]}, v[2], v[3]}; });
}

// Создает правильный многоугольник из вектора параметров
std::optional<Shape> MakePolygon(const std::vector<double> &v) {
    // 1. Проверяем, что есть 4 параметра: x, y (центр), радиус, число сторон
    // 2. Проверяем, что радиус > 0 и число сторон - это целое число >= 3
    // 3. Создаем правильный многоугольник
    return RequireSize(v, 4)
        .and_then([&v](auto) { return RequirePositive(v[2]); })
        .and_then([&v](auto) { return RequireIntegerAtLeast(v[3], 3); })
        .transform([&v](auto) -> Shape { return RegularPolygon{{v[0], v[1]}, v[2], static_cast<int>(v[3])}; });
}

// Парсит одну фигуру из строки токена
// * Вход: строка вида "<тип> <параметры>" (например: "circle 0 0 2.5")
// * Поддерживаемые типы: circle, line, triangle, rectangle, polygon
// * Выход: если парсинг успешен, то фигура std::optional<Shape>, иначе std::nullopt
std::optional<Shape> ParseSingleShape(std::string_view token) {
    // 1. Разбиваем токен на слова: первое слово - тип фигуры, остальные - параметры
    auto parts = SplitIntoWords(token);
    if (parts.empty())
        return std::nullopt;

    std::string_view type = parts[0];  // тип фигуры ("circle", "line", ...)
    std::string param_str;             // строка параметров (для парсинга чисел)

    // 2. Объединяем все слова параметров в одну строку
    for (auto i : std::views::iota(1u, parts.size())) {
        if (!param_str.empty())
            param_str += ' ';
        param_str += std::string(parts[i]);
    }

    // Псевдоним типа для фабричной функции фигуры
    // (принимает вектор параметров, возвращает optional<Shape>)
    using ShapeMaker = std::function<std::optional<Shape>(const std::vector<double> &)>;

    // 3*. Создаем словарь фабричных функций (инициализируется один раз при первом вызове функции)
    static const std::unordered_map<std::string_view, ShapeMaker> maker_map = {{"circle", MakeCircle},
                                                                               {"line", MakeLine},
                                                                               {"triangle", MakeTriangle},
                                                                               {"rectangle", MakeRectangle},
                                                                               {"polygon", MakePolygon}};

    // Фабрика функций: отображает строковый тип фигуры в соответствующую фабричную функцию
    auto get_maker = [](std::string_view t) -> std::optional<ShapeMaker> {
        return maker_map.contains(t) ? std::make_optional(maker_map.at(t)) : std::nullopt;
    };

    // 4. Монадическая цепочка обработки (без исключений):
    // - Получаем фабричную функцию: get_maker(type) -> std::optional<ShapeMaker> (если тип неизвестен -> nullopt)
    // - Передаем фабрику в лямбду: and_then([&](auto maker) {...}) (если фабрики нет -> nullopt)
    // - Парсим строку параметров: ParseDoubles(...) -> std::optional<vector> (если ошибка -> nullopt)
    // - Применяем фабрику к вектору параметров: and_then(maker) -> std::optional<Shape> (если ошибка -> nullopt)
    // Результат: если все этапы успешны, то фигура std::optional<Shape>, иначе nullopt
    return get_maker(type).and_then([&](auto maker) { return ParseDoubles(param_str).and_then(maker); });
}

std::vector<Shape> ParseShapes(std::string_view input) {
    std::vector<Shape> result;

    // Разделяем по ';'
    size_t start = 0;
    size_t end = 0;
    while (start < input.size()) {
        end = input.find(';', start);
        if (end == std::string_view::npos)
            end = input.size();

        std::string_view token = input.substr(start, end - start);
        // Убираем пробелы по краям
        while (!token.empty() && std::isspace(static_cast<unsigned char>(token.front())))
            token.remove_prefix(1);
        while (!token.empty() && std::isspace(static_cast<unsigned char>(token.back())))
            token.remove_suffix(1);

        if (!token.empty()) {
            auto shape_opt = ParseSingleShape(token);
            if (shape_opt.has_value()) {
                result.push_back(*shape_opt);
            }
        }

        start = end + 1;
    }

    return result;
}

// Функция поиска всех пар пересекающихся фигур
std::vector<std::pair<Shape, Shape>> FindAllCollisions(std::span<const Shape> shapes) {
    // Проверяем, что в списке есть фигуры
    if (shapes.empty()) {
        return {};  // пустой вектор
    }

    // Создаем последовательность индексов всех фигур [0, size)
    auto all_indices = rv::iota(0u, shapes.size());

    // Формируем вектор пар пересекающихся фигур с помощью композиции отображений
    return rv::cartesian_product(all_indices, all_indices) |  // создаем все пары (i, j) индексов фигур
           rv::filter([&](const auto &idx_pair) {             // отбираем уникальные пары (i < j) индексов фигур
               auto &&[i, j] = idx_pair;                      //  с пересекающимися ограничивающими боксами
               return (i < j) && queries::BoundingBoxesOverlap(shapes[i], shapes[j]);
           }) |
           rv::transform([&](const auto &idx_pair) {  // преобразуем индексы в фигуры
               auto &&[i, j] = idx_pair;
               return std::pair{shapes[i], shapes[j]};
           }) |
           rs::to<std::vector>();  // преобразуем диапазон пар фигур в вектор пар фигур (одно выделение памяти)
}

// Функция поиска индекса фигуры c max высотой (Y-координатой)
std::optional<size_t> FindHighestShape(std::span<const Shape> shapes) {
    // Проверяем, что в списке есть фигуры
    if (shapes.empty()) {
        return std::nullopt;
    }

    // Создаем последовательность пар (индекс, фигура)
    auto enum_shapes = rv::enumerate(shapes);

    // Находим пару (индекс, фигура) с max высотой фигуры
    auto it_max_h = rs::max_element(enum_shapes, std::less<>{},  // ищем max
                                    [](const auto &pair) {       // проекция: высота фигуры
                                        auto &&[idx, shape] = pair;
                                        return queries::GetHeight(shape);
                                    });

    // Возвращаем индекс - первый элемент пары
    // (it_max_h гарантированно не end(), т.к. shapes не пуст)
    return std::get<0>(*it_max_h);
}
}  // namespace geometry::utils