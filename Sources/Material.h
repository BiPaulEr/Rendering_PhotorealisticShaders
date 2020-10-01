#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/quaternion.hpp>

#include "ShaderProgram.h"
#include "stb_image.h"

class Material {
public:
  Material(glm::vec3 albedo, float roughness, glm::vec3 metallic) : albedo(albedo), roughness(roughness), metallic(metallic) {}
  ~Material() {}
  glm::vec3 getAlbedo() {return albedo;}
  float getRoughness() {return roughness;}
  glm::vec3 getMetallic() {return metallic;}
  void setAlbedo(glm::vec3 albedo_) {albedo = albedo_;}
  GLuint loadTextureFromFileToGPU (const std::string & filename);

private:
  glm::vec3 albedo;
  float roughness;
  glm::vec3 metallic;
};

#endif // MATERIAL_H
