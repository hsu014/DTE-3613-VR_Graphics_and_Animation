#pragma once

class Scene
{
private:

public:
	Scene();
	~Scene();

};


/*
Scene must know:
  Lists of all objects (shapes)
    Divided into which shader to use
  List of all lights
  ActivateShaderBase
  ActivateShaderPhong(light)
  ActivateShaderParticle



Shapes must know
  Model matrix
  Material



Needed to render:
Base shader
  Model matrix
  [u]View matrix
  [u]Projection Matrix
  Material
  Texture?

Phong
  Model matrix
  [u]View matrix
  [u]Projection Matrix
  Material
  Texture?
  [u]Lights

Skybox
  [u]View matrix (modify)
  [u]Projection Matrix
  Texture

Emitter
  Model matrix
  [u]View matrix
  [u]Projection Matrix
  [u]Camera up
  [u]Camera front
  Texture


*/