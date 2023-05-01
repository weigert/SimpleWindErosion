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

  Wind(glm::vec2 _pos){ pos = vec3(_pos.x, 0, _pos.y); }

  int age = 0;

  glm::vec3 pos;
  glm::vec3 speed = pspeed;
  glm::vec3 pspeed = glm::normalize(glm::vec3(1.0,0.0,1.0));

  float sediment = 0.5;     //Sediment Mass

  //Parameters
  const float abrasion = 0.01;

  static int maxAge;                  // Maximum Particle Age
  static float suspension;            //Affects transport rate
  static float gravity;
  static float windfriction;

  bool fly();

};

int Wind::maxAge = 1024;
float Wind::suspension = 0.001f;
float Wind::gravity = 0.1;
float Wind::windfriction = 1.0;

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

  if(pos.y <= cell->height){
    if(dot(n, speed) < 0){
      speed = mix(speed, cross(n, cross(speed,n)), 1.0);
    }
    pos.y = cell->height;
  }

  float hfac = exp(-0.01f*(pos.y - cell->height));

//  vec3 ground_speed = vec3(cell->momentumx, cell->momentumy, cell->momentumz);

//  vec3 fspeed = mix(pspeed, ground_speed, hfac);
  speed = mix(pspeed, speed, hfac);


  //Movement Mechanics

  if(pos.y > cell->height)      //Flying Movement
    speed.y -= 0.25*sediment;   //Gravity
//  else                    //Contact Movement

//  speed = mix(speed, pspeed, 0.2);
  if(length(speed) > 0){

  speed = normalize(speed);
  pos += sqrt(2.0f)*speed;

  cell->momentumx_track += sediment*speed.x;
  cell->momentumy_track += sediment*speed.y;
  cell->momentumz_track += sediment*speed.z;
  cell->massflow_track += sediment;

  }



/*


*/

  // Find the Equilibrium amount of Sediment

  // Next Cell

  ivec2 npos = vec2(pos.x, pos.z);

  node = World::map.get(npos);
  if(node == NULL)
    return false;

  cell = node->get(npos);
  if(cell == NULL)
    return false;


  // Erosion Calculation

  if(pos.y <= cell->height){

    double force = length(speed)*(cell->height-pos.y)*(1.0f-sediment);
    cell->height -= suspension*force;
    sediment += (suspension*force);

    World::cascade(ipos);

    pos.y = cell->height;

  }

  else {

    cell->height += suspension*sediment;
    sediment -= suspension*sediment;


  }

  World::cascade(npos);


      /*
        Basically what needs to happen is:
        If momentum against a point is high,
        this increases the shear-stress.

        Higher shear-stress means faster rate of sediment
        uptake or rather high maximum sediment value.

        So we have an equilibrium amount of Sediment,
        determined by the shear-stress, determined by the
        collisions of particles with the

        Then when there is no collision the shear-stress is low
        and thus the particles start losing capacity
        and so on

        we are accelerated by the momentum
        the total shear-stress is given by the total momentum though
        and is like an activation function
        after some critical amount of momentum???

        We also have drag working on the particles
        They are also being accelerated by the wind
      */

  return true;

};

#endif
