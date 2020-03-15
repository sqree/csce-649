#include <vector>
#include <stdio.h>
#include <string>
#include <cstring>

#include <glm/glm.hpp>
#include <GL/glew.h>

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

//calculate derived state
std::vector <vertexMass> deriveState (std::vector <struts> springs, std::vector <vertexMass> currentState, std::vector <faces> objectFaces, std::vector <vertexMass> vF, GLfloat tS) {
    vertexMass temp;
    std::vector <vertexMass> updatedState;
    glm::vec3 g = glm::vec3(0.0f, -0.05f, 0.0f);
    glm::vec3 windVel;
    float dar = 0.0f;    //air res
    float dw = 0.1;     //wind coeff
    
    // update position, velocity for new state force calculation
    for (int i=0; i < currentState.size(); i++) {
        currentState[i].vertexPos = currentState[i].vertexPos + vF[i].vertexVel*tS;
        currentState[i].vertexVel = currentState[i].vertexVel + vF[i].vertexForce*tS;
    }
    // for each spring
    for(int k = 0; k < springs.size(); k++) {
        glm::vec3 springPos, springVel, springAcc;
        glm::vec3 fSpring, fDamp, vj, vi, xij;
        GLfloat kSpring, d, l0, l;
        int pi, pj;
        kSpring = springs[k].parameters[0];
        d = springs[k].parameters[1];
        l0 = springs[k].parameters[2];
        pi = springs[k].vertex_indices[0];
        pj = springs[k].vertex_indices[1];
        vi = currentState[pi].vertexVel;
        vj = currentState[pj].vertexVel;
        xij = currentState[pj].vertexPos - currentState[pi].vertexPos;
        l = glm::length(xij);
        fSpring = kSpring*(l-l0)*xij/l;
        fDamp = d*glm::dot((vj-vi),xij/l)*xij/l;
        currentState[pi].vertexForce = currentState[pi].vertexForce + fSpring + fDamp; // push strut force to point i
        currentState[pj].vertexForce = currentState[pj].vertexForce-fSpring - fDamp; // push strut force to point j
    }
    
    //for torsional springs
    std::vector <int> x;
    int tempX, face1;
    GLfloat thetaNot = 0.0f;
    GLfloat ktheta = 0.3f;
    GLfloat dtheta = 0.8f;
    glm::vec3 f0, f1, f2, f3;
    
    for (int i=0; i < springs.size(); i++) {
        if(springs[i].type.compare("torsional")==0) {
            // set hinge points = torsional spring
            int springIndices = i;
            tempX = springs[springIndices].vertex_indices[0];
            x.push_back(tempX);
            tempX = springs[springIndices].vertex_indices[1];
            x.push_back(tempX);
            
            bool match = false;
            // for each face, get associated unique vertices
            while(x.size()<4) {
                for(int l=0; l < 2; l++) {
                    face1 = springs[springIndices].face_indices[l];
                    for(int m=0; m<3; m++) {
                        for(int n=0; n<2; n++) {
                            tempX = springs[objectFaces[face1].strut_indices[m]].vertex_indices[n];
                            for(int j=0; j < x.size(); j++) {
                                if(tempX == x[j]) {
                                    match = true;
                                }
                            }
                            if(!match) {
                                x.push_back(tempX);
                            }
                            match = false;
                        }
                    }
                }
            }
            //calculate xij, h
            glm::vec3 x01 = glm::vec3(currentState[x[1]].vertexPos - currentState[x[0]].vertexPos);
            glm::vec3 x02 = glm::vec3(currentState[x[2]].vertexPos - currentState[x[0]].vertexPos);
            glm::vec3 x03 = glm::vec3(currentState[x[3]].vertexPos - currentState[x[0]].vertexPos);
            glm::vec3 h = x01/glm::length(x01);
            glm::vec3 nl = glm::cross(h,x02/glm::length(x02));
            glm::vec3 nr = glm::cross(x03/glm::length(x03),h);
            glm::vec3 rl = x02-glm::dot(x02, h)*h;
            glm::vec3 rr = x03-glm::dot(x03, h)*h;
            
            GLfloat thetaCurrent = atan2(glm::dot(glm::cross(nl,nr),h),glm::dot(nl, nr))*180.0f/3.1415f;
            glm::vec3 tSpring = ktheta*(thetaCurrent-thetaNot)*h;
            
            // calculate speed in direction of face normal
            GLfloat sl = glm::dot(currentState[x[2]].vertexVel,nl);
            GLfloat sr = glm::dot(currentState[x[3]].vertexVel,nr);
            GLfloat thetal = sl/glm::length(rl);
            GLfloat thetar = sr/glm::length(rr);
            glm::vec3 tDamp = -dtheta*(thetal+thetar)*h;
            glm::vec3 tTotal = tSpring + tDamp;
            
            //calculate forces exerted on vertices
            f2 = nl*glm::dot(tTotal, h)/glm::length(rl);
            f3 = nr*glm::dot(tTotal, h)/glm::length(rr);
            GLfloat d02 = glm::dot(x02, h);
            GLfloat d03 = glm::dot(x03, h);
            f1 = -(d02*f2+d03*f3)/glm::length(x01);
            f0 = -(f1+f2+f3);
            
            // add forces to affected vertices
            currentState[x[0]].vertexForce = currentState[x[0]].vertexForce + f0;
            currentState[x[1]].vertexForce = currentState[x[1]].vertexForce + f1;
            currentState[x[2]].vertexForce = currentState[x[2]].vertexForce + f2;
            currentState[x[3]].vertexForce = currentState[x[3]].vertexForce + f3;
            x.clear();
        }
    }
    // update state vectors for each vertex
    for (int i=0; i < currentState.size(); i++) {
        // store K-state info
        temp.vertexPos = currentState[i].vertexPos;
        temp.vertexVel = currentState[i].vertexVel;
        temp.vertexForce = currentState[i].vertexForce + g + dar*currentState[i].vertexVel;
        updatedState.push_back(temp);
    }
    return updatedState;
}
