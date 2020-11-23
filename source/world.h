#include "vegetation.h"
#include "wind.h"

#define SIZE 256

class World{
public:
  //Constructor
  void generate();                      //Initialize Heightmap
  void erode(int cycles);               //Erode with N Particles

  int SEED = 0;
  glm::ivec2 dim = glm::vec2(SIZE, SIZE);  //Size of the heightmap array

  double scale = 80.0;                  //"Physical" Height scaling of the map
  double heightmap[SIZE*SIZE] = {0.0};    //Flat Array

  double sealevel = 20.0;               //

  double windpath[SIZE*SIZE] = {0.0};    //Wind Strength
  double sediment[SIZE*SIZE] = {0.0};    //Sedimentation Pile

  double tmp[SIZE*SIZE] = {0.0};          //Temporary Array

  //Trees
  std::vector<Plant> trees;
  double plantdensity[SIZE*SIZE] = {0.0}; //Density for Plants

  //Erosion Process
  bool active = false;
};

/*
===================================================
          WORLD GENERATING FUNCTIONS
===================================================
*/

void World::generate(){
  std::cout<<"Generating New World"<<std::endl;
  if(SEED == 0) SEED = time(NULL);

  std::cout<<"Seed: "<<SEED<<std::endl;
  //Seed the Random Generator
  srand(SEED);

  std::cout<<"... generating height ..."<<std::endl;

  //Initialize Heightmap
  noise::module::Perlin perlin;

  //Mountainy:
  perlin.SetOctaveCount(8);
  perlin.SetFrequency(1.0);
  perlin.SetPersistence(0.5);


  double min, max = 0.0;
  for(int i = 0; i < dim.x*dim.y; i++){
    tmp[i] = perlin.GetValue((i/dim.y)*(1.0/dim.x), (i%dim.y)*(1.0/dim.y), SEED);
    if(tmp[i] > max) max = tmp[i];
    if(tmp[i] < min) min = tmp[i];
  }
  //Normalize
  for(int i = 0; i < dim.x*dim.y; i++){
    float x = i/dim.y;
    float y = i%dim.y;

    float p = 1.2-2*(abs(x-dim.x/2)/dim.x+abs(y-dim.y/2)/dim.y);
    sediment[i] += (tmp[i] - min)/(max - min);

  }

  min = max = 0.0;
  for(int i = 0; i < dim.x*dim.y; i++){
    heightmap[i] = perlin.GetValue((i/dim.y)*(1.0/dim.x), (i%dim.y)*(1.0/dim.y), SEED);
    if(heightmap[i] > max) max = heightmap[i];
    if(heightmap[i] < min) min = heightmap[i];
  }
  //Normalize
  for(int i = 0; i < dim.x*dim.y; i++){
    float x = i/dim.y;
    float y = i%dim.y;

    tmp[i] = 1.2-2*(abs(x-dim.x/2)/dim.x+abs(y-dim.y/2)/dim.y);//0.9*(heightmap[i] - min)/(max - min);
    heightmap[i] = tmp[i];
    if(tmp[i] > sediment[i]){
      sediment[i] = 0.0;
    }
    else{
      sediment[i] -= heightmap[i];
    }
  }



}

/*
===================================================
          HYDRAULIC EROSION FUNCTIONS
===================================================
*/

void World::erode(int cycles){

  //Track the Movement of all Particles
  bool track[dim.x*dim.y] = {false};

  //Do a series of iterations!
  for(int i = 0; i < cycles; i++){

    //Spawn New Particle
    glm::vec2 newpos = glm::vec2(rand()%(int)dim.x, rand()%(int)dim.y);
    Wind wind(newpos);
    wind.fly(heightmap, windpath, sediment, track, plantdensity, dim, scale, sealevel);

  }

  //Update Path
  double lrate = 0.01;
  for(int i = 0; i < dim.x*dim.y; i++)
    windpath[i] = (1.0-lrate)*windpath[i] + lrate*((track[i])?1.0:0.0);

}

/*
===================================================
                RENDERING STUFF
===================================================
*/

World world;

int WIDTH = 1000;
int HEIGHT = 1000;

bool paused = true;

float zoom = 0.2;
float zoomInc = 0.005;

//Rotation and View
float rotation = 0.0f;
glm::vec3 cameraPos = glm::vec3(50, 50, 50);
glm::vec3 lookPos = glm::vec3(0, 0, 0);
glm::mat4 camera = glm::lookAt(cameraPos, lookPos, glm::vec3(0,1,0));
glm::mat4 projection = glm::ortho(-(float)WIDTH*zoom, (float)WIDTH*zoom, -(float)HEIGHT*zoom, (float)HEIGHT*zoom, -800.0f, 500.0f);

glm::vec3 viewPos = glm::vec3(world.dim.x/2.0, world.scale/2.0, world.dim.y/2.0);

//Shader Stuff
float steepness = 0.8;
//Desert Colors
glm::vec3 steepColor = glm::vec3(0.78, 0.6, 0.168);
glm::vec3 flatColor = glm::vec3(0.84, 0.65, 0.36);
glm::vec3 waterColor = glm::vec3(0.96, 0.48, 0.32);

