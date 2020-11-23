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
