#pragma once
#include "geometry.hpp"
#include <utility>
#include <vector>

namespace geometry::utils {

// Функция парсинга строковых описаний фигур (токенов) в массив объектов
std::vector<Shape> ParseShapes(std::string_view input);

// Функция поиска всех пар пересекающихся фигур
std::vector<std::pair<Shape, Shape>> FindAllCollisions(std::span<const Shape> shapes);

// Функция поиска индекса фигуры c max высотой (Y-координатой)
std::optional<size_t> FindHighestShape(std::span<const Shape> shapes);

// Фабричные функции для создания фигур
std::optional<Shape> MakeCircle(const std::vector<double> &v);
std::optional<Shape> MakeLine(const std::vector<double> &v);
std::optional<Shape> MakeTriangle(const std::vector<double> &v);
std::optional<Shape> MakeRectangle(const std::vector<double> &v);
std::optional<Shape> MakePolygon(const std::vector<double> &v);

}  // namespace geometry::utils