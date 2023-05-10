#include <cmath>
#include <iostream>
#include <osmium/geom/haversine.hpp>
#include <osmium/handler.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/index/map/sparse_mem_array.hpp>
#include <osmium/io/gzip_compression.hpp>
#include <osmium/io/xml_input.hpp>
#include <osmium/visitor.hpp>
#include <vector>

class DangerousBendHandler : public osmium::handler::Handler {
 public:
  DangerousBendHandler(const std::vector<std::string> &way_tags,
                       double distance_threshold, double angle_threshold)
      : way_tags(way_tags),
        distance_threshold(distance_threshold),
        angle_threshold(angle_threshold * DEGREE_TO_RADIAN) {}

  void way(const osmium::Way &way) {
    if (way.tags().has_tag("oneway", "yes")) return;
    if (way.tags().has_tag("junction", "roundabout")) return;
    const auto *highway_value = way.tags().get_value_by_key("highway");
    if (!highway_value) return;
    if (std::all_of(way_tags.cbegin(), way_tags.cend(),
                    [highway_value](const std::string &way_tag) {
                      return way_tag != highway_value;
                    }))
      return;
    add_dangerous_bend(way);
  }

  const std::vector<osmium::NodeRef> &get_dangerous_bends() const {
    return dangerous_bends;
  }

 private:
  void add_dangerous_bend(const osmium::Way &way) {
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
        if (osmium::geom::haversine::distance(
                nodes[right_node_index].location(),
                nodes[node_index].location()) > distance_threshold)
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

  /// @brief  Calculate the angle of a triangle
  /// (https://en.wikipedia.org/wiki/Law_of_cosines).
  /// @param node_a Fist node of triangle
  /// @param node_c Second node of triangle, The return value is the angle
  /// corresponding to this node
  /// @param node_b Third node of triangle
  /// @return The angle corresponding to @param node_c on Radian
  double get_angle(const osmium::Location &node_a,
                   const osmium::Location &node_c,
                   const osmium::Location &node_b) {
    double dist_a = osmium::geom::haversine::distance(node_c, node_b);
    double dist_b = osmium::geom::haversine::distance(node_a, node_c);
    double dist_c = osmium::geom::haversine::distance(node_a, node_b);

    return std::acos(
        ((dist_a * dist_a) + (dist_b * dist_b) - (dist_c * dist_c)) /
        (2 * dist_a * dist_b));
  }

  static constexpr double DEGREE_TO_RADIAN = (M_PI / 180.0);

  const std::vector<std::string> way_tags;
  const double distance_threshold;
  const double angle_threshold;
  std::vector<osmium::NodeRef> dangerous_bends;
};

int main() {
  osmium::io::Reader reader{"west.osm.gz", osmium::osm_entity_bits::node |
                                               osmium::osm_entity_bits::way};

  osmium::index::map::SparseMemArray<osmium::unsigned_object_id_type,
                                     osmium::Location>
      index;
  auto location_handler = osmium::handler::NodeLocationsForWays{index};

  DangerousBendHandler dangerous_bend_handler{
      {"trunk", "primary", "secondary", "tertiary"}, 50, 135};

  osmium::apply(reader, location_handler, dangerous_bend_handler);
  reader.close();

  for (const auto &node : dangerous_bend_handler.get_dangerous_bends()) {
    std::cout << "https://www.openstreetmap.org/node/" << node.ref()
              << std::endl;
  }

  return 0;
}