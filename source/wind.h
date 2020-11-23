

struct Wind{
  //Construct Particle at Position
  Wind(glm::vec2 _pos){ pos = _pos; }
  Wind(glm::vec2 _p, glm::ivec2 dim){
    pos = _p;
    int index = _p.x*dim.y+_p.y;
  }

  //Properties
  int index;
  glm::vec2 pos;
  float height = 0.0;
  glm::vec3 pspeed = glm::vec3(1.0,0.0,1.0);
  glm::vec3 speed = glm::vec3(1.0,0.0,1.0);
  double sediment = 0.0; //Sediment Mass

  //Parameters
  const float dt = 0.6;
  const double density = 2.0;         //Affects inertia, volume, area
  const double abration = 0.001;    //Affects transport rate
  const double suspension = 0.0001;  //Affects transport rate
  const double dragcoef = 0.1;    //
  const double liftcoef = 0.1;    //
  const double minVol = 0.001;

  //Sedimenation Process
  void fly(double* h, double* path, double* pool, bool* track, double* pd, glm::ivec2 dim, double scale, double sealevel);
  void cascade(double* h, double* pool, glm::ivec2 dim);
};

glm::vec3 surfaceNormal(int index, double* h, glm::ivec2 dim, double scale){

  glm::vec3 n = glm::vec3(0.0);

  //Two large triangles adjacent to the plane (+Y -> +X) (-Y -> -X)
  for(int i = 1; i <= 1; i++){
    n += (1.0f/(float)i*i)*glm::cross(glm::vec3(0.0, scale*(h[index+i]-h[index]), i), glm::vec3( i, scale*(h[index+i*dim.y]-h[index]), 0.0));
    n += (1.0f/(float)i*i)*glm::cross(glm::vec3(0.0, scale*(h[index-i]-h[index]),-i), glm::vec3(-i, scale*(h[index-i*dim.y]-h[index]), 0.0));
    n += (1.0f/(float)i*i)*glm::cross(glm::vec3( i, scale*(h[index+i*dim.y]-h[index]), 0.0), glm::vec3(0.0, scale*(h[index-i]-h[index]),-i));
    n += (1.0f/(float)i*i)*glm::cross(glm::vec3(-i, scale*(h[index-i*dim.y]-h[index]), 0.0), glm::vec3(0.0, scale*(h[index+i]-h[index]), i));
  }

  return glm::normalize(n);
}

glm::vec3 surfaceNormal(glm::vec2 pos, double* h, glm::ivec2 dim, double scale){

  glm::ivec2 P00 = pos;  //Floored Position

  //Single Precision
//  return surfaceNormal(P00.x*dim.y+P00.y, h, dim, scale);

  glm::ivec2 P10 = P00 + glm::ivec2(1, 0);
  glm::ivec2 P01 = P00 + glm::ivec2(0, 1);
  glm::ivec2 P11 = P00 + glm::ivec2(1, 1);

  glm::vec3 N00 = surfaceNormal(P00.x*dim.y+P00.y, h, dim, scale);
  glm::vec3 N10 = surfaceNormal(P10.x*dim.y+P10.y, h, dim, scale);
  glm::vec3 N01 = surfaceNormal(P01.x*dim.y+P01.y, h, dim, scale);
  glm::vec3 N11 = surfaceNormal(P11.x*dim.y+P11.y, h, dim, scale);

  //Weights (modulo position)
  glm::vec2 w = 1.0f-glm::mod(pos, glm::vec2(1.0));
  return w.x*w.y*N00 + (1.0f-w.x)*w.y*N10 + w.x*(1.0f-w.y)*N01 + (1.0f-w.x)*(1.0f-w.y)*N11;

}

