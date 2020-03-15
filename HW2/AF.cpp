#include <vector>
#include <stdio.h>
#include <string>
#include <cstring>

#include <glm/glm.hpp>

#include "AF.hpp"

 // calculate triangle normal
 glm::vec3 triNormal (glm::vec3 point1, glm::vec3 point2, glm::vec3 point3) {
 glm::vec3 norm = glm::normalize(glm::cross(glm::vec3(point2-point1),glm::vec3(point3-point1)));
 return norm;
 }
 
 // calculate intersect with triangle
 float intersect(
 glm::vec3 ray,
 glm::vec3 startPoint,
 glm::vec3 point1,
 glm::vec3 point2,
 glm::vec3 point3
 ) {
 glm::vec3 norm = triNormal(point1, point2, point3);
 float intersectPoint = (glm::dot(norm,glm::vec3(point1-startPoint)))/(glm::dot(norm,ray));
 glm::vec3 tPh = glm::vec3(startPoint+ glm::vec3(ray*intersectPoint));
 glm::vec3 A = glm::cross(glm::vec3(point2-point1),glm::vec3(point3-point1));
 glm::vec3 A0 = glm::cross(glm::vec3(tPh-point2),glm::vec3(tPh-point3));
 glm::vec3 A1 = glm::cross(glm::vec3(tPh-point3),glm::vec3(tPh-point1));
 glm::vec3 A2 = glm::cross(glm::vec3(tPh-point1),glm::vec3(tPh-point2));
 float u,v,w;
 u = v = w = 0;
 if(std::max(abs(A[0]),std::max(abs(A[1]),abs(A[2]))) == abs(A[0])) {
 u = A1[0]/A[0];
 v = A2[0]/A[0];
 w = A0[0]/A[0];
 }
 else if(std::max(abs(A[0]),std::max(abs(A[1]),abs(A[2]))) == abs(A[1])) {
 u = A1[1]/A[1];
 v = A2[1]/A[1];
 w = A0[1]/A[1];
 }
 else if(std::max(abs(A[0]),std::max(abs(A[1]),abs(A[2]))) == abs(A[2])) {
 u = A1[2]/A[2];
 v = A2[2]/A[2];
 w = A0[2]/A[2];
 }
 //check if areas valid
 if((u<0 || u>1)||(v<0 || v>1)||(1-u-v<0 || 1-u-v>1)|| std::isnan(u) || std::isnan(v)) {
 intersectPoint = std::numeric_limits<float>::max();
 }
 
 return intersectPoint;
 }
