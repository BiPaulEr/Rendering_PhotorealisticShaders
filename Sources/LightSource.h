#ifndef LIGHTSOURCE_H
#define LIGHTSOURCE_H

#include <glm/glm.hpp>
#include <glm/ext.hpp>

class LightSource : public Transform {
public:
  LightSource() {}
  LightSource(glm::vec3 position, glm::vec3 color, float intensity, float a_c, float a_l, float a_q, float coneAngle, glm::vec3 direction) :
  Transform(), position(position), color(color), intensity(intensity), a_c(a_c), a_l(a_l), a_q(a_q), coneAngle(coneAngle), direction(direction) {}
  ~LightSource() {}
  glm::vec3 getPosition() {return position;}
  glm::vec3 getColor() {return color;}
  float getIntensity() {return intensity;}
  float getA_c() {return a_c;}
  float getA_l() {return a_l;}
  float getA_q() {return a_q;}
  float getConeAngle() {return coneAngle;}
  glm::vec3 getDirection() {return direction;}
  void setConeAngle(float coneAngle_) {coneAngle = coneAngle_;}
  void setDirection(glm::vec3 direction_) {direction = direction_;}

private:
  glm::vec3 position;
  glm::vec3 color;
	float intensity;
  float a_c;
  float a_l;
  float a_q;
  float coneAngle;
  glm::vec3 direction;
};

#endif // LIGHTSOURCE_H
