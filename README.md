# Simple Wind Erosion

Basic Idea: Use Particle Based Erosion to Simulate Wind Erosion

Generally "Aeolian Processes"

I want to explore these things for the common video game landscape generator.

Key Assumptions:
- 2D Map, No Layering (Fix Later!)
-

## Idea

Particle Based Wind Erosion.

Wind packets move around the landscape and transport sediment.

Key Effects:
- Vegetation Reduces Wind Velocities
- "Deflation": Wind Carries Dust Particles
    - "Surface Creep": Large Particles Roll on Ground (e.g. Dune)
    - "Saltation": Bouncing Across the Surface
    - "Suspension": Light small particles float in the air

    Note: Saltation > Suspension > Surface Creep (frequency)

- "Abrasion": Dust Particles Strike Surface, Abrading
- ""

Erosion is stronger in places where soil is dry, as it tends to abrade more easily.

Deflation is a question of how particles move based on the mass.
Abrasion is a question is mass transfer into the particle.

## Observed Effects
- Rock formations sculpted by Wind
- Dunes off of which sand is blown
- Dust storms during increased wind speeds?

We could consider a map of additional sediment.
How does this map move though?
We don't have a flood process, rather we have a process where we pass material along according to the gradient, i.e. some kind of cascading which slowly occurs.

## Properties of Soil
-> Cohesiveness, scales with wetness too.
-> Wetness / Dryness
-> Roughness / Density / Compactness



## Proposed Model
Mass Transfer + Movement Model

Key Questions:
1. How do particles move
2. How do they transport sediment?

Additional Maps:
1. Wind Velocity Map
2. Sedimentation Map





  /*
      How does a wind packet move?

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
      -> This occurs in an algorithm similar to the flooding algorithm?

      Process:
      -> Wind packets can carry a certain amount of sediment, using equilibrium mass transport.
      -> Wind packets with sediment convert non-covered area into sedimented areas via abrasion.
      -> Sedimented areas undergo a cascading algorithm that leads to the formation of slopes of sediment.
      -> Wind packets not in contact with the ground can't accumulate particles so the equilibrium concentration is zero and they lose sediment.
        -> This has a rate though

      If a particle is rolling then how is its velocity affected?
        -> Prevailing velocity minus drag minus gravity normal component.
        -> Basically there is a gravity force working on it as well.

        The particle otherwise slides along the surface if it trying to be force into one.
        It will move along with the prevailing wind speed. Every time this happens, abrasion occurs.
        Or rather every time this occurs we have mass transport, either to gain sediment from sedimented ground
        or to abrade the ground.

      Cascading algorithm is a whole different question.

      Basically this algorithm uses something like cellular automata?
      When do I cascade?
      Because a particle can deposit all the time.

      Every time mass-transfer occurs and something is triggered right?

  */
