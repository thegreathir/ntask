#include <iostream>
#include <osmium/handler.hpp>
#include <osmium/io/gzip_compression.hpp>
#include <osmium/io/xml_input.hpp>
#include <osmium/visitor.hpp>

class DangerousBendHandler : public osmium::handler::Handler {
 public:
  void way(const osmium::Way& way) { std::cout << way.id() << std::endl; }
};

int main() {
  osmium::io::File input_file{"west.osm.gz"};
  osmium::io::Reader reader{input_file};

  DangerousBendHandler handler;
  osmium::apply(reader, handler);
  reader.close();
  return 0;
}