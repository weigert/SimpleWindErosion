#ifndef SIMPLEWINDEROSION_MODEL
#define SIMPLEWINDEROSION_MODEL

const int WIDTH = 800;
const int HEIGHT = 800;

bool paused = true;
bool viewmap = false;
bool viewmomentum = false;

//Coloring
float steepness = 0.8;
glm::vec3 flatColor = glm::vec3(0.80, 0.68, 0.44);
glm::vec3 sedimentColor = glm::vec3(0.96, 0.48, 0.32);
glm::vec3 steepColor = sedimentColor;


glm::vec3 skyCol = glm::vec4(0.3, 0.42, 0.48, 1.0f);
glm::vec3 lightPos = glm::vec3(-100.0f, 100.0f, 150.0f);
glm::vec3 lightCol = glm::vec3(1.0f, 1.0f, 0.9f);

float ssaoradius = 10.0f;

float lightStrength = 1.2;

//Depth Map Rendering

glm::vec3 worldcenter = glm::vec3(quad::res.x/2, quad::mapscale/2, quad::res.y/2);

//Matrix for Making Stuff Face Towards Light (Trees)
float ds = quad::mapsize*400;
glm::mat4 dp = glm::ortho<float>(-ds, ds, -ds, ds, -ds, ds);
glm::mat4 dv = glm::lookAt(worldcenter + lightPos, worldcenter, glm::vec3(0,1,0));
glm::mat4 bias = glm::mat4(
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 0.5, 0.0,
    0.5, 0.5, 0.5, 1.0
);
glm::mat4 dvp = dp*dv;
glm::mat4 dbvp = bias*dvp;

#endif
