#ifndef AF3_H
#define AF3_H

// calculate triangle normal
glm::vec3 triNormal (
                     glm::vec3 point1,
                     glm::vec3 point2,
                     glm::vec3 point3
                     );

// calculate intersect with triangle
float intersect(
                glm::vec3 ray,
                glm::vec3 startPoint,
                glm::vec3 point1,
                glm::vec3 point2,
                glm::vec3 point3
                );

#endif
