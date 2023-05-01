#ifndef SIMPLEWINDEROSION_WIND
#define SIMPLEWINDEROSION_WIND

float min(float& a, float b){
  return (a < b)?a:b;
}

/*
  Modelling Saltation:

  1. Particles are Lifted by Wind
  2. Since we normalize everything to the same velicoty,
    I think we can just say "relative to the prevailing wind speed"
  3.
*/

struct Wind{

  Wind(glm::vec2 _pos){ pos = vec3(_pos.x, -1.0f, _pos.y);
    time = 0.0f;
  }

  int age = 0;

  glm::vec3 pos;
  glm::vec3 speed = pspeed;
  glm::vec3 pspeed = glm::normalize(glm::vec3(cos(time),0.0,sin(time)));

  float sediment = 0.0;     //Sediment Mass

  //Parameters
  const float abrasion = 0.01;

  static int maxAge;                  // Maximum Particle Age
  static float suspension;            //Affects transport rate
  static float gravity;
  static float windfriction;
  static float time;

  bool fly();
};

int Wind::maxAge = 1024;
float Wind::suspension = 0.1f;
float Wind::gravity = 0.1;
float Wind::windfriction = 1.0;
float Wind::time = 0.0f;

bool Wind::fly(){

  if(age++ > maxAge)
    return false;

  const glm::ivec2 ipos = round(vec2(pos.x, pos.z));

  quad::node* node = World::map.get(ipos);
  if(node == NULL)
    return false;

  quad::cell* cell = node->get(ipos);
  if(cell == NULL)
    return false;

  // Correct Height, Compute Factor f. Distance to Surface

  if(pos.y < cell->height)
    pos.y = cell->height;

  float hfac = 1.0f-exp(-1.0f*(pos.y - cell->height));

  // Normal Vector

  const glm::vec3 n = World::map.normal(ipos);

  // Compute Velocity

  speed += 0.5f*(vec3(rand()%1001, rand()%1001, rand()%1001)-vec3(500))/500.0f;

  speed = mix(cross(n, cross(speed,n)), pspeed, hfac);

  if(pos.y > cell->height)
    speed.y -= 0.05;

  if(length(speed) > 0)
    speed = normalize(speed);

  pos += sqrt(2.0f)*speed;

  cell->momentumx_track += speed.x;
  cell->momentumy_track += speed.y;
  cell->momentumz_track += speed.z;
  cell->massflow_track += sediment;





  // Compute the Saltation

  ivec2 npos = vec2(pos.x, pos.z);

  node = World::map.get(npos);
  if(node == NULL)
    return false;

  cell = node->get(npos);
  if(cell == NULL)
    return false;












  // Compute Shearing Capacity

  float force = dot(speed, n);

  if(force > 0)
    force = 0;
  if(force < 0)
    force *= -1;

  float capacity = force*(1.0f-hfac);

  // ...

  float diff = capacity - sediment;
  cell->height -= suspension*diff;
  sediment += suspension*diff;

  World::cascade(npos);
  World::cascade(ipos);
  World::cascade(npos);
  World::cascade(ipos);
  World::cascade(npos);
  World::cascade(ipos);
  World::cascade(npos);
  World::cascade(ipos);

  return true;

};

#endif
