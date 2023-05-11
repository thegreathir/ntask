#ifndef NTASK_DANGEROUS_BEND_HPP
#define NTASK_DANGEROUS_BEND_HPP

#include <cmath>
#include <osmium/handler.hpp>
#include <osmium/osm/node_ref.hpp>
#include <string>
#include <vector>

namespace ntask {

/// @brief Handler to scan the ways in a given map and find dangerous bends
class DangerousBendHandler : public osmium::handler::Handler {
 public:
  /// @brief Configuration of @c DangerousBendHandler
  struct Configuration {
    /// @brief Ways without any of these values assigned to the key `highway`
    /// will be filtered out.
    std::vector<std::string> highway_tags;

    /// @brief Ways with these tags will be filtered out (e.g. `oneway=yes` will
    /// filter one-way roads).
    std::vector<std::pair<std::string, std::string>> blacklisted_tags;

    /// @brief Distance threshold in meter to search for finding two nodes
    /// around a specific node in a road to construct a tight angle
    double distance_threshold;

    /// @brief Angles less than this threshold will be marked as dangerous bend
    /// @note Unit is degree
    double angle_threshold;
  };

  explicit DangerousBendHandler(const Configuration &configuration);

  void way(const osmium::Way &way);

  /// @return Founded nodes related to a dangerous bend
  [[nodiscard]] auto get_dangerous_bends() const noexcept
      -> const std::vector<osmium::NodeRef> &;

 private:
  void add_dangerous_bend(const osmium::Way &way);

  /// @brief  Calculate the angle of a triangle
  /// (https://en.wikipedia.org/wiki/Law_of_cosines).
  /// @param node_a Fist node of triangle
  /// @param node_c Second node of triangle, The return value is the angle
  /// corresponding to this node
  /// @param node_b Third node of triangle
  /// @return The angle corresponding to @param node_c in Radian
  static auto get_angle(const osmium::Location &node_a,
                        const osmium::Location &node_c,
                        const osmium::Location &node_b) -> double;

  static constexpr double DEGREE_TO_RADIAN = (M_PI / 180.0);

  const Configuration configuration;
  const double angle_threshold;
  std::vector<osmium::NodeRef> dangerous_bends;
};

}  // namespace ntask

#endif