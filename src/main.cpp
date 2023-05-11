#include <fstream>
#include <iostream>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/index/map/sparse_mem_array.hpp>
#include <osmium/io/gzip_compression.hpp>
#include <osmium/io/xml_input.hpp>
#include <osmium/visitor.hpp>

#include "dangerous_bend.hpp"
#include "nlohmann/json.hpp"

auto main() -> int {
  try {
    constexpr const char* CONFIG_FILE_NAME = "config.json";
    std::ifstream config_file(CONFIG_FILE_NAME);
    const auto config = nlohmann::json::parse(config_file);

    osmium::io::Reader reader{
        static_cast<std::string>(config["input_file"]),
        osmium::osm_entity_bits::node | osmium::osm_entity_bits::way};

    using IndexType =
        osmium::index::map::SparseMemArray<osmium::unsigned_object_id_type,
                                           osmium::Location>;
    IndexType index;
    auto location_handler =
        osmium::handler::NodeLocationsForWays<IndexType>{index};

    std::vector<std::string> highway_tags;
    std::transform(
        config["highway_tags"].cbegin(), config["highway_tags"].cend(),
        std::back_inserter(highway_tags),
        [](const auto& highway_tag) -> std::string { return highway_tag; });

    std::vector<std::pair<std::string, std::string>> blacklisted_tags;
    std::transform(
        config["blacklisted_tags"].cbegin(), config["blacklisted_tags"].cend(),
        std::back_inserter(blacklisted_tags), [](const auto& blacklisted_tag) {
          return std::make_pair(blacklisted_tag["key"],
                                blacklisted_tag["value"]);
        });

    ntask::DangerousBendHandler dangerous_bend_handler{
        ntask::DangerousBendHandler::Configuration{
            .highway_tags = highway_tags,
            .blacklisted_tags = blacklisted_tags,
            .distance_threshold = config["distance_threshold"],
            .angle_threshold = config["angle_threshold"]}};

    osmium::apply(reader, location_handler, dangerous_bend_handler);
    reader.close();

    auto result = nlohmann::json::array();
    for (const auto& node : dangerous_bend_handler.get_dangerous_bends()) {
      result.push_back(nlohmann::json{
          {"location", {{"lat", node.lat()}, {"lon", node.lon()}}},
          {"link", "https://www.openstreetmap.org/node/" +
                       std::to_string(node.ref())}});
    }

    std::ofstream result_file{config["output_file"]};
    result_file << result.dump(2) << std::endl;

    return 0;
  } catch (const std::exception& err) {
    std::cerr << "Exception occurred: " << err.what() << std::endl;
    return 1;
  }
}