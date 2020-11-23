#include "vegetation.h"
#include "wind.h"

#define SIZE 256

class World{
public:
  //Constructor
  void generate();                      //Initialize Heightmap
  void erode(int cycles);               //Erode with N Particles
  void grow();

  int SEED = 0;
  glm::ivec2 dim = glm::vec2(SIZE, SIZE);  //Size of the heightmap array

  double scale = 40.0;                  //"Physical" Height scaling of the map
  double heightmap[SIZE*SIZE] = {0.0};    //Flat Array

  double sealevel = 20.0;               //

  double windpath[SIZE*SIZE] = {0.0};    //Wind Strength
  double wspeedx[SIZE*SIZE] = {1.0};     //Wind Strength
  double wspeedy[SIZE*SIZE] = {1.0};    //Wind Strength
  double sediment[SIZE*SIZE] = {0.0};    //Sedimentation Pile

  double tmp[SIZE*SIZE] = {0.0};          //Temporary Array
  void diffuse(float D, float dt);

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
    heightmap[i] = perlin.GetValue((i/dim.y)*(1.0/dim.x), (i%dim.y)*(1.0/dim.y), SEED);
    if(heightmap[i] > max) max = heightmap[i];
    if(heightmap[i] < min) min = heightmap[i];
  }
  //Normalize
  for(int i = 0; i < dim.x*dim.y; i++){
    heightmap[i] = (heightmap[i] - min)/(max - min);
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


  //  int spill = 5;
  //  while(drop.volume > drop.minVol && spill != 0){

      wind.fly(heightmap, windpath, sediment, track, plantdensity, dim, scale, sealevel);

  //    if(drop.volume > drop.minVol)
  //      drop.flood(heightmap, sediment, dim);

  //    spill--;
  //  }
  }

  //Update Path
  double lrate = 0.01;
  for(int i = 0; i < dim.x*dim.y; i++)
    windpath[i] = (1.0-lrate)*windpath[i] + lrate*((track[i])?1.0:0.0);

  //Diffusion Erosion (Cracking)
  //diffuse(0.001, 1.2);
}

void World::grow(){

/*
  //Random Position
  {
    int i = rand()%(dim.x*dim.y);
    glm::vec3 n = surfaceNormal(i, heightmap, dim, scale);

    if( sediment[i] == 0.0 &&
        windpath[i] < 0.2 &&
        n.y > 0.8 && (float)(rand()%10)/(10.0) > 0.5){

        Plant ntree(i, dim);
        ntree.root(plantdensity, dim, 1.0);
        trees.push_back(ntree);
    }
  }

  //Loop over all Trees
  for(int i = 0; i < trees.size(); i++){

    //Grow the Tree
    trees[i].grow();

    //Spawn a new Tree!
    if(rand()%50 == 0){
      //Find New Position
      glm::vec2 npos = trees[i].pos + glm::vec2(rand()%9-4, rand()%9-4);

      //Check for Out-Of-Bounds
      if( npos.x >= 0 && npos.x < dim.x &&
          npos.y >= 0 && npos.y < dim.y ){

        Plant ntree(npos, dim);
        glm::vec3 n = surfaceNormal(ntree.index, heightmap, dim, scale);

        if( sediment[ntree.index] == 0.0 &&
            windpath[ntree.index] < 0.2 &&
            n.y > 0.8 &&
            (double)(rand()%1000)/1000.0 > plantdensity[ntree.index]){
              ntree.root(plantdensity, dim, 1.0);
              trees.push_back(ntree);
            }
      }
    }

    //If the tree is in a pool or in a stream, kill it
    if(sediment[trees[i].index] > 0.0 ||
       windpath[trees[i].index] > 0.2 ||
       rand()%1000 == 0 ){ //Random Death Chance
         trees[i].root(plantdensity, dim, -1.0);
         trees.erase(trees.begin()+i);
         i--;
       }
  }
*/
};

void World::diffuse(float D, float dt){
  //dh/dt = D(d^2h/dx^2 + d^2h/dy^2)

  for(int i = 0; i < dim.x; i++){ //X
    for(int j = 0; j < dim.y; j++){ //Y
      tmp[i*dim.y+j] = 0;

      //Compute the Indices (X)
      int nx, cx, px;
      int ny, cy, py;

      if(i == 0){
        nx = i*dim.y+j;
        cx = (i+1)*dim.y+j;
        px = (i+2)*dim.y+j;
      }
      else if(i == dim.x-1){
        nx = (i-2)*dim.y+j;
        cx = (i-1)*dim.y+j;
        px = i*dim.y+j;
      }
      else{
        nx = (i-1)*dim.y+j;
        cx = i*dim.y+j;
        px = (i+1)*dim.y+j;
      }

      //Compute the Indices (Y)
      if(j == 0){
        ny = i*dim.y+j;
        cy = i*dim.y+j+1;
        py = i*dim.y+j+2;
      }
      else if(j == dim.y-1){
        ny = i*dim.y+j-2;
        cy = i*dim.y+j-1;
        py = i*dim.y+j;
      }
      else{
        ny = i*dim.y+j-1;
        cy = i*dim.y+j;
        py = i*dim.y+j+1;
      }

      tmp[i*dim.y+j] += D*abs(heightmap[px]-heightmap[nx])*(heightmap[px]+heightmap[nx]-2*heightmap[cx]);
      tmp[i*dim.y+j] += D*abs(heightmap[py]-heightmap[ny])*(heightmap[py]+heightmap[ny]-2*heightmap[cy]);
    }
  }

  for(int i = 0; i < dim.x*dim.y; i++){
    heightmap[i] += dt*tmp[i];
  }
};

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
      double p = ease::langmuir(world.windpath[ind], 10.0);

      //See if we are water or not!
      if(water1) color = waterColor;
      else color = flatColor;//glm::mix(flatColor, waterColor, p);

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
  glm::vec4 color = glm::mix(glm::vec4(0.0, 0.0, 0.0, 1.0), glm::vec4(0.96, 0.48, 0.32, 1.0), ease::langmuir(t1, 10.0));
  color = glm::mix(color, glm::vec4(1.0, 0.87, 0.73, 1.0), ease::langmuir(t2, 10.0));
  return color;
};
