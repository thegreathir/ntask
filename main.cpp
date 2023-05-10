#include <iostream>
#include <osmium/handler.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/index/map/sparse_mem_array.hpp>
#include <osmium/io/gzip_compression.hpp>
#include <osmium/io/xml_input.hpp>
#include <osmium/visitor.hpp>
#include <vector>

class DangerousBendHandler : public osmium::handler::Handler {
private:
  const std::vector<std::string> way_tags;

public:
  DangerousBendHandler(const std::vector<std::string> &way_tags)
      : way_tags(way_tags) {}

  void way(const osmium::Way &way) {
    if (way.tags().has_tag("oneway", "yes"))
      return;
    const auto *highway_value = way.tags().get_value_by_key("highway");
    if (!highway_value)
      return;
    if (std::all_of(way_tags.cbegin(), way_tags.cend(),
                    [highway_value](const std::string &way_tag) {
                      return way_tag != highway_value;
                    }))
      return;

    std::cout << way.id() << std::endl;
  }
};

int main() {
  osmium::io::Reader reader{"west.osm.gz", osmium::osm_entity_bits::node |
                                               osmium::osm_entity_bits::way};

  osmium::index::map::SparseMemArray<osmium::unsigned_object_id_type,
                                     osmium::Location>
      index;
  auto location_handler = osmium::handler::NodeLocationsForWays{index};

  const std::vector<std::string> way_tags = {"trunk", "primary", "secondary",
                                             "tertiary"};
  DangerousBendHandler dangerous_bend_handler{way_tags};

  osmium::apply(reader, location_handler, dangerous_bend_handler);
  reader.close();
  return 0;
}