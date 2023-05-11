#include <fstream>
#include <iostream>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/index/map/sparse_mem_array.hpp>
#include <osmium/io/gzip_compression.hpp>
#include <osmium/io/xml_input.hpp>
#include <osmium/visitor.hpp>

#include "dangerous_bend.hpp"
#include "json.hpp"

int main() {
  osmium::io::Reader reader{"west.osm.gz", osmium::osm_entity_bits::node |
                                               osmium::osm_entity_bits::way};

  using IndexType =
      osmium::index::map::SparseMemArray<osmium::unsigned_object_id_type,
                                         osmium::Location>;
  IndexType index;
  auto location_handler =
      osmium::handler::NodeLocationsForWays<IndexType>{index};

  ntask::DangerousBendHandler dangerous_bend_handler{
      ntask::DangerousBendHandler::Configuration{
          .highway_tags = {"trunk", "primary", "secondary", "tertiary"},
          .blacklisted_tags = {{"oneway", "yes"}, {"junction", "roundabout"}},
          .distance_threshold = 50,
          .angle_threshold = 135}};

  osmium::apply(reader, location_handler, dangerous_bend_handler);
  reader.close();

  auto result = nlohmann::json::array();
  for (const auto& node : dangerous_bend_handler.get_dangerous_bends()) {
    result.push_back(nlohmann::json{
        {"location", {{"lat", node.lat()}, {"lon", node.lon()}}},
        {"link",
         "https://www.openstreetmap.org/node/" + std::to_string(node.ref())}});
  }

  constexpr const char* RESULT_FILE_NAME = "result.json";
  std::ofstream result_file{RESULT_FILE_NAME};
  result_file << result.dump(2) << std::endl;

  return 0;
}