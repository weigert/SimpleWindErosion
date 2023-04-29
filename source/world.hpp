#ifndef SIMPLEWINDEROSION_WORLD
#define SIMPLEWINDEROSION_WORLD

#include "include/FastNoiseLite.h"
#include "include/math.h"

#include "cellpool.h"

class World{
public:

  static unsigned int SEED;
  static quad::map map;

  //Constructor

  static float lrate;
  static float maxdiff;
  static float settling;

  static void erode(int cycles);               //Erode with N Particles
  static void cascade(vec2 pos);              // Perform Sediment Cascade

};

unsigned int World::SEED = 0;

quad::map World::map;

float World::lrate = 0.1f;
float World::maxdiff = 0.005;
float World::settling = 0.05;

#include "wind.hpp"

void World::erode(int cycles){

  for(auto& node: map.nodes)
  for(auto [cell, pos]: node.s){
    cell.massflow_track = 0;
    cell.momentumx_track = 0;
    cell.momentumy_track = 0;
  }

  //Do a series of iterations!
  for(auto& node: map.nodes)
  for(int i = 0; i < cycles; i++){

    //Spawn New Particle on Boundary
    glm::vec2 newpos;
    int shift = rand()%(int)(quad::tileres.x+quad::tileres.y);
    if(shift < quad::tileres.x) newpos = glm::vec2(shift, 0);
    else              newpos = glm::vec2(0, shift-quad::tileres.x);

    Wind wind(newpos);
    while(wind.fly());

  }

  //Update Fields
  for(auto& node: map.nodes)
  for(auto [cell, pos]: node.s){
    cell.massflow = (1.0f-lrate)*cell.massflow + lrate*cell.massflow_track;
    cell.momentumx = (1.0f-lrate)*cell.momentumx + lrate*cell.momentumx_track;
    cell.momentumy = (1.0f-lrate)*cell.momentumy + lrate*cell.momentumy_track;
  }
}

void World::cascade(vec2 pos){

  // Get Non-Out-of-Bounds Neighbors

  static const ivec2 n[] = {
    ivec2(-1, -1),
    ivec2(-1,  0),
    ivec2(-1,  1),
    ivec2( 0, -1),
    ivec2( 0,  1),
    ivec2( 1, -1),
    ivec2( 1,  0),
    ivec2( 1,  1)
  };

  struct Point {
    ivec2 pos;
    float h;
    float d;
  };

  static Point sn[8];
  int num = 0;

  ivec2 ipos = pos;

  for(auto& nn: n){

    ivec2 npos = ipos + quad::lodsize*nn;

    if(World::map.oob(npos))
      continue;

    sn[num++] = { npos, World::map.get(npos)->get(npos)->height, length(vec2(nn)) };

  }

  //Iterate over all sorted Neighbors

  sort(std::begin(sn), std::begin(sn) + num, [&](const Point& a, const Point& b){
    return a.h < b.h;
  });

  for (int i = 0; i < num; ++i) {

    auto& npos = sn[i].pos;

    //Full Height-Different Between Positions!
    float diff = World::map.get(ipos)->get(ipos)->height - sn[i].h;
    if(diff == 0)   //No Height Difference
      continue;

      //The Amount of Excess Difference!
    float excess = 0.0f;
    if(sn[i].h > 0.1){
      excess = abs(diff) - sn[i].d*maxdiff * quad::lodsize;
    } else {
      excess = abs(diff);
    }

    if(excess <= 0)  //No Excess
      continue;

    //Actual Amount Transferred
    float transfer = settling * excess / 2.0f;

    //Cap by Maximum Transferrable Amount
    if(diff > 0){
      World::map.get(ipos)->get(ipos)->height -= transfer;
      World::map.get(npos)->get(npos)->height += transfer;
    }
    else{
      World::map.get(ipos)->get(ipos)->height += transfer;
      World::map.get(npos)->get(npos)->height -= transfer;
    }

  }

}


#endif
