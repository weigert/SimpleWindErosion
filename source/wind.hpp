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

/*
  vec3 fspeed = vec3(cell->momentumx, cell->momentumy, cell->momentumz);
  if(length(fspeed) > 0 && length(speed) > 0){
    speed = mix(speed, fspeed, 0.5);
  }
  */
  //  speed += quad::lodsize*1.0f*(1.0f-dot(normalize(fspeed), normalize(speed)))/(sediment + cell->massflow)*fspeed;

  // Speed is accelerated by pspeed
  //float damping = 0.1+0.9*dot(pspeed, n);
  //if(damping > 0) damping = 0;

  float damping = dot(pspeed, n);
  if(damping < 0) damping = 0;
  damping = 1.0f-damping;

  speed += 0.05f*((0.1f+0.9f*damping)*pspeed - speed);

  if(pos.y > cell->height)
    speed.y -= gravity*sediment;

  // Speed is accelerated by terrain features

  speed += 0.9f*( (0.0f+1.0f*damping)*mix(pspeed, cross(n, cross(speed, n)), hfac) - speed);

  // Speed is damped by drag

  //speed *= (1.0f - 0.2*sediment);

  // Speed i

//  vec3 gspeed = 0.95f*;
//  speed += (hfac)*(gspeed - speed);
//  vec3 targetspeed =  + hfac*pspeed;
//  speed = targetspeed;//0.25f*(targetspeed-speed);
//  speed = mix(pspeed, gspeed, hfac);
  //speed += 0.2f*n;
  speed += 0.25f*(vec3(rand()%1001, rand()%1001, rand()%1001)-500.0f)/500.0f*dot(speed, n);

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

  float lift = (1.0f - dot(normalize(speed), n))*length(speed);

  float force = -dot(normalize(speed), n)*length(speed);//*sediment;
  if(force < 0)
    force = 0;

  float capacity = (force + 0.01*lift)*(hfac);

  // Mass Transfer to Equilibrium

  float diff = capacity - sediment;
  cell->height -= suspension*diff;
  sediment += suspension*diff;

  World::cascade(ipos);
  World::cascade(npos);
  return true;

};

#endif
