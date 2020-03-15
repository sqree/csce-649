#include <vector>
#include <stdio.h>
#include <string>
#include <cstring>

#include <glm/glm.hpp>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

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

glm::vec3 barycentric3 (glm::vec3 hitPoint, glm::vec3 point1, glm::vec3 point2, glm::vec3 point3) {
    glm::vec3 A = glm::cross(glm::vec3(point2-point1),glm::vec3(point3-point1));
    glm::vec3 A0 = glm::cross(glm::vec3(hitPoint-point2),glm::vec3(hitPoint-point3));
    glm::vec3 A1 = glm::cross(glm::vec3(hitPoint-point3),glm::vec3(hitPoint-point1));
    glm::vec3 A2 = glm::cross(glm::vec3(hitPoint-point1),glm::vec3(hitPoint-point2));
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
    glm::vec3 bCoord = glm::vec3(u, v, w);
    return bCoord;
}

glm::vec3 collisionVel (glm::vec3 vel, glm::vec3 collN, GLfloat cor, GLfloat fric) {
    glm::vec3 velN = glm::vec3(glm::dot(vel, collN)*collN);
    glm::vec3 velT = glm::vec3(vel-velN);
    velN = -cor*velN; // coeff of restitution
    velT = (1-fric)*velT; // friction
    glm::vec3 collV = velN+velT;
    return collV;
}

glm::vec3 centerOfMass (GLfloat vertices[], int verticesSize) {
    glm::vec3 d, e, f, tetraCoM;
    GLfloat tetraV;
    glm::vec3 objCoM = glm::vec3(0.0f,0.0f,0.0f);
    GLfloat objV = 0;
    for(int i=0; i<verticesSize; i++) {
        d = glm::vec3(vertices[9*i], vertices[9*i+1], vertices[9*i+2]);
        e = glm::vec3(vertices[9*i+3], vertices[9*i+4], vertices[9*i+5]);
        f = glm::vec3(vertices[9*i+6], vertices[9*i+7], vertices[9*i+8]);
        tetraV = (1.0f/6.0f)*glm::dot(d,glm::cross(e,f));
        tetraCoM = (1.0f/4.0f)*(d+e+f);
        objV = objV + tetraV;
        objCoM = objCoM + tetraV*tetraCoM;
    }
    return (objCoM/objV);
}

glm::mat3 boxMoI (GLfloat mass, GLfloat w, GLfloat l, GLfloat h) {
    glm::mat3 MoI;
    for(int i = 0; i<3; i++) {
        for(int j = 0; j<3; j++) {
            if(i==0 && i==j) {
                MoI[i][j] = (mass/12.0f)*(w*w+h*h);
            }
            else if(i==1 && i==j) {
                MoI[i][j] = (mass/12.0f)*(l*l+h*h);
            }
            else if(i==2 && i==j) {
                MoI[i][j] = (mass/12.0f)*(l*l+w*w);
            }
            else {
                MoI[i][j] = 0;
            }
        }
    }
    return MoI;
}

bool AABBcol (AABB bV1, AABB bV2) {
    bool col;
    if((bV2.minX > bV1.maxX) || (bV1.minX > bV2.maxX) || (bV2.minY > bV1.maxY) || (bV1.minY > bV2.maxY) || (bV2.minZ > bV1.maxZ) || (bV1.minZ > bV2.maxZ)) {
        col = false;
    }
    else {
        col = true;
    }
    return col;
}

rigidBodyState deriveRBState (rigidBodyState currentState, rigidBodyState currentDerivedState, GLfloat timeStep) {
    rigidBodyState dState;
    //update initial state for new state calculations
    dState.objPos = currentState.objPos + currentDerivedState.objPos*timeStep;
    dState.objOrient = currentState.objOrient + currentDerivedState.objOrient*timeStep;
    dState.objP = currentState.objP + currentDerivedState.objP*timeStep;
    dState.objL = currentState.objL + currentDerivedState.objL*timeStep;
    
    //get I0
    glm::mat3 localMoI = boxMoI(1.0f, 4.0f, 4.0f, 4.0f);
    
    //calculate inverse world moment of inertia I(-1) = R*I0(-1)*transpose(R)
    glm::mat3 iWorldMoI = dState.objOrient*glm::inverse(localMoI)*glm::transpose(dState.objOrient);
    
    //get w for new state calculations
    glm::vec3 w = iWorldMoI*dState.objL;
    
    //get cross product matrix version of w
    GLfloat mwCP[9] = {0.0f, w[2], -w[1], -w[2], 0.0f, w[0], w[1], -w[0], 0.0f};
    glm::mat3 wCP = glm::make_mat3(mwCP);
    //get Rdot
    glm::mat3 wR = wCP*dState.objOrient;
    
    //calculate external forces
    glm::vec3 g = glm::vec3(0.0f,-0.7f,0.0f);
    glm::vec3 forces = g - 0.1f*dState.objP;
    glm::vec3 torques = -0.1f*dState.objL;
    
    //return dx
    dState.objPos = dState.objP;
    //return wR
    dState.objOrient = wR;
    //return dP = forces
    dState.objP = forces;
    //update dL = torque
    dState.objL = torques;
    
    return dState;
}
