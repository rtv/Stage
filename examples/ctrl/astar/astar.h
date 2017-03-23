
namespace ast {

class point_t {
public:
  uint32_t x, y;

  point_t(uint32_t x, uint32_t y) : x(x), y(y) {}
};

bool astar(uint8_t *map, uint32_t width, uint32_t height, const point_t &start, const point_t &goal,
           std::vector<point_t> &path);
}
