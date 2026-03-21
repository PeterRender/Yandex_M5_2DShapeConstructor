#pragma once
#include "geometry.hpp"
#include <utility>
#include <vector>

namespace geometry::utils {

std::vector<Shape> ParseShapes(std::string_view input);

// Функция поиска всех пар пересекающихся фигур
std::vector<std::pair<Shape, Shape>> FindAllCollisions(std::span<const Shape> shapes);

// Функция поиска индекса фигуры c max высотой (Y-координатой)
std::optional<size_t> FindHighestShape(std::span<const Shape> shapes);

}  // namespace geometry::utils