#include "TinyEngine/TinyEngine.h"
#include <noise/noise.h>
#include "source/world.h" //Model

int main( int argc, char* args[] ) {

  if(argc == 2)
    world.SEED = std::stoi(args[1]);

  //Generate the World
  world.generate();

  //Initialize the Visualization
  Tiny::view.vsync = false;
  Tiny::init("Wind Erosion Simulator", WIDTH, HEIGHT);

  //Setup Shaders
  Shader shader("source/shader/default.vs", "source/shader/default.fs", {"in_Position", "in_Normal", "in_Color"});
  Shader depth("source/shader/depth.vs", "source/shader/depth.fs", {"in_Position"});
  Shader effect("source/shader/effect.vs", "source/shader/effect.fs", {"in_Quad", "in_Tex"});
  Shader billboard("source/shader/billboard.vs", "source/shader/billboard.fs", {"in_Quad", "in_Tex"});

  //Particle System Shaders
  Shader sprite("source/shader/sprite.vs", "source/shader/sprite.fs", {"in_Quad", "in_Tex", "in_Model"});
  Shader spritedepth("source/shader/spritedepth.vs", "source/shader/spritedepth.fs", {"in_Quad", "in_Tex", "in_Model"});

  //Setup Rendering Billboards
  Billboard shadow(2000, 2000, true); //800x800, depth only
  Billboard image(WIDTH, HEIGHT, false); //1200x800S

  //Setup 2D Images
  Billboard map(world.dim.x, world.dim.y, false); //Render target for automata
  map.raw(image::make<double>(world.dim, world.windpath, world.sediment, dunemap));

  //Setup World Model
  Model model(constructor);
  model.translate(-viewPos);

  //Visualization Hooks
  Tiny::event.handler = eventHandler;
	Tiny::view.interface = [](){};
  Tiny::view.pipeline = [&](){

    Tiny::view.target(glm::vec3(1.0,0.0,0.0));    //Prepare Target
    billboard.use();
    glActiveTexture(GL_TEXTURE0+0);
    glBindTexture(GL_TEXTURE_2D, map.texture);
    map.move(glm::vec2(0), glm::vec2(1));
    billboard.setMat4("model", map.model);
    map.render();

  };

  //Define a World Mesher?

  Tiny::loop([&](){
    //Do Erosion Cycles!
    if(!paused){

      //Erode the World and Update the Model
      //timer::benchmark<std::chrono::microseconds>([&]{
        world.erode(250);
      //});

      //world.grow();
      model.construct(constructor); //Reconstruct Updated Model

      //Redraw the Path and Death Image
      if(viewmap)
        map.raw(image::make<double>(world.dim, world.windpath, world.sediment, dunemap));

    }
  });

  return 0;
}