void Wind::fly(double* h, double* w, double* s, bool* track, double* pd, glm::ivec2 dim, double scale, double sealevel){

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

  glm::ivec2 ipos;

  int N =  1000;

  while(N-- > 0){

    //Initial Position
    ipos = pos;
    int ind = ipos.x*dim.y+ipos.y;

    //Set Height Correctly
    if(height < h[ind] + s[ind]) height = h[ind] + s[ind];

    //Surface Normal
    glm::vec3 n = surfaceNormal(pos, h, dim, scale);

    //Movement Mechanics
    if(height > h[ind] + s[ind]){

      speed.y -= dt*0.1;      //Gravity
                              //Drag
                              //Lift

      speed += 0.1f*dt*(pspeed - speed);      //Prevailing Wind

      pos += dt*glm::vec2(speed.x, speed.z);  //Adjust Position
      height += dt*speed.y;

    }
    else{ //On the Surface

      //Add to Path
      track[ind] = true;

      //Deflect Direction
      speed += glm::cross(glm::cross(speed,n),n);
      speed += 0.1f*dt*(pspeed - speed);

      pos += dt*glm::vec2(speed.x, speed.z);
      height += dt*speed.y;

    }

    //New Position
    int nind = (int)pos.x*dim.y+(int)pos.y;

    //Out-Of-Bounds
    if(!glm::all(glm::greaterThanEqual(pos, glm::vec2(0))) ||
       !glm::all(glm::lessThan((glm::ivec2)pos, dim))){
         break;
    }

    /*
        Future: Split Mass-Transport into:
        - Abrasion leads to sediment production
        - Suspension lifts and moves sediment
        - Cascading equalized sediment.
    */

    //Mass Transport
    if(height > h[nind] + s[nind]){
      sediment -= dt*0.01*sediment;    //No Abrasion
      h[ind] -= dt*0.001*sediment;
    }
    else{
      double cdiff = glm::length(speed)*(h[nind]-height);
      sediment += dt*0.01*cdiff;
      h[ind] -= dt*0.001*cdiff;
    }

    //Particle has no speed (equilibrium movement)
    if(length(speed) < 0.01)
      break;
  }
};

void Wind::cascade(double* h, double* p, glm::ivec2 dim){

  /*
      Cascading Algorithm:

      Every time sediment is deposited, we perform a cascade.

      The sediment map undergoes a cellular automata style distribution of material.

      In real life, a dropping particle only has physical contact with neighboring particles.
      This contact then "cascades" around with mass being moved back and forth until we reach some equilibrium pile shape.

      We have a friction coefficient that determines the threshold when a cascade occurs.

      Question: How is the mass redistributed?


  */

/*
  //Current Height
  index = (int)pos.x*dim.y + (int)pos.y;

//  if(p[index]*volumeFactor < volume)
//    return;

  double plane = h[index] + p[index];
  double initialplane = plane;


  //Floodset
  std::vector<int> set;
  int fail = 10;

  //Iterate
  while(volume > minVol && fail){

    set.clear();
    int size = dim.x*dim.y;
    bool tried[size] = {false};

    int drain;
    bool drainfound = false;

    std::function<void(int)> floodfill = [&](int i){
      //Out of Bounds
      if(i < 0 || i >= size) return;

      //Position has been tried
      if(tried[i]) return;
      tried[i] = true;

      //Wall / Boundary
      if(plane < h[i] + p[i]) return;

      //Drainage Point
      if( initialplane > h[i] + p[i] ){

        //No Drain yet
        if(!drainfound)
          drain = i;

        //Lower Drain
        else if( p[drain] + h[drain] < p[i] + h[i] )
          drain = i;

        drainfound = true;
        return;
      }

      set.push_back(i);
      floodfill(i+1);
      floodfill(i+dim.y+1);  //Fill Diagonals
      floodfill(i+dim.y);    //Fill Neighbors
      floodfill(i+dim.y-1);
      floodfill(i-1);
      floodfill(i-dim.y-1);
      floodfill(i-dim.y);
      floodfill(i-dim.y+1);
    };

    //Perform Flood
    floodfill(index);

    //Drainage Point
    if(drainfound){

      //Set the Drop Position and Evaporate
      pos = glm::vec2(drain/dim.y, drain%dim.y);

      //Set the New Waterlevel (Slowly)
      double drainage = 0.001;
      plane = (1.0-drainage)*initialplane + drainage*(h[drain] + p[drain]);

      //Compute the New Height
      for(auto& s: set)
        p[s] = (plane > h[s])?(plane-h[s]):0.0;

      //Remove Sediment
      sediment *= 0.2;
      break;
    }

    //Get Volume under Plane
    double tVol = 0.0;
    for(auto& s: set)
      tVol += volumeFactor*(plane - (h[s]+p[s]));

    //We can partially fill this volume
    if(tVol <= volume && initialplane < plane){

      //Raise water level to plane height
      for(auto& s: set)
        p[s] = plane - h[s];

      //Adjust Drop Volume
      volume -= tVol;
      tVol = 0.0;
    }

    //Plane was too high.
    else fail--;

    //Adjust Planes
    initialplane = (plane > initialplane)?plane:initialplane;
    plane += 0.5*(volume-tVol)/(double)set.size()/volumeFactor;
  }

  //Couldn't place the volume (for some reason)- so ignore this drop.
  if(fail == 0)
    volume = 0.0;
    */
}
