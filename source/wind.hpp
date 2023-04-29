#ifndef SIMPLEWINDEROSION_WIND
#define SIMPLEWINDEROSION_WIND

double min(double& a, double b){
  return (a < b)?a:b;
}

struct Wind{

  Wind(glm::vec2 _pos){ pos = _pos; }

  int age = 0;
  glm::vec2 pos;

  float height = 0.0;
  glm::vec3 speed = pspeed;
  glm::vec3 pspeed = 2.0f*glm::normalize(glm::vec3(1.0,0.0,1.0));

  double sediment = 0.0; //Sediment Mass

  //Parameters
  const float dt = 0.25;
  const double suspension = 0.002;  //Affects transport rate
  const double abrasion = 0.01;


  bool fly();

};

bool Wind::fly(){

  const glm::ivec2 ipos = pos;

  quad::node* node = World::map.get(ipos);
  if(node == NULL)
    return false;

  quad::cell* cell = node->get(ipos);
  if(cell == NULL)
    return false;


  if(height < cell->height) // Raise Particle Height
    height = cell->height;

  //Movement Mechanics

  if(height > cell->height)   //Flying
    speed.y -= dt*0.01;       //Gravity
  else{ //Contact Movement
    const glm::vec3 n = World::map.normal(ipos);
    speed += dt*glm::cross(glm::cross(speed,n),n);
  }

  speed += 0.1f*dt*(pspeed - speed);
  pos += dt*glm::vec2(speed.x, speed.z);
  height += dt*speed.y;

  cell->discharge_track += 0.2;

  //Mass Transport

  if(World::map.oob(pos)){
    return false;
  }

  //Surface Contact
  if(height <= cell->height){

    double force = glm::length(speed)*(cell->height - height);

    cell->height -= dt*suspension*force;
    sediment += dt*suspension*force;

  }

  //Flying Particle
  else{
    sediment -= dt*suspension*sediment;
    cell->height   += dt*suspension*sediment;
  }

  World::cascade(pos);

  return true;

};

#endif
