#ifndef SIMPLEWINDEROSION_CELLPOOL
#define SIMPLEWINDEROSION_CELLPOOL

/*
================================================================================
                    Interleaved Cell Data Memory Pool
================================================================================
  Individual cell properties are stored in an interleaved data format.
  The mappool acts as a fixed-size memory pool for these cells.
  This acts as the base for creating sliceable, indexable, iterable map regions.
*/

namespace mappool {

// Raw Interleaved Data Buffer
template<typename T> struct buf;
template<typename T> struct buf_iterator {
  T* cur = NULL;
  buf_iterator() noexcept : cur(NULL){};
  buf_iterator(T* t) noexcept : cur(t){};

  const T operator*() noexcept {
      return *this->cur;
  };

  const buf_iterator<T>& operator++() noexcept {
    if(cur != NULL) ++cur;
    return *this;
  };

  const bool operator!=(const buf_iterator<T>& other) const noexcept {
    return this->cur != other.cur;
  };
};
template<typename T> struct buf {
  T* start = NULL;
  size_t size = 0;

  const buf_iterator<T> begin() const noexcept { return buf_iterator<T>(start); }
  const buf_iterator<T> end()   const noexcept { return buf_iterator<T>(start+size); }
};

// Raw Interleaved Data Buffer Slice
template<typename T> struct slice;
template<typename T> struct sliceval {
  T& start;             // Variable Reference
  ivec2 pos = ivec2(0); // Slice Position
};
template<typename T> struct slice_iterator {
  ivec2 pos = ivec2(0);
  buf_iterator<T> cur = NULL;
  const ivec2 res;

  slice_iterator() noexcept : cur(NULL){};
  slice_iterator(const buf_iterator<T>& t, const ivec2 r) noexcept : cur(t), res(r){};

  const sliceval<T> operator*() noexcept {
      return {*(cur.cur), pos};
  };

  const slice_iterator<T>& operator++() noexcept {
    ++cur;
    if((pos.y + 1)%res.x == 0)
      pos.x = (pos.x + 1);
    pos.y = (pos.y + 1)%res.x;
    return *this;
  };

  const bool operator!=(const slice_iterator<T> &other) const noexcept {
    return this->cur != other.cur;
  };
};
template<typename T> struct slice {

  mappool::buf<T> root;
  ivec2 res = ivec2(0);

  const inline size_t size(){
    return res.x * res.y;
  }

  const inline bool oob(const ivec2 p){
    if(p.x >= res.x)  return true;
    if(p.y >= res.y)  return true;
    if(p.x  < 0)      return true;
    if(p.y  < 0)      return true;
    return false;
  }

  inline T* get(const ivec2 p){
    if(root.start == NULL) return NULL;
    if(oob(p)) return NULL;
    return root.start + math::flatten(p, res);
  }

  /*

  // Periodic Boundary Condition

  const inline bool oob(const ivec2 p){
    return false;
  }

  inline T* get(ivec2 p){
    if(root.start == NULL) return NULL;
    while(p.x  < 0)  p.x += res.x;
    while(p.y  < 0)  p.y += res.y;
    while(p.x >= res.x)  p.x -= res.x;
    while(p.y >= res.y)  p.y -= res.y;
    return root.start + math::flatten(p, res);
  }

  */

  slice_iterator<T> begin() const noexcept { return slice_iterator<T>(root.begin(), res); }
  slice_iterator<T> end()   const noexcept { return slice_iterator<T>(root.end(), res); }

};

// Raw Interleaved Data Pool
template<typename T>
struct pool {

  buf<T> root;
  deque<buf<T>> free;

  pool(){}
  pool(size_t _size){
    reserve(_size);
  }

  ~pool(){
    if(root.start != NULL){
      delete[] root.start;
      root.start = NULL;
    }
  }

  void reserve(size_t _size){
    root.size = _size;
    root.start = new T[root.size];
    free.emplace_front(root.start, root.size);
  }

  buf<T> get(size_t _size){

    if(free.empty())
      return {NULL, 0};

    if(_size > root.size)
      return {NULL, 0};

    if(free.front().size < _size)
      return {NULL, 0};

    buf<T> sec = {free.front().start, _size};
    free.front().start += _size;
    free.front().size -= _size;

    return sec;

  }

};

};  // namespace mappool

/*
================================================================================
                Cell Buffer Spatial Indexing / Slicing
================================================================================
  A mapslice acts as an indexable, bound-checking structure for this.
  This is the base-structure for retrieving data.

  For now, our map implementation uses a fixed arrangement of tiles.
  Soon, we will be able to switch to a quadtree with arbitrary shape.

  Finally, we can make the nodes have multiple scales and reimplement
  their retrieval functions.
*/

namespace quad {

const int mapscale = 80;

const int tilesize = 512;
const int tilearea = tilesize*tilesize;
const ivec2 tileres = ivec2(tilesize);

const int mapsize = 1;
const int maparea = mapsize*mapsize;

const int size = mapsize*tilesize;
const int area = maparea*tilearea;
const ivec2 res = ivec2(size);

const int lodsize = 1;
const int lodarea = lodsize*lodsize;

template<typename T>
vec3 _normal(T& t, ivec2 p){

  vec3 n = vec3(0, 0, 0);
  const vec3 s = vec3(1.0, quad::mapscale, 1.0);

  if(!t.oob(p + quad::lodsize*ivec2( 1, 1)))
    n += cross( s*vec3( 0.0, t.height(p+quad::lodsize*ivec2( 0, 1)) - t.height(p), 1.0), s*vec3( 1.0, t.height(p+quad::lodsize*ivec2( 1, 0)) - t.height(p), 0.0));

  if(!t.oob(p + quad::lodsize*ivec2(-1,-1)))
    n += cross( s*vec3( 0.0, t.height(p-quad::lodsize*ivec2( 0, 1)) - t.height(p),-1.0), s*vec3(-1.0, t.height(p-quad::lodsize*ivec2( 1, 0)) - t.height(p), 0.0));

  //Two Alternative Planes (+X -> -Y) (-X -> +Y)
  if(!t.oob(p + quad::lodsize*ivec2( 1,-1)))
    n += cross( s*vec3( 1.0, t.height(p+quad::lodsize*ivec2( 1, 0)) - t.height(p), 0.0), s*vec3( 0.0, t.height(p-quad::lodsize*ivec2( 0, 1)) - t.height(p),-1.0));

  if(!t.oob(p + quad::lodsize*ivec2(-1, 1)))
    n += cross( s*vec3(-1.0, t.height(p-quad::lodsize*ivec2( 1, 0)) - t.height(p), 0.0), s*vec3( 0.0, t.height(p+quad::lodsize*ivec2( 0, 1)) - t.height(p), 1.0));

  if(length(n) > 0)
    n = normalize(n);
  return n;

}

// Raw Interleaved Cell Data
struct cell {

  float height;

  float massflow;
  float momentumx;
  float momentumy;
  float momentumz;

  float massflow_track;
  float momentumx_track;
  float momentumy_track;
  float momentumz_track;

};

struct node {

  ivec2 pos = ivec2(0);   // Absolute World Position
  uint* vertex = NULL;    // Vertexpool Rendering Pointer
  mappool::slice<cell> s; // Raw Interleaved Data Slices

  inline cell* get(const ivec2 p){
    return s.get((p - pos)/lodsize);
  }

  const inline bool oob(const ivec2 p){
    return s.oob((p - pos)/lodsize);
  }

  const inline float height(ivec2 p){
    cell* c = get(p);
    if(c == NULL) return 0.0f;
    return c->height;
  }

  const inline vec3 normal(ivec2 p){
    return _normal(*this, p);
  }

};

void indexnode(Vertexpool<Vertex>& vertexpool, quad::node& t){

  // Iterate over the Node's Slice
  for(const auto& [cell, pos]: t.s){
    if(pos.x == tilesize/lodsize - 1) continue;
    if(pos.y == tilesize/lodsize - 1) continue;
    vertexpool.indices.push_back(math::flatten(pos + ivec2(0, 0), tileres/lodsize));
    vertexpool.indices.push_back(math::flatten(pos + ivec2(0, 1), tileres/lodsize));
    vertexpool.indices.push_back(math::flatten(pos + ivec2(1, 0), tileres/lodsize));
    vertexpool.indices.push_back(math::flatten(pos + ivec2(1, 0), tileres/lodsize));
    vertexpool.indices.push_back(math::flatten(pos + ivec2(0, 1), tileres/lodsize));
    vertexpool.indices.push_back(math::flatten(pos + ivec2(1, 1), tileres/lodsize));
  }

  // Side-Drapes

  /*
  for(size_t i = 0; i < tilesize/lodsize - 1; i++){
    vertexpool.indices.push_back(i);
    vertexpool.indices.push_back(tilesize + i);
    vertexpool.indices.push_back(tilesize + i + 1);
    vertexpool.indices.push_back(i+1);
    vertexpool.indices.push_back(tilesize + i + 1);
    vertexpool.indices.push_back(tilesize + i);
  }
  */

  // Update the Vertexpool Properties
  vertexpool.resize(t.vertex, vertexpool.indices.size());
  vertexpool.index();
  vertexpool.update();

}

void updatenode(Vertexpool<Vertex>& vertexpool, quad::node& t){

  for(auto [cell, pos]: t.s){

    glm::vec2 p = t.pos + lodsize*pos;
    glm::vec2 pT = t.pos + lodsize*(pos + ivec2( 1, 0));
    glm::vec2 pB = t.pos + lodsize*(pos + ivec2( 0, 1));

    glm::vec3 P = glm::vec3(p.x, quad::mapscale*t.height(p), p.y);
    glm::vec3 T = glm::vec3(pT.x, quad::mapscale*t.height(pT), pT.y);
    glm::vec3 B = glm::vec3(pB.x, quad::mapscale*t.height(pB), pB.y);

    vertexpool.fill(t.vertex, math::flatten(pos, tileres/lodsize),
      P,
      t.normal(p),
      T - P,
      B - P
    );

  }

  /*
  for(size_t i = 0; i < tilesize/lodsize; i++){
    vertexpool.fill(t.vertex, tilesize + i,
      glm::vec3(0, -10, i),
      glm::vec3(1, 0, 0),
      glm::vec3(0, 1, 0),
      glm::vec3(0, 0, 1)
    );
  }*/

}

struct map {

  node nodes[maparea];

  void init(Vertexpool<Vertex>& vertexpool, mappool::pool<cell>& cellpool, int SEED){

    // Generate the Node Array

    for(int i = 0; i < mapsize; i++)
    for(int j = 0; j < mapsize; j++){

      int ind = i*mapsize + j;

      nodes[ind] = {
        tileres*ivec2(i, j),
        vertexpool.section(tilearea/lodarea, 0, glm::vec3(0), vertexpool.indices.size()),
        { cellpool.get(tilearea/lodarea), tileres/lodsize }
      };

      indexnode(vertexpool, nodes[ind]);

    }

    // Fill the Node Array

    std::cout<<"Generating New World"<<std::endl;
    std::cout<<"Seed: "<<SEED<<std::endl;

    std::cout<<"... generating height ..."<<std::endl;

    static FastNoiseLite noise; //Noise System
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noise.SetFractalType(FastNoiseLite::FractalType_FBm);


    for(auto& node: nodes){

      for(auto [cell, pos]: node.s){
        cell.height = 0.0f;
        cell.massflow = 0.0f;
      }


      // Add Gaussian

      for(auto [cell, pos]: node.s){
        vec2 p = vec2(node.pos+lodsize*pos)/vec2(quad::tileres);
        vec2 c = vec2(node.pos+quad::tileres/ivec2(4, 2))/vec2(quad::tileres);
        float d = length(p-c);
        cell.height = exp(-d*d*quad::tilesize*0.2);
      }


/*

      // Add Layers of Noise

      float frequency = 1.0f;
      float scale = 0.6f;

      for(size_t o = 0; o < 8; o++){

        noise.SetFrequency(frequency);

        for(auto [cell, pos]: node.s){

          vec2 p = vec2(node.pos+lodsize*pos)/vec2(quad::tileres);
          cell.height += scale*noise.GetNoise(p.x, p.y, (float)(SEED%10000));

        }

        frequency *= 2;
        scale *= 0.6;

      }

*/


    }

    float min = 0.0f;
    float max = 0.0f;

    for(auto& node: nodes)
    for(auto [cell, pos]: node.s){
      min = (min < cell.height)?min:cell.height;
      max = (max > cell.height)?max:cell.height;
    }

    for(auto& node: nodes)
    for(auto [cell, pos]: node.s){
      cell.height = 0.5*((cell.height - min)/(max - min));
    }

  }

  const inline bool oob(ivec2 p){
    if(p.x  < 0)  return true;
    if(p.y  < 0)  return true;
    if(p.x >= size)  return true;
    if(p.y >= size)  return true;
    return false;
  }

  inline node* get(ivec2 p){
    if(oob(p)) return NULL;
    p /= tileres;
    int ind = p.x*mapsize + p.y;
    return &nodes[ind];
  }

  /*

  // Periodic Boundary Condition

  const inline bool oob(ivec2 p){
    return false;
  }

  inline node* get(ivec2 p){
    while(p.x  < 0)  p.x += size;
    while(p.y  < 0)  p.y += size;
    while(p.x >= size)  p.x -= size;
    while(p.y >= size)  p.y -= size;
    p /= tileres;
    int ind = p.x*mapsize + p.y;
    return &nodes[ind];
  }

  */

  inline cell* getCell(ivec2 p){
    if(oob(p)) return NULL;
    return get(p)->get(p);
  }

  const inline float height(ivec2 p){
    node* n = get(p);
    if(n == NULL) return 0.0f;
    return n->height(p);
  }

  const inline vec3 normal(ivec2 p){
    return _normal(*this, p);
  }

};

}; // namespace quad

#endif