//Lighting and Shading
glm::vec3 skyCol = glm::vec4(0.64, 0.75, 0.9, 1.0f);
glm::vec3 lightPos = glm::vec3(-100.0f, 100.0f, 150.0f);
glm::vec3 lightCol = glm::vec3(1.0f, 1.0f, 0.9f);
float lightStrength = 1.4;
glm::mat4 depthModelMatrix = glm::mat4(1.0);
glm::mat4 depthProjection = glm::ortho<float>(-300, 300, -300, 300, 0, 800);
glm::mat4 depthCamera = glm::lookAt(lightPos, glm::vec3(0), glm::vec3(0,1,0));
bool viewmap = true;

glm::mat4 biasMatrix = glm::mat4(
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 0.5, 0.0,
    0.5, 0.5, 0.5, 1.0
);

std::function<void(Model* m)> constructor = [](Model* m){
  //Clear the Containers
  m->indices.clear();
  m->positions.clear();
  m->normals.clear();
  m->colors.clear();

  //Loop over all positions and add the triangles!
  for(int i = 0; i < world.dim.x-1; i++){
    for(int j = 0; j < world.dim.y-1; j++){

      //Get Index
      int ind = i*world.dim.y+j;

      //Add to Position Vector
      glm::vec3 a = glm::vec3(i, world.scale*world.heightmap[ind], j);
      glm::vec3 b = glm::vec3(i+1, world.scale*world.heightmap[ind+world.dim.y], j);
      glm::vec3 c = glm::vec3(i, world.scale*world.heightmap[ind+1], j+1);
      glm::vec3 d = glm::vec3(i+1, world.scale*world.heightmap[ind+world.dim.y+1], j+1);

      //Check if the Surface is Water
      bool water1 = (world.sediment[ind] > 0.0 &&
                     world.sediment[ind+world.dim.y] > 0.0 &&
                     world.sediment[ind+1] > 0.0);

      bool water2 = (world.sediment[ind+world.dim.y] > 0.0 &&
                     world.sediment[ind+1] > 0.0 &&
                     world.sediment[ind+world.dim.y+1] > 0.0);

      //Add the Pool Height
      a += glm::vec3(0.0, world.scale*world.sediment[ind], 0.0);
      b += glm::vec3(0.0, world.scale*world.sediment[ind+world.dim.y], 0.0);
      c += glm::vec3(0.0, world.scale*world.sediment[ind+1], 0.0);
      d += glm::vec3(0.0, world.scale*world.sediment[ind+world.dim.y+1], 0.0);

      std::function<double(double d)> ease = [](double d){
        //const double K = 0.01;
        //const double B = 0.01*world.scale;
        //return B*K*d/(1+K*d);
        //0.1 = shallow river
        //

        //if(d > 0.1)
        //  return -0.02*(d-0.1)*world.scale;
        return 0.0;
      };

      //Add the Stream Height
      a += glm::vec3(0.0, ease(world.windpath[ind]), 0.0);
      b += glm::vec3(0.0, ease(world.windpath[ind+world.dim.y]), 0.0);
      c += glm::vec3(0.0, ease(world.windpath[ind+1]), 0.0);
      d += glm::vec3(0.0, ease(world.windpath[ind+world.dim.y+1]), 0.0);

      //UPPER TRIANGLE

      //Get the Color of the Ground (Water vs. Flat)
      glm::vec3 color;
      double p = ease::langmuir(world.sediment[ind], 10.0);

      //See if we are water or not!
      color = glm::mix(flatColor, waterColor, p);

      glm::vec3 othercolor;

      //Add Indices
      m->indices.push_back(m->positions.size()/3+0);
      m->indices.push_back(m->positions.size()/3+1);
      m->indices.push_back(m->positions.size()/3+2);

      m->positions.push_back(a.x);
      m->positions.push_back(a.y);
      m->positions.push_back(a.z);
      m->positions.push_back(b.x);
      m->positions.push_back(b.y);
      m->positions.push_back(b.z);
      m->positions.push_back(c.x);
      m->positions.push_back(c.y);
      m->positions.push_back(c.z);

      glm::vec3 n1 = glm::normalize(glm::cross(a-b, c-b));

      for(int i = 0; i < 3; i++){
        m->normals.push_back(n1.x);
        m->normals.push_back(n1.y);
        m->normals.push_back(n1.z);

        //Add the Color!
        if(n1.y < steepness && !water1){
          othercolor = glm::mix(steepColor, color, p);
          m->colors.push_back(othercolor.x);
          m->colors.push_back(othercolor.y);
          m->colors.push_back(othercolor.z);
          m->colors.push_back(1.0);
        }
        else{
          m->colors.push_back(color.x);
          m->colors.push_back(color.y);
          m->colors.push_back(color.z);
          m->colors.push_back(1.0);
        }

      }

      //Lower Triangle
      if(water2) color = waterColor;
      else color = flatColor;//glm::mix(flatColor, waterColor, p);

      m->indices.push_back(m->positions.size()/3+0);
      m->indices.push_back(m->positions.size()/3+1);
      m->indices.push_back(m->positions.size()/3+2);

      m->positions.push_back(d.x);
      m->positions.push_back(d.y);
      m->positions.push_back(d.z);
      m->positions.push_back(c.x);
      m->positions.push_back(c.y);
      m->positions.push_back(c.z);
      m->positions.push_back(b.x);
      m->positions.push_back(b.y);
      m->positions.push_back(b.z);

      glm::vec3 n2 = glm::normalize(glm::cross(d-c, b-c));

      for(int i = 0; i < 3; i++){
        m->normals.push_back(n2.x);
        m->normals.push_back(n2.y);
        m->normals.push_back(n2.z);

        if(n2.y < steepness && !water2){
          m->colors.push_back(steepColor.x);
          m->colors.push_back(steepColor.y);
          m->colors.push_back(steepColor.z);
          m->colors.push_back(1.0);
        }
        else{
          m->colors.push_back(color.x);
          m->colors.push_back(color.y);
          m->colors.push_back(color.z);
          m->colors.push_back(1.0);
        }

      }
    }
  }
};

