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
  const double abrasion = 0.01;

  static int maxAge;                  // Maximum Particle Age
  static float suspension;            //Affects transport rate
  static float gravity;
  static float windfriction;

  bool fly();

};

int Wind::maxAge = 100;
float Wind::suspension = 0.001f;
float Wind::gravity = 0.25;
float Wind::windfriction = 0.8;

bool Wind::fly(){

  const glm::ivec2 ipos = pos;

  quad::node* node = World::map.get(ipos);
  if(node == NULL)
    return false;

  quad::cell* cell = node->get(ipos);
  if(cell == NULL)
    return false;

  if(age > maxAge){
    cell->height += suspension*sediment;
    World::cascade(ipos);
    return false;
  }

  const glm::vec3 n = World::map.normal(ipos);

  // Fix Height

  if(height < cell->height) // Raise Particle Height
    height = cell->height;


//  if(height > cell->height)    //Flying Movement
//    speed.y -= gravity;   //Gravity
/*
  else                    //Contact Movement
//    speed = mix(speed, cross(cross(speed,n),n), windfriction);
    speed = mix(speed, 20*2*dot(n,normalize(speed))*n - normalize(speed), windfriction);
*/

  // Manipulate Sped


  // Surface-Contact
  if(height <= cell->height){
    speed += 20.0f*(2*dot(n,normalize(speed))*n - normalize(speed));
  }


  // Momentum
  vec2 fspeed = vec2(cell->momentumx, cell->momentumy);
  if(length(fspeed) > 0 && length(speed) > 0){
    float momentumTransfer = 5.0f;
    float effTransfer = momentumTransfer*dot(normalize(fspeed), normalize(vec2(speed.x, speed.z)));
    speed.x += quad::lodsize*effTransfer/(sediment + cell->massflow)*fspeed.x;
    speed.z += quad::lodsize*effTransfer/(sediment + cell->massflow)*fspeed.y;
  }

  // Prevailing Speed + Gravity
  speed += vec3(8.0f, -2.0f, 8.0f);

  if(length(speed) > 0)
    speed = (quad::lodsize*cbrt(3.0f))*normalize(speed);

  // Adjust Position

  pos += glm::vec2(speed.x, speed.z);
  height += speed.y;



  //Mass Transport

  if(World::map.oob(pos)){
    return false;
  }




  double diff = 0;
  if(height <= cell->height){
    diff = 0.1f;//(dot(normalize(speed), normalize(n)));
    cell->height -= suspension*diff;
    sediment += suspension*diff;
  }
  else{
    diff = sediment;
    cell->height += suspension*diff;
    sediment -= suspension*diff;
  }

  cell->momentumx_track += sediment*speed.x;
  cell->momentumy_track += sediment*speed.z;
  cell->massflow_track += sediment;

  World::cascade(pos);
  World::cascade(ipos);

  return true;

};

#endif
