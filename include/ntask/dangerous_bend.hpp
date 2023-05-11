#ifndef NTASK_DANGEROUS_BEND_HPP
#define NTASK_DANGEROUS_BEND_HPP

#include <cmath>
#include <osmium/handler.hpp>
#include <osmium/osm/node_ref.hpp>
#include <string>
#include <vector>

namespace ntask {

class DangerousBendHandler : public osmium::handler::Handler {
 public:
  struct Configuration {
    const std::vector<std::string> &highway_tags;
    const std::vector<std::pair<std::string, std::string>> &blacklisted_tags;
    double distance_threshold;
    double angle_threshold;
  };

explicit DangerousBendHandler(const Configuration &configuration);

  void way(const osmium::Way &way);

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

  const std::vector<std::string> highway_tags;
  const std::vector<std::pair<std::string, std::string>> blacklisted_tags;
  const double distance_threshold;
  const double angle_threshold;
  std::vector<osmium::NodeRef> dangerous_bends;
};

}  // namespace ntask

#endif