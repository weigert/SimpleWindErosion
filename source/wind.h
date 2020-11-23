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
  glm::vec3 pspeed = 2.0f*glm::normalize(glm::vec3(1.0,0.0,1.0));
  glm::vec3 speed = pspeed;
  double sediment = 0.0; //Sediment Mass

  //Parameters
  const float dt = 0.25;
  const double suspension = 0.0001;  //Affects transport rate

  //Sedimenation Process
  void fly(double* h, double* path, double* pool, bool* track, double* pd, glm::ivec2 dim, double scale, double sealevel);
  void cascade(int i, double* height, double* sediment, glm::ivec2 dim);
};

/*
    Note: It matters where we came from when changing the heights...
*/

glm::vec3 surfaceNormal(int index, double* h, double* s, glm::ivec2 dim, double scale){

  glm::vec3 n = glm::vec3(0.0);

  //Two large triangles adjacent to the plane (+Y -> +X) (-Y -> -X)
  for(int i = 1; i <= 1; i++){
    n += (1.0f/(float)i*i)*glm::cross(glm::vec3(0.0, scale*(h[index+i]-h[index]+s[index+i]-s[index]), i), glm::vec3( i, scale*(h[index+i*dim.y]-h[index]+s[index+i*dim.y]-s[index]), 0.0));
    n += (1.0f/(float)i*i)*glm::cross(glm::vec3(0.0, scale*(h[index-i]-h[index]+s[index-i]-s[index]),-i), glm::vec3(-i, scale*(h[index-i*dim.y]-h[index]+s[index-i*dim.y]-s[index]), 0.0));
    n += (1.0f/(float)i*i)*glm::cross(glm::vec3( i, scale*(h[index+i*dim.y]-h[index]+s[index+i*dim.y]-s[index]), 0.0), glm::vec3(0.0, scale*(h[index-i]-h[index]+s[index-i]-s[index]),-i));
    n += (1.0f/(float)i*i)*glm::cross(glm::vec3(-i, scale*(h[index-i*dim.y]-h[index]+s[index-i*dim.y]-s[index]), 0.0), glm::vec3(0.0, scale*(h[index+i]-h[index]+s[index+i]-s[index]), i));
  }

  return glm::normalize(n);
}

glm::vec3 surfaceNormal(glm::vec2 pos, double* h, double* s, glm::ivec2 dim, double scale){

  glm::ivec2 P00 = pos;  //Floored Position

  glm::ivec2 P10 = P00 + glm::ivec2(1, 0);
  glm::ivec2 P01 = P00 + glm::ivec2(0, 1);
  glm::ivec2 P11 = P00 + glm::ivec2(1, 1);

  glm::vec3 N00 = surfaceNormal(P00.x*dim.y+P00.y, h, s, dim, scale);
  glm::vec3 N10 = surfaceNormal(P10.x*dim.y+P10.y, h, s, dim, scale);
  glm::vec3 N01 = surfaceNormal(P01.x*dim.y+P01.y, h, s, dim, scale);
  glm::vec3 N11 = surfaceNormal(P11.x*dim.y+P11.y, h, s, dim, scale);

  //Weights (modulo position)
  glm::vec2 w = 1.0f-glm::mod(pos, glm::vec2(1.0));
  return w.x*w.y*N00 + (1.0f-w.x)*w.y*N10 + w.x*(1.0f-w.y)*N01 + (1.0f-w.x)*(1.0f-w.y)*N11;

}

void Wind::fly(double* h, double* w, double* s, bool* track, double* pd, glm::ivec2 dim, double scale, double sealevel){

  glm::ivec2 ipos;

  while(true){

    //Initial Position
    ipos = pos;
    int ind = ipos.x*dim.y+ipos.y;

    //Set Height Correctly
    if(height < h[ind] + s[ind]) height = h[ind] + s[ind];

    //Surface Normal (Using Heightmap + Sediment Map)
    glm::vec3 n = surfaceNormal(pos, h, s, dim, scale);

    //Movement Mechanics
    if(height > h[ind] + s[ind]){ //Flying
      speed.y -= dt*0.01; //Gravity
    }
    else{ //Contact Movement
      track[ind] = true;
      speed += glm::cross(glm::cross(speed,n),n);
    }

    speed += 0.1f*dt*(pspeed - speed);
    pos += dt*glm::vec2(speed.x, speed.z);
    height += dt*speed.y;

    //New Position
    int nind = (int)pos.x*dim.y+(int)pos.y;

    //Out-Of-Bounds
    if(!glm::all(glm::greaterThanEqual(pos, glm::vec2(0))) ||
       !glm::all(glm::lessThan((glm::ivec2)pos, dim)))
         break;

    /*
        Future: Split Mass-Transport into:
        - Abrasion leads to sediment production
        - Suspension lifts and moves sediment
        - Cascading equalized sediment.
    */

    //Mass Transport
    if(height <= h[nind] + s[nind]){ //Abrading Particle

      //Non-Abrading Process
      double cdiff = glm::length(speed)*(s[nind]+h[nind]-height);
      sediment += dt*0.01*cdiff;

      double change = dt*0.001*cdiff;

      if(s[nind] <= 0){
        s[nind] = 0;
        //Abrade Here
      }
      //Remove the Sediment First
      else if(s[nind] > change){
        s[nind] -= 0.5*change;
        s[ind] -= 0.5*change;
        cascade(nind, h, s, dim);
        cascade(ind, h, s, dim);
      }
      else s[ind] = 0;

    }
    else{ //Floating Particle

      sediment -= dt*0.005*sediment;  //Lose Sediment
      s[nind] += 0.5*dt*0.0005*sediment;  //Deposit as Loose
      s[ind] += 0.5*dt*0.0005*sediment;  //Deposit as Loose
      cascade(nind, h, s, dim);
      cascade(ind, h, s, dim);

    }

    //Particle has no speed (equilibrium movement)
    if(length(speed) < 0.01)
      break;
  }
};

void Wind::cascade(int i, double* h, double* s, const glm::ivec2 dim){

  /*
    To-Do: Make this neighbor recursive?

    Is this even necessary? Not sure!
  */

  const double thresh = 0.005; //Minimum Height Difference (Slope)
  const double settle = 0.01;  //Settling Rate

  const int size = dim.x*dim.y;

  //Neighbor Positions (8-Way)
  const int nx[8] = {-1,-1,-1,0,0,1,1,1};
  const int ny[8] = {-1,0,1,-1,1,-1,0,1};

  int n[8] = {i-dim.y-1, i-dim.y, i-dim.y+1, i-1, i+1,
              i+dim.y-1, i+dim.y, i+dim.y+1};

  glm::ivec2 ipos;

  //Iterate over all Neighbors
  for(int m = 0; m < 8; m++){

    ipos = pos;

    if(n[m] < 0 || n[m] >= size) continue;

    if(ipos.x+nx[m] >= dim.x || ipos.y+ny[m] >= dim.y) continue;
    if(ipos.x+nx[m] < 0 || ipos.y+ny[m] < 0) continue;

    //Pile Size Difference
    float diff = (h[i]+s[i]) - (h[n[m]]+s[n[m]]);
    float excess = abs(diff) - thresh;
    if(excess <= 0) continue;

    //Transfer Mass
    float transfer;
    if(diff > 0) //Pile is Larger
      transfer = min(s[i], excess/2.0);
    else         //Neighbor is Larger
      transfer = -min(s[n[m]], excess/2.0);

    s[i] -= dt*settle*transfer;
    s[n[m]] += dt*settle*transfer;

  }

}
