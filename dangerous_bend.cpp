#include "dangerous_bend.hpp"

#include <osmium/geom/haversine.hpp>

using ntask::DangerousBendHandler;

DangerousBendHandler::DangerousBendHandler(const Configuration &configuration)
    : highway_tags(configuration.highway_tags),
      blacklisted_tags(configuration.blacklisted_tags),
      distance_threshold(configuration.distance_threshold),
      angle_threshold(configuration.angle_threshold * DEGREE_TO_RADIAN) {}

void DangerousBendHandler::way(const osmium::Way &way) {
  if (std::any_of(
          blacklisted_tags.begin(), blacklisted_tags.end(),
          [&way](const std::pair<std::string, std::string> &blacklisted_tag) {
            return way.tags().has_tag(blacklisted_tag.first.c_str(),
                                      blacklisted_tag.second.c_str());
          }))
    return;

  const auto *highway_value = way.tags().get_value_by_key("highway");
  if (!highway_value) return;
  if (std::all_of(highway_tags.cbegin(), highway_tags.cend(),
                  [highway_value](const std::string &way_tag) {
                    return way_tag != highway_value;
                  }))
    return;

  add_dangerous_bend(way);
}

const std::vector<osmium::NodeRef> &DangerousBendHandler::get_dangerous_bends()
    const noexcept {
  return dangerous_bends;
}

void DangerousBendHandler::add_dangerous_bend(const osmium::Way &way) {
  const auto &nodes = way.nodes();

  for (int node_index = 0; node_index < static_cast<int>(nodes.size());
       ++node_index) {
    std::vector<int> left_node_indices;
    std::vector<int> right_node_indices;

    for (int left_node_index = node_index - 1; left_node_index >= 0;
         --left_node_index) {
      if (osmium::geom::haversine::distance(nodes[left_node_index].location(),
                                            nodes[node_index].location()) >
          distance_threshold)
        break;

      left_node_indices.push_back(left_node_index);
    }

    for (int right_node_index = node_index + 1;
         right_node_index < static_cast<int>(nodes.size());
         ++right_node_index) {
      if (osmium::geom::haversine::distance(nodes[right_node_index].location(),
                                            nodes[node_index].location()) >
          distance_threshold)
        break;

      right_node_indices.push_back(right_node_index);
    }

    double min_angle = std::numeric_limits<double>::infinity();
    for (auto left_index : left_node_indices)
      for (auto right_index : right_node_indices) {
        min_angle =
            std::min(min_angle, get_angle(nodes[left_index].location(),
                                          nodes[node_index].location(),
                                          nodes[right_index].location()));
      }

    if (min_angle < angle_threshold)
      dangerous_bends.push_back(nodes[node_index]);
  }
}

double DangerousBendHandler::get_angle(const osmium::Location &node_a,
                                       const osmium::Location &node_c,
                                       const osmium::Location &node_b) {
  double dist_a = osmium::geom::haversine::distance(node_c, node_b);
  double dist_b = osmium::geom::haversine::distance(node_a, node_c);
  double dist_c = osmium::geom::haversine::distance(node_a, node_b);

  return std::acos(((dist_a * dist_a) + (dist_b * dist_b) - (dist_c * dist_c)) /
                   (2 * dist_a * dist_b));
}
