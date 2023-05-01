#ifndef SIMPLEWINDEROSION_WIND
#define SIMPLEWINDEROSION_WIND

struct Wind{

  Wind(glm::vec2 _pos){ pos = vec3(_pos.x, -1.0f, _pos.y);
  }

  int age = 0;
  float sediment = 0.0;     //Sediment Mass

  glm::vec3 pos;
  glm::vec3 speed = pspeed;
  glm::vec3 pspeed = glm::normalize(vec3(1, 0, 0));

  static int maxAge;                  // Maximum Particle Age
  static float boundarylayer;
  static float suspension;            //Affects transport rate
  static float gravity;

  bool fly();
};

int Wind::maxAge = 1024;
float Wind::boundarylayer = 1.0;
float Wind::suspension = 0.1f;
float Wind::gravity = 0.01;

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

  speed = mix(length(speed)*cross(n, cross(normalize(speed),n)), pspeed, hfac);

  if(pos.y > cell->height)
    speed.y -= gravity;

  pos += speed;

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

  const glm::vec3 nn = World::map.normal(npos);

  float lift = 1.0f - abs(dot(normalize(speed), n));

  float force = -dot(normalize(speed), n);//*sediment;
  if(force < 0)
    force = 0;

  float capacity = (force + 0.01*lift)*(1.0f-hfac);

  // Mass Transfer to Equilibrium

  float diff = capacity - sediment;
  ncell->height -= suspension*diff;
  sediment += suspension*diff;

  World::cascade(ipos);
  World::cascade(npos);
  return true;

};

#endif
