#include <iostream>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/index/map/sparse_mem_array.hpp>
#include <osmium/io/gzip_compression.hpp>
#include <osmium/io/xml_input.hpp>
#include <osmium/visitor.hpp>

#include "dangerous_bend.hpp"

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
      {"trunk", "primary", "secondary", "tertiary"}, 50, 135};

  osmium::apply(reader, location_handler, dangerous_bend_handler);
  reader.close();

  for (const auto &node : dangerous_bend_handler.get_dangerous_bends()) {
    std::cout << "https://www.openstreetmap.org/node/" << node.ref()
              << std::endl;
  }

  return 0;
}