std::function<void()> eventHandler = [&](){

  if(!Tiny::event.scroll.empty()){

    if(Tiny::event.scroll.back().wheel.y > 0.99 && zoom <= 0.3){
      zoom /= 0.975;
      projection = glm::ortho(-(float)WIDTH*zoom, (float)WIDTH*zoom, -(float)HEIGHT*zoom, (float)HEIGHT*zoom, -800.0f, 500.0f);
    }
    else if(Tiny::event.scroll.back().wheel.y < -0.99 && zoom > 0.005){
      zoom *= 0.975;
      projection = glm::ortho(-(float)WIDTH*zoom, (float)WIDTH*zoom, -(float)HEIGHT*zoom, (float)HEIGHT*zoom, -800.0f, 500.0f);
    }
    else if(Tiny::event.scroll.back().wheel.x < -0.8){
      rotation += 1.5f;
      camera = glm::rotate(camera, glm::radians(1.5f), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    else if(Tiny::event.scroll.back().wheel.x > 0.8){
      rotation -= 1.5f;
      camera = glm::rotate(camera, glm::radians(-1.5f), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    //Adjust Stuff
    if(rotation < 0.0) rotation = 360.0 + rotation;
    else if(rotation > 360.0) rotation = rotation - 360.0;
    camera = glm::rotate(glm::lookAt(cameraPos, lookPos, glm::vec3(0,1,0)), glm::radians(rotation), glm::vec3(0,1,0));
    Tiny::event.scroll.pop_back();
  }

  if(!Tiny::event.keys.empty()){

    if(Tiny::event.keys.back().key.keysym.sym == SDLK_p){
      paused = !paused;
    }

    if(Tiny::event.keys.back().key.keysym.sym == SDLK_ESCAPE){
      viewmap = !viewmap;
    }

    if(Tiny::event.keys.back().key.keysym.sym == SDLK_SPACE){
      viewPos += glm::vec3(0.0, 1.0, 0.0);
    }

    if(Tiny::event.keys.back().key.keysym.sym == SDLK_c){
      viewPos -= glm::vec3(0.0, 1.0, 0.0);
    }

    if(Tiny::event.keys.back().key.keysym.sym == SDLK_w){
      viewPos -= glm::vec3(1.0, 0.0, 0.0);
    }

    if(Tiny::event.keys.back().key.keysym.sym == SDLK_a){
      viewPos += glm::vec3(0.0, 0.0, 1.0);
    }

    if(Tiny::event.keys.back().key.keysym.sym == SDLK_s){
      viewPos += glm::vec3(1.0, 0.0, 0.0);
    }

    if(Tiny::event.keys.back().key.keysym.sym == SDLK_d){
      viewPos -= glm::vec3(0.0, 0.0, 1.0);
    }

    if(Tiny::event.keys.back().key.keysym.sym == SDLK_UP){
      cameraPos += glm::vec3(0, 5, 0);
      camera = glm::rotate(glm::lookAt(cameraPos, lookPos, glm::vec3(0,1,0)), glm::radians(rotation), glm::vec3(0,1,0));
    }

    if(Tiny::event.keys.back().key.keysym.sym == SDLK_DOWN){
      cameraPos -= glm::vec3(0, 5, 0);
      camera = glm::rotate(glm::lookAt(cameraPos, lookPos, glm::vec3(0,1,0)), glm::radians(rotation), glm::vec3(0,1,0));
    }

    //Remove the guy
    Tiny::event.keys.pop_back();
  }
};

std::function<glm::vec4(double, double)> hydromap = [](double t1, double t2){
  glm::vec4 color = glm::mix(glm::vec4(0.0, 0.0, 0.0, 1.0), glm::vec4(1.0, 0.87, 0.73, 1.0), ease::langmuir(t2, 10.0));

  color = glm::mix(color, glm::vec4(0.96, 0.48, 0.32, 1.0), ease::langmuir(t1, 10.0));
  return color;
};
