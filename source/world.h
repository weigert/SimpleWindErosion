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
  double heightmap[SIZE*SIZE] = {0.0};  //Flat Array

  double sealevel = 20.0;               //

  double windpath[SIZE*SIZE] = {0.0};   //Wind Strength
  double windxdir[SIZE*SIZE] = {0.0};   //Wind Strength
  double windydir[SIZE*SIZE] = {0.0};   //Wind Strength
  double sediment[SIZE*SIZE] = {0.0};   //Sedimentation Pile
  double tmp[SIZE*SIZE] = {0.0};        //Temporary Array

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
    //heightmap[i] += (tmp[i] - min)/(max - min);
    sediment[i] += (tmp[i] - min)/(max - min);
    //sediment[i] = 0.1;
  }


  /*
  //Note: Uncomment to place a huge pyramid in the middle

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
  */

}

/*
===================================================
          HYDRAULIC EROSION FUNCTIONS
===================================================
*/

void World::erode(int cycles){

  //Track the Movement of all Particles
  bool track[dim.x*dim.y] = {false};

  double xtrack[dim.x*dim.y] = {0.0};
  double ytrack[dim.x*dim.y] = {0.0};

  //Do a series of iterations!
  for(int i = 0; i < cycles; i++){

    //Spawn New Particle on Boundary
    glm::vec2 newpos;
    int shift = rand()%(int)(dim.x+dim.y);
    if(shift < dim.x) newpos = glm::vec2(shift, 1);
    else              newpos = glm::vec2(1, shift-dim.x);

    Wind wind(newpos);
    wind.fly(heightmap, windpath, sediment, track, dim, scale, xtrack, ytrack);

  }

  //Update Path
  double lrate = 0.01;
  for(int i = 0; i < dim.x*dim.y; i++){
    windpath[i] = (1.0-lrate)*windpath[i] + lrate*((track[i])?1.0:0.0);
    if(xtrack[i])
      windxdir[i] = 0.9*windxdir[i] + 0.1*((xtrack[i]));
    if(ytrack[i])
      windydir[i] = 0.9*windydir[i] + 0.1*((ytrack[i]));

  }

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
glm::vec3 flatColor = glm::vec3(0.80, 0.68, 0.44);
glm::vec3 sedimentColor = glm::vec3(0.96, 0.48, 0.32);
glm::vec3 steepColor = sedimentColor;//glm::vec3(0.78, 0.6, 0.168);

//Lighting and Shading
//glm::vec3 skyCol = glm::vec4(0.64, 0.75, 0.9, 1.0f);
glm::vec3 skyCol = glm::vec4(0.3, 0.42, 0.48, 1.0f);
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
      bool cover1 = (world.sediment[ind] > 0.001 &&
                     world.sediment[ind+world.dim.y] > 0.001 &&
                     world.sediment[ind+1] > 0.001);

      bool cover2 = (world.sediment[ind+world.dim.y] > 0.001 &&
                     world.sediment[ind+1] > 0.001 &&
                     world.sediment[ind+world.dim.y+1] > 0.001);

      //Add the Pool Height
      a += glm::vec3(0.0, world.scale*world.sediment[ind], 0.0);
      b += glm::vec3(0.0, world.scale*world.sediment[ind+world.dim.y], 0.0);
      c += glm::vec3(0.0, world.scale*world.sediment[ind+1], 0.0);
      d += glm::vec3(0.0, world.scale*world.sediment[ind+world.dim.y+1], 0.0);

      //UPPER TRIANGLE
      glm::vec3 color;
      if(!cover1) color = flatColor;
      else color = sedimentColor;//mix(flatColor, sedimentColor, ease::langmuir(world.sediment[ind], 1000.0));

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

        m->colors.push_back(color.x);
        m->colors.push_back(color.y);
        m->colors.push_back(color.z);
        m->colors.push_back(1.0);

      }

      if(!cover2) color = flatColor;
      else color = sedimentColor;//mix(flatColor, sedimentColor, ease::langmuir(world.sediment[ind], 1000.0));

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

        m->colors.push_back(color.x);
        m->colors.push_back(color.y);
        m->colors.push_back(color.z);
        m->colors.push_back(1.0);

      }
    }
  }
};

std::function<void()> eventHandler = [](){

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

std::function<glm::vec4(double, double)> dunemap = [](double t1, double t2){
  glm::vec4 color = glm::mix(glm::vec4(0.0,0.0,0.0,1.0), glm::vec4(1.0), ease::langmuir(t1, 10.0));
  return color;
};

std::function<glm::vec4(double, double)> directionmap = [](double xt, double yt){
  glm::vec4 color = glm::vec4(xt/2, 0.0, yt/2, 1.0);
  //std::cout<<color.x<<" "<<color.y<<" "<<color.z<<std::endl;
  return color;
};

std::function<glm::vec4(double, double, double, double)> combined = [](double t1, double t2, double xt, double yt){
  glm::vec4 color = glm::mix(glm::vec4(0.0,0.0,0.0,1.0), glm::vec4(0.96, 0.48, 0.32, 1.0), ease::langmuir(t1, 10.0));
  color = glm::mix(glm::vec4(xt/2, 0.0, yt/2, 1.0), color, ease::langmuir(t1, 10.0));
  return color;
};
