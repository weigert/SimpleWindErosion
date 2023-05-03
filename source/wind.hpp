#ifndef SIMPLEWINDEROSION_WIND
#define SIMPLEWINDEROSION_WIND

struct Wind{

  Wind(glm::vec2 _pos){ pos = vec3(_pos.x, 0.0f, _pos.y); }

  int age = 0;
  float sediment = 0.0;     //Sediment Mass

  glm::vec3 pos;
  glm::vec3 speed = vec3(0);
  glm::vec3 pspeed = glm::normalize(vec3(1, 0, 0));

  static int maxAge;                  // Maximum Particle Age
  static float boundarylayer;
  static float suspension;            //Affects transport rate
  static float gravity;

  bool fly();
};

int Wind::maxAge = 64;
float Wind::boundarylayer = 1.0f;
float Wind::suspension = 0.05f;
float Wind::gravity = 0.1;

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

  const glm::vec3 n = World::map.normal(ipos);

  if(pos.y < cell->height)
    pos.y = cell->height;

  // Compute Movement

  const float hfac = exp(-(pos.y - cell->height)/boundarylayer);

  // Apply Base Prevailign Wind-Speed w. Shadowing

  float shadow = dot(pspeed, n);
  if(shadow < 0)
    shadow = 0;
  shadow = 1.0f-shadow;

  speed += 0.05f*((0.1f+0.9f*shadow)*pspeed - speed);

  // Apply Gravity

  if(pos.y > cell->height)
    speed.y -= gravity*sediment;

  // Compute Collision Factor

  float collision = -dot(normalize(speed), n);
  if(collision < 0) collision = 0;

  // Compute Redirect Velocity

  vec3 rspeed = cross(n, cross((1.0f-collision)*speed, n));

  // Speed is accelerated by terrain features

  speed += 0.9f*( shadow*mix(pspeed, rspeed, shadow*hfac) - speed);

  // Speed is damped by drag

  speed *= (1.0f - 0.2*sediment);

  // Turbulence

  speed += 0.1f*hfac*collision*(vec3(rand()%1001, rand()%1001, rand()%1001)-500.0f)/500.0f;

  // Move

  pos += speed;

  cell->momentumx_track += speed.x;
  cell->momentumy_track += speed.y;
  cell->momentumz_track += speed.z;
  cell->massflow_track += sediment;

  // Compute Mass Transport

  float force = -dot(normalize(speed), n)*length(speed);
  float capacity = force*hfac;
  //if(capacity < 0)
  //  capacity = 0;

  // Mass Transfer to Equilibrium

  float diff = capacity - sediment;
  cell->height -= suspension*diff;
  sediment += suspension*diff;

  World::cascade(ipos);
  return true;

};

#endif
