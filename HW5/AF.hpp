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

struct rigidBodyState {
    glm::vec3 objPos;
    //R = [ux, uy, uz]
    glm::mat3 objOrient;
    //P = mv
    glm::vec3 objP;
    //L = Iw
    glm::vec3 objL;
};
struct AABB {
    GLfloat maxX;
    GLfloat maxY;
    GLfloat maxZ;
    GLfloat minX;
    GLfloat minY;
    GLfloat minZ;
};

struct object {
    glm::vec3 CoM;
    glm::mat3 MoI;
    rigidBodyState objState;
};

glm::vec3 centerOfMass (
    GLfloat vertices[],
    int verticesSize);

glm::mat3 boxMoI (
    GLfloat mass,
    GLfloat w,
    GLfloat l,
    GLfloat h);

bool AABBcol (
    AABB bV1,
    AABB bV2
);

rigidBodyState deriveRBState (
    rigidBodyState currentState,
    rigidBodyState currentDerivedState,
    GLfloat timeStep
);

#endif
