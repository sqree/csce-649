#ifndef AF_H
#define AF_H

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

glm::vec3 barycentric3 (
    glm::vec3 hitPoint,
    glm::vec3 point1,
    glm::vec3 point2,
    glm::vec3 point3
);

glm::vec3 collisionVel (
    glm::vec3 vel,
    glm::vec3 collN,
    GLfloat cor,
    GLfloat fric
);

//  set up structs for spring data containers
struct struts {
    glm::vec3 parameters; // k, d, l0
    glm::vec2 vertex_indices;
    glm::vec2 face_indices;
    std::string type;
};

struct faces {
    glm::vec3 strut_indices;
    glm::vec3 vertex_angles;
};

struct vertexMass {
    GLfloat mass;
    glm::vec3 vertexPos;
    glm::vec3 vertexVel;
    glm::vec3 vertexForce;
};

struct object {
    std::vector <struts> objSprings;
    std::vector <faces> objFaces;
    std::vector <vertexMass> objVertices;
};

std::vector <vertexMass> deriveState (
    std::vector <struts> springs,
    std::vector <vertexMass> currentState,
    std::vector <faces> objectFaces,
    std::vector <vertexMass> vF,
    GLfloat tS
);

#endif
