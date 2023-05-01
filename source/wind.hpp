#ifndef SIMPLEWINDEROSION_WIND
#define SIMPLEWINDEROSION_WIND

struct Wind{

  Wind(glm::vec2 _pos){ pos = vec3(_pos.x, -1.0f, _pos.y);
    time = 0.0f;
  }

  int age = 0;

  glm::vec3 pos;
  glm::vec3 speed = pspeed;
  glm::vec3 pspeed = glm::normalize(glm::vec3(cos(time),0.0,sin(time)));

  float sediment = 0.0;     //Sediment Mass

  static int maxAge;                  // Maximum Particle Age
  static float suspension;            //Affects transport rate
  static float gravity;
  static float time;

  static float boundarylayer;

  bool fly();
};

int Wind::maxAge = 1024;
float Wind::boundarylayer = 1.0;
float Wind::suspension = 0.1f;


float Wind::gravity = 0.1;
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

  if(pos.y < cell->height)
    pos.y = cell->height;

  // Compute Movement

  const glm::vec3 n = World::map.normal(ipos);
  const float hfac = 1.0f-exp(-(pos.y - cell->height)/boundarylayer);

  speed += 0.1f*(vec3(rand()%1001, rand()%1001, rand()%1001)-500.0f)/500.0f;

  speed = mix(cross(n, cross(speed,n)), pspeed, hfac);

  //if(pos.y > cell->height)
  //  speed.y -= 0.05;

  //if(length(speed) > 0)
  //  speed = normalize(speed);

  pos += sqrt(2.0f)*speed;

  cell->momentumx_track += speed.x;
  cell->momentumy_track += speed.y;
  cell->momentumz_track += speed.z;
  cell->massflow_track += sediment;

  // Compute the Saltation

  const glm::ivec2 npos = round(vec2(pos.x, pos.z));

  quad::node* nnode = World::map.get(npos);
  if(nnode == NULL)
    return false;

  quad::cell* ncell = nnode->get(npos);
  if(ncell == NULL)
    return false;

  //

  // Compute Mass Transport

  // Lift-Capacity

  float lift = 1.0f - abs(dot(normalize(speed), n));


  // Compute Shearing Capacity

  float force = dot(normalize(speed), n)*sediment;

  if(force > 0)
    force = 0;
  if(force < 0)
    force *= -1;

  float capacity = force + 0.05*lift;

  // ...

  float diff = capacity*(1.0f-hfac) - sediment;
  ncell->height -= suspension*diff;
  sediment += suspension*diff;

  World::cascade(ipos);
  World::cascade(npos);
  World::cascade(ipos);
  World::cascade(npos);
  World::cascade(ipos);
  World::cascade(npos);
  World::cascade(ipos);
  World::cascade(npos);

  return true;

};

#endif
