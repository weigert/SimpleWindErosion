#include <TinyEngine/TinyEngine>
#include <TinyEngine/camera>
#include <TinyEngine/image>

#include "source/vertexpool.hpp"
#include "source/world.hpp"
#include "source/model.hpp"

#include <random>

mappool::pool<quad::cell> cellpool;
Vertexpool<Vertex> vertexpool;

int main( int argc, char* args[] ) {

  assert(TINYENGINE_VERSION == "1.7");

  Tiny::view.vsync = false;
  Tiny::view.blend = false;
  Tiny::window("Simple Wind Erosion", WIDTH, HEIGHT);
  glDisable(GL_CULL_FACE);

  //Initialize the World

  World world;

  if(argc >= 2){
    World::SEED = std::stoi(args[1]);
    srand(std::stoi(args[1]));
  }
  else {
    World::SEED = time(NULL);
    srand(World::SEED);
  }

  cellpool.reserve(quad::area);
  vertexpool.reserve(quad::tilearea, quad::maparea);
  World::map.init(vertexpool, cellpool, World::SEED);

  for(auto& node: world.map.nodes){
    updatenode(vertexpool, node);
  }

  // Initialize the Visualization

  // Camera

  cam::near = -800.0f;
  cam::far = 800.0f;
  cam::moverate = 10.0f;
  cam::look = glm::vec3(quad::size/2, quad::mapscale/2, quad::size/2);
  cam::roty = 60.0f;
  cam::rot = 180.0f;
  cam::init(3, cam::ORTHO);
  cam::update();

  //Setup Shaders

  Shader defaultshader({"source/shader/default.vs", "source/shader/default.fs"}, {"in_Position", "in_Normal", "in_Tangent", "in_Bitangent"});
  Shader defaultdepth({"source/shader/depth.vs", "source/shader/depth.fs"}, {"in_Position"});

  Shader ssaoshader({"source/shader/ssao.vs", "source/shader/ssao.fs"}, {"in_Quad", "in_Tex"});
  Shader imageshader({"source/shader/image.vs", "source/shader/image.fs"}, {"in_Quad", "in_Tex"});
  Shader mapshader({"source/shader/map.vs", "source/shader/map.fs"}, {"in_Quad", "in_Tex"});

  // Rendering Targets

  Billboard image(WIDTH, HEIGHT);             //1200x800, color and depth

  Texture shadowmap(8000, 8000, {GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT});
  Target shadow(8000, 8000);
  shadow.bind(shadowmap, GL_DEPTH_ATTACHMENT);

  Square2D flat;

  // SSAO

  Texture gPosition(WIDTH, HEIGHT, {GL_RGBA16F, GL_RGBA, GL_FLOAT});
  Texture gNormal(WIDTH, HEIGHT, {GL_RGBA16F, GL_RGBA, GL_FLOAT});
  Texture gColor(WIDTH, HEIGHT, {GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE});
  Texture gDepth(WIDTH, HEIGHT, {GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE});

  Target gBuffer(WIDTH, HEIGHT);
  gBuffer.bind(gPosition, GL_COLOR_ATTACHMENT0);
  gBuffer.bind(gNormal, GL_COLOR_ATTACHMENT1);
  gBuffer.bind(gColor, GL_COLOR_ATTACHMENT2);
  gBuffer.bind(gDepth, GL_DEPTH_ATTACHMENT);

  Texture ssaotex(WIDTH, HEIGHT, {GL_RED, GL_RED, GL_FLOAT});
  Target ssaofbo(WIDTH, HEIGHT);
  ssaofbo.bind(ssaotex, GL_COLOR_ATTACHMENT0);

  // generate sample kernel
  // ----------------------
  std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
  std::default_random_engine generator;
  std::vector<glm::vec3> ssaoKernel;
  for (unsigned int i = 0; i < 64; ++i){
      glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
      sample = glm::normalize(sample);
      sample *= randomFloats(generator);
      float scale = float(i) / 64.0f;
      scale = 0.1f + scale*scale*(1.0f-0.1f);
      sample *= scale;
      ssaoKernel.push_back(sample);
  }

  // generate noise texture
  // ----------------------
  std::vector<glm::vec3> ssaoNoise;
  for (unsigned int i = 0; i < 16; i++) {
      glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
      ssaoNoise.push_back(noise);
  }

  Texture noisetex(4, 4, {GL_RGBA32F, GL_RGB, GL_FLOAT}, &ssaoNoise[0]);
  glBindTexture(GL_TEXTURE_2D, noisetex.texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // Map

  Texture windmap(image::make([&](const ivec2 p){
    return vec4(0,0,0,0);
  }, quad::res));

  glm::mat4 mapmodel = glm::mat4(1.0f);
  mapmodel = glm::scale(mapmodel, glm::vec3(1,1,1)*glm::vec3((float)HEIGHT/(float)WIDTH, 1.0f, 1.0f));

  // Event Handler

  Tiny::event.handler = [&](){

    cam::handler();

    if(!Tiny::event.press.empty() && Tiny::event.press.back() == SDLK_p)
      paused = !paused;

    if(!Tiny::event.press.empty() && Tiny::event.press.back() == SDLK_m)
      viewmap = !viewmap;

  };

  Tiny::view.interface = [](){};
  Tiny::view.pipeline = [&](){

    // Full GBuffer

    gBuffer.target(vec3(0));
    defaultshader.use();
    defaultshader.uniform("proj", cam::proj);
    defaultshader.uniform("view", cam::view);
    defaultshader.uniform("flatColor", sedimentColor);
    defaultshader.uniform("steepColor", steepColor);
    vertexpool.render(GL_TRIANGLES);

    // SSAO Texture

    ssaofbo.target(vec3(0));
    ssaoshader.use();
    for (unsigned int i = 0; i < 64; ++i)
      ssaoshader.uniform("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
    ssaoshader.uniform("projection", cam::proj);
    ssaoshader.texture("gPosition", gPosition);
    ssaoshader.texture("gNormal", gNormal);
    ssaoshader.texture("texNoise", noisetex);
    ssaoshader.uniform("radius", ssaoradius);
    flat.render();

    //Render Shadowmap

    shadow.target();                  //Prepare Target
    defaultdepth.use();               //Prepare Shader
    defaultdepth.uniform("dvp", dvp);
    vertexpool.render(GL_TRIANGLES);  //Render Surface Model

    //Render Scene to Screen

    Tiny::view.target(skyCol);    //Prepare Target
    imageshader.use();
    imageshader.texture("gPosition", gPosition);
    imageshader.texture("gNormal", gNormal);
    imageshader.texture("gColor", gColor);
    imageshader.texture("gDepth", gDepth);
    imageshader.texture("ssaoTex", ssaotex);
    imageshader.texture("shadowMap", shadowmap);
    imageshader.uniform("view", cam::view);
    imageshader.uniform("dbvp", dbvp);
    imageshader.uniform("lightCol", lightCol);
    imageshader.uniform("skyCol", skyCol);
    imageshader.uniform("lightPos", lightPos);
    imageshader.uniform("lookDir", cam::pos);
    imageshader.uniform("lightStrength", lightStrength);
    flat.render();

    //Render Map to Screen

    if(viewmap){

      mapshader.use();
      mapshader.texture("windmap", windmap);
      mapshader.uniform("model", mapmodel);
      flat.render();

    }

  };

  int n = 0;
  Tiny::loop([&](){

    if(paused)
      return;

    world.erode(2*quad::tilesize);

    for(auto& node: world.map.nodes){
      updatenode(vertexpool, node);
    }
    cout<<n++<<endl;

    windmap.raw(image::make([&](const ivec2 p){
      double d = World::map.discharge(p);
      //std::cout<<d<<std::endl;
      return vec4(glm::mix(flatColor, steepColor, d), 1);
    }, quad::res));

  });

  return 0;
}
