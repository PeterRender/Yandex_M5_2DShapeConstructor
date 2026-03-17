#include "visualization.hpp"
#include "geometry.hpp"

#include <matplot/matplot.h>
#include <print>

namespace geometry::visualization {

template <class... Ts>
struct Multilambda : Ts... {
    using Ts::operator()...;
};
auto DrawConfig()
{
    using namespace geometry;
    using namespace matplot;

    // Disable gnuplot warnings
    auto f = figure(false);
    f->backend()->run_command("unset warnings");
    f->ioff();
    f->size(900, 900);

    hold(on);     // Multiple plots mode
    axis(equal);  // Squre view
    grid(on);     // Enable grid by default
    return f;
}

void Draw(std::span<geometry::Shape> shapes) {
    using namespace geometry;
    using namespace matplot;
    const auto& fh = DrawConfig();
    for (const auto &[index, shape] : std::ranges::views::enumerate(shapes)) {
        /**
         * @brief Для каждой фигуры примените `std::visit` с помощью мульти-лямбдs (Multilambda),
         *    которая обрабатывает каждый возможный тип фигуры отдельно.
         *    Внутри каждой лямбды:
         *    - Вызовите метод `.Lines()` у фигуры — он возвращает структуру с двумя векторами:
         *      `.x` и `.y`, содержащими координаты точек для отрисовки.
         *    - Передайте эти координаты в функцию `plot(lines.x, lines.y)`.
         *    - Настройте внешний вид линии: установите толщину `.line_width(2)` и задайте цвет `.color()`:
         *        • Line      → "yellow"
         *        • Triangle  → "blue"
         *        • Rectangle → "green"
         *        • RegularPolygon → "magenta"
         *        • Circle    → "red"
         *        • Polygon   → "cyan"
         * 
         */

        //ваш код тут
        // Add shape number
        const auto center = shape.visit([](auto &&s) { return s.Center(); });
        auto t = text(center.x, center.y, std::to_string(index));
        t->font_size(14);
        t->color("black");
    }

    // Display plot
    fh->show();
}

void Draw(std::span<const geometry::triangulation::DelaunayTriangle> triangles) {
    using namespace geometry;
    using namespace matplot;
    
    const auto& fh = DrawConfig();

    for (const auto &[index, d_triangle] : std::ranges::views::enumerate(triangles)) {
        const geometry::Triangle tri{d_triangle.a, d_triangle.b, d_triangle.c};
        const auto lines = tri.Lines();
        plot(lines.x, lines.y)->line_width(2).color("cyan");

        // Add triangle number
        const auto center = tri.Center();
        auto t = text(center.x, center.y, std::to_string(index));
        t->font_size(14);
        t->color("black");
    }

    // Display plot
    fh->show();
}

}  // namespace geometry::visualization
