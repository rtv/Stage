
class point_t 
{ 
public:
  unsigned int x, y; 

  point_t( unsigned int x, unsigned int y ) : x(x), y(y) {}
};

bool astar( uint8_t* map, 
				unsigned int width, 
				unsigned int height,
				const point_t start, 
				const point_t goal, 
				std::vector<point_t>& path );

