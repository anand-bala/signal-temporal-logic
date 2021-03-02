#include <catch2/catch.hpp>

#include <memory>
#include <random>
#include <string>
#include <string_view>

class RandomStrGenerator : public Catch::Generators::IGenerator<std::string> {
  static constexpr std::string_view alphanum =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";

  std::minstd_rand m_rand;
  std::uniform_int_distribution<size_t> m_dist;
  size_t str_len;
  std::string current_string;

 public:
  RandomStrGenerator(size_t len) :
      m_rand(std::random_device{}()),
      m_dist{0, alphanum.size()},
      str_len{len},
      current_string{} {
    static_cast<void>(next());
  }

  [[nodiscard]] std::string const& get() const override {
    return current_string;
  }
  bool next() override {
    current_string = {};
    for (size_t i = 0; i < str_len; i++) {
      char c = alphanum[m_dist(m_rand)];
      current_string += c;
    }
    return true;
  }
};

Catch::Generators::GeneratorWrapper<std::string> random_str(size_t len) {
  return Catch::Generators::GeneratorWrapper<std::string>(
      std::make_unique<RandomStrGenerator>(len));
}
