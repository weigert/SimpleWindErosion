# Simple Wind Erosion

Basic Idea: Use Particle Based Erosion to Simulate Wind Erosion

Generally "Aeolian Processes"

I want to explore these things for the common video game landscape generator.

Key Assumptions:
- 2D Map, No Layering (Fix Later!)
-

## To Do
- Wind Dust in the Air Tracking
- Wind path influences other particles and their flight path
- Surface abrasion process
- Different soil types / layers
- Sediment can be water logged, and can be moved by liquid water
- Vegetation reduces wind velocity

## Idea

Particle Based Wind Erosion.

Wind packets move around the landscape and transport sediment.

Key Effects:
- "Deflation": Wind Carries Dust Particles
    - "Surface Creep": Large Particles Roll on Ground (e.g. Dune)
    - "Saltation": Bouncing Across the Surface
    - "Suspension": Light small particles float in the air
    Note: Saltation > Suspension > Surface Creep (frequency)
- "Abrasion": Dust Particles Strike Surface, Abrading

Erosion is stronger in places where soil is dry, as it tends to abrade more easily.

-> Soil properties matter

Deflation is a question of how particles move based on the mass.
Abrasion is a question is mass transfer into the particle.

Additionally, there is a settling process by cascading.

These three effects combined yield the wind erosion system.

## Desirable Effects
- Rock formations sculpted by Wind
- Dunes off of which sand is blown
- Dust storms during increased wind speeds?

## Proposed Model
Mass Transfer + Movement Model

Key Questions:
1. How do particles move
2. How do they transport sediment?

Additional Maps:
1. Wind Velocity Map
2. Sedimentation Map

### How it Works

Wind Movement:
-> Wind has a 3D position and spawn randomly on the surface
-> Wind moves at prevailing wind speed, with prevailing wind direction.
-> Wind slides along the surface for now
-> Wind will fly off the edge and move straight if it is not at height map height.
-> If the heightmap is higher, it moves to the position.
-> If the heightmap is lower, it flys straight
-> Height in the air is affected by lift, while velocity is affected by drag
-> When sediment is placed, it can cascade the sediment pile which does a cellular automata pass
-> Wind direction is deflected by the landscape by reflecting around the surface normal

Later:
  Movement is affected by the wind direction map
  Particle sediment affects velocity with lift drag, and gravity
  Wind shadow somehow matters in this manner, as only where particles are on the ground we have contact?
  Wind velocity is reduced by vegetation

Mass Transport:
-> Raw ground is abraded into sand. this depends on mass, speed, shear force
-> Equilibrium transport then determines the amount of sediment that can be carried and moved

Cascading:
-> When sediment is deposited, it has a certain amount of friction
-> This determines how mass is moved around when there are gradients
-> Cellular automata style mass-passing to neighbors
