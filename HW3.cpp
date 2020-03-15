// Include standard headers
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <limits>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <common/shader.hpp>
#include <common/controls.hpp>

// include sphere rendering stuff (courtesy of Song Ho Ahn (song.ahn@gmail.com))
#include <common/Icosphere.h>
#include <common/objloader.hpp>
#include <common/texture.hpp>

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
}

int main( void )
{
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return -1;
    }
    
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Open a window and create its OpenGL context
    window = glfwCreateWindow( 1024, 768, "HW 3 - flocking", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    
    //for interactivity
    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwPollEvents();
    glClearColor(0.9f, 0.9f, 0.9f, 0.0f);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    
    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders( "TransformVertexShader.vertexshader", "ColorFragmentShader.fragmentshader" );
    
    // Get a handle for our "MVP" uniform
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    
    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 Projection = glm::perspective(glm::radians(90.0f), 4.0f / 3.0f, 0.1f, 100.0f);
    // Camera matrix
    glm::mat4 View       = glm::lookAt(
                                       glm::vec3(0,0,3), // Camera is at (4,3,-3), in World Space
                                       glm::vec3(0,0,0), // and looks at the origin
                                       glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                                       );
    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model      = glm::mat4(1.0f);
    // Our ModelViewProjection : multiplication of our 3 matrices
    glm::mat4 MVP        = Projection * View * Model; // Remember, matrix multiplication is the other way around
    
    GLuint vertexbuffer;
    GLuint colorbuffer;
    
    GLuint scolorbuffer;
    GLuint svertexbuffer;
    
    float sphereRadius = 0.3;
    Icosphere tSphere;
    tSphere.setRadius(sphereRadius);
    tSphere.setSubdivision(2);
    tSphere.setSmooth(false);
    tSphere.printSelf();
    
    glm::mat4 rotationMatrix(1);
    
    float simTime = 0;
    int timeStep = 0;
    
    float g = 0.0;
    float dt = 0.005;
    float dar = 0.01;    //air res
    float dw = 0.1;     //wind coeff
    
    std::vector <glm::vec3> ballPos, ballVel, ballAcc, ballColor;
    std::vector <float> ballAge, ballLifespan;
    int numberOfBalls = 40;
    
    glm::vec3 windVel;
    GLfloat amax = 4.0f;
    float agentAngle;
    GLfloat r1 = 0.5;
    GLfloat r2 = 4.0;
    GLfloat theta1 = 60;
    GLfloat theta2 = 330;
    
    //flocking constants
    GLfloat ka, kv, kc;
    ka = 1.5f;
    kv = 0.5f;
    kc = 1.0;
    
    //generate particles
    for(int i = 0; i < numberOfBalls/4; i++) {
        float xVel = std::rand() % 100;
        xVel = xVel/1000;
        float yVel = std::rand() % 100;
        yVel = yVel/100;
        ballPos.push_back(glm::vec3(0.1*(i-numberOfBalls/8), -2.0f, 0.0f));
        ballVel.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
        ballAcc.push_back(glm::vec3(0.0f, -g, 0.0f));
        float rContent = (std::rand() % 100);
        float gContent = (std::rand() % 100);
        float bContent = 214.0f/255.0f*100.0f;
        ballColor.push_back(glm::vec3(fmin(rContent/100,79.0f/255.0f), gContent/100, bContent/100));
        ballAge.push_back(0);
        float lifeSpan = fmax(std::rand() % 100,50);
        ballLifespan.push_back(lifeSpan/80*20);
    }
    
    for(int i = 0; i < numberOfBalls/4; i++) {
        float xVel = std::rand() % 100;
        xVel = xVel/1000;
        float yVel = std::rand() % 100;
        yVel = yVel/100;
        ballPos.push_back(glm::vec3(0.1*(i-numberOfBalls/8), 2.0f, 0.0f));
        ballVel.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
        ballAcc.push_back(glm::vec3(0.0f, -g, 0.0f));
        float rContent = (std::rand() % 100);
        float gContent = (std::rand() % 100);
        float bContent = 214.0f/255.0f*100.0f;
        ballColor.push_back(glm::vec3(fmin(rContent/100,79.0f/255.0f), gContent/100, bContent/100));
        ballAge.push_back(0);
        float lifeSpan = fmax(std::rand() % 100,50);
        ballLifespan.push_back(lifeSpan/80*20);
    }
    
    for(int i = 0; i < numberOfBalls/4; i++) {
        float xVel = std::rand() % 100;
        xVel = xVel/100;
        float yVel = std::rand() % 100;
        yVel = yVel/1000;
        ballPos.push_back(glm::vec3(2.0f, 0.1*(i-numberOfBalls/8), 0.0f));
        ballVel.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
        ballAcc.push_back(glm::vec3(0.0f, -g, 0.0f));
        float rContent = (std::rand() % 100);
        float gContent = (std::rand() % 100);
        float bContent = 214.0f/255.0f*100.0f;
        ballColor.push_back(glm::vec3(fmin(rContent/100,79.0f/255.0f), gContent/100, bContent/100));
        ballAge.push_back(0);
        float lifeSpan = fmax(std::rand() % 100,50);
        ballLifespan.push_back(lifeSpan/80*20);
    }
    
    for(int i = 0; i < numberOfBalls/4; i++) {
        float xVel = std::rand() % 100;
        xVel = xVel/100;
        float yVel = std::rand() % 100;
        yVel = yVel/1000;
        ballPos.push_back(glm::vec3(-2.0f, 0.1*(i-numberOfBalls/8), 0.0f));
        ballVel.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
        ballAcc.push_back(glm::vec3(0.0f, -g, 0.0f));
        float rContent = (std::rand() % 100);
        float gContent = (std::rand() % 100);
        float bContent = 214.0f/255.0f*100.0f;
        ballColor.push_back(glm::vec3(fmin(rContent/100,79.0f/255.0f), gContent/100, bContent/100));
        ballAge.push_back(0);
        float lifeSpan = fmax(std::rand() % 100,50);
        ballLifespan.push_back(lifeSpan/80*20);
    }
    
    GLfloat tri_vertex[6*3*numberOfBalls];
    
    static const GLfloat tri_base[] = {
        -0.1f, 0.0f, 0.0f,
        0.0f, -0.025f, 0.0f,
        0.0f, -0.025f, 0.0f,
        0.0f,  0.025f, 0.0f,
        0.0f,  0.025f, 0.0f,
        -0.1f, 0.0f, 0.0f,
    };
    
    static const GLfloat cube_base[] = {
        -0.05, -0.05f, 0.05f,
        -0.05f, -0.05f, -0.05f,
        -0.05f, -0.05f, -0.05f,
        0.05f, -0.05f, -0.05f,
        0.05f, -0.05f, -0.05f,
        0.05f, -0.05f, 0.05f,
        0.05f, -0.05f, 0.05f,
        -0.05f,-0.05f,0.05f,
        
        -0.05f,0.05f,0.05f,
        -0.05f,0.05f,-0.05f,
        -0.05f,0.05f,-0.05f,
        0.05f,0.05f,-0.05f,
        0.05f,0.05f,-0.05f,
        0.05f,0.05f,0.05f,
        0.05f,0.05f,0.05f,
        -0.05f,0.05f,0.05f,
        
        -0.05f,-0.05f,0.05f,
        -0.05f,0.05f,0.05f,
        -0.05f,-0.05f,-0.05f,
        -0.05f,0.05f,-0.05f,
        0.05f,-0.05f,-0.05f,
        0.05f,0.05f,-0.05f,
        0.05f,-0.05f,0.05f,
        0.05f,0.05f,0.05f
    };
    
    GLfloat cube_vertices[] = {
        -0.05f,-0.05f,0.05f,
        -0.05f,0.05f,0.05f,
        -0.05f,0.05f,-0.05f,
        -0.05f,-0.05f,-0.05f,
        0.05f,0.05f,-0.05f,
        0.05f,0.05f,0.05f,
        0.05f,-0.05f,-0.05f,
        0.05f,-0.05f,0.05f
    };
    
    
    GLfloat fsphere_vertex[3*tSphere.getVertexCount()];
    GLfloat sphere_color[3*tSphere.getVertexCount()];
    float const* p = tSphere.getVertices();
    for (int i = 0; i<tSphere.getVertexCount(); i++) {
        fsphere_vertex[3*i] = *p;
        p++;
        fsphere_vertex[3*i+1] = *p;
        p++;
        fsphere_vertex[3*i+2] = *p;
        p++;
        sphere_color[3*i] = 0.9f;
        sphere_color[3*i+1] = 0.4f;
        sphere_color[3*i+2] = 0.3f;
    }
    GLfloat fsphere_color[3*6*numberOfBalls]; //repurposed for triangles
    for (int k = 0; k<numberOfBalls; k++) {
        for (int i = 0; i<6; i++) {
            fsphere_color[3*i+k*6*3] = (1-ballAge[k]/ballLifespan[k])*ballColor[k][0]+ballAge[k]/ballLifespan[k]*(205.f/255.0f);
            fsphere_color[3*i+1+k*6*3] = (1-ballAge[k]/ballLifespan[k])*ballColor[k][1]+ballAge[k]/ballLifespan[k]*(205.f/255.0f);
            fsphere_color[3*i+2+k*6*3] = (1-ballAge[k]/ballLifespan[k])*ballColor[k][2]+ballAge[k]/ballLifespan[k]*(205.f/255.0f);
        }
    }
    
    // loop
    do{
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Use our shader
        glUseProgram(programID);
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        
        // set mouse ctrl callback
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        
        glm::vec3 newballPos, newballVel, newballAcc;
        std::vector <glm::vec3> fballPos, fballVel, fballAcc;
        
        for(int k = 0; k < numberOfBalls; k++) {
            int count = 0;
            while (count<0.02/dt) {
                
                windVel = glm::vec3(cos(0.01*simTime),0.0f,0.0f);
                
                
                newballPos = glm::vec3(ballPos[k][0] + ballVel[k][0]*dt, ballPos[k][1] + ballVel[k][1]*dt, ballPos[k][2] + ballVel[k][2]*dt);
                
                // for flocking
                glm::vec3 collAvoidance, velMatching, centering, obsAvoidance;
                collAvoidance = velMatching = centering = obsAvoidance = glm::vec3(0.0f, 0.0f, 0.0f); // set flocking accelerations
                GLfloat neighborCount = 0; // set influence particle counter
                if(k==0) {
                    // store current state snapshot of ballPosition for flocking before updates
                    for(int i=0; i<numberOfBalls; i++) {
                        fballPos.push_back(ballPos[i]);
                        fballVel.push_back(ballVel[i]);
                        fballAcc.push_back(ballAcc[i]);
                    }
                }
                
                // calculate flocking accelerations
                for(int i = 0; i<numberOfBalls; i++) {
                    // for all other particles
                    if(i!=k) {
                        glm::vec3 xij = fballPos[i] - fballPos[k];
                        GLfloat dij = glm::length(xij);
                        // calculate distance weighting factor
                        GLfloat kd;
                        if(dij<r1) {
                            kd = 1.0f;
                        }
                        else if(dij<r2) {
                            kd = (r2-dij)/(r2-r1);
                        }
                        else {
                            kd=0.0f;
                        }
                        
                        // check within distance bound
                        if(dij<r2) {
                            //calculate FOV weighting factor
                            GLfloat ktheta;
                            
                            glm::vec3 u = xij/fmax(glm::length(xij),0.00001f);
                            glm::vec3 v = fballVel[k]/fmax(glm::length(fballVel[k]),0.00001f);
                            
                            GLfloat theta = atan2(glm::length(glm::cross(u,v)),glm::dot(u,v));
                            
                            if((theta>-theta1/2 && theta<theta1/2)) {
                                ktheta = 1.0;
                            }
                            else if(theta>theta1/2 && theta<theta2/2) {
                                ktheta = (theta2/2-abs(theta))/(theta2/2-theta1/2);
                            }
                            else {
                                ktheta = 0.0;
                            }
                            //kd = ktheta = 1.0f;
                            collAvoidance = collAvoidance-kd*ktheta*(ka/fmax(0.00005f,dij))*(xij/glm::length(xij)/fmax(0.00005f,dij));
                            velMatching = velMatching + kd*ktheta*kv*(fballVel[i] - fballVel[k]);
                            centering = centering + kd*ktheta*kc*xij;
                            neighborCount++;
                        }
                    }
                }
                if (neighborCount!=0) {
                    collAvoidance = collAvoidance/neighborCount;
                    velMatching = velMatching/neighborCount;
                    centering = centering/neighborCount;
                    //std::cout<< neighborCount << ", " << glm::length(collAvoidance) << ", " << glm::length(velMatching) << ", " << glm::length(centering) << "\n";
                }
                else {
                    collAvoidance = velMatching = centering = glm::vec3(0.0f, 0.0f, 0.0f);
                }
                
                //set acceleration limit
                GLfloat ar = amax;
                collAvoidance = fmin(ar, glm::length(collAvoidance))*(collAvoidance/fmax(glm::length(collAvoidance),0.00001f));
                ar = ar - glm::length(collAvoidance);
                velMatching = fmin(ar,glm::length(velMatching))*(velMatching/fmax(glm::length(velMatching),0.00001f));
                ar = ar - glm::length(velMatching);
                centering = fmin(ar, glm::length(centering))*(centering/fmax(glm::length(centering),0.00001f));
                
                // check for obstacles within certain range
                if(glm::length(ballPos[k]-glm::vec3(0.0f,0.0f,0.0f))<3.0) {
                    glm::vec3 ballTraj = fballVel[k]/glm::length(fballVel[k]);
                    glm::vec3 xis = glm::vec3(0.0f,0.0f,0.0f)-fballPos[k];
                    
                    GLfloat sclose = glm::dot(xis, ballTraj);
                    
                    glm::vec3 xclose = fballPos[k] + sclose*ballTraj;
                    
                    if(glm::length(xclose-glm::vec3(0.0f,0.0f,0.0f))< 1.5*sphereRadius) {
                        glm::vec3 vperp = xclose - glm::vec3(0.0f,0.0f,0.0f);
                        vperp = vperp/glm::length(vperp);
                        glm::vec3 xt = glm::vec3(0.0f,0.0f,0.0f)+2.0f*sphereRadius*vperp;
                        GLfloat dt = glm::length(xt-ballPos[k]);
                        GLfloat vt = glm::dot(fballVel[k],glm::vec3(xt-fballPos[k]))/dt;
                        GLfloat tt = dt/vt;
                        GLfloat deltavs = glm::length(glm::cross(ballTraj,xt-fballPos[k]))/tt;
                        GLfloat accMag = 2.0f*deltavs/tt;
                        
                        //correct accMag
                        GLfloat ep = glm::dot(vperp, -dar*fballVel[k]+dw*windVel+collAvoidance+velMatching+centering);
                        accMag = fmax(accMag-ep,0);
                        obsAvoidance = 1.5f*accMag*vperp;
                    }
                    else {
                        obsAvoidance = glm::vec3(0.0f,0.0f,0.0f);
                    }
                    
                    if(sclose<0) {
                        obsAvoidance = glm::vec3(0.0f,0.0f,0.0f);
                    }
                }
                
                newballAcc = glm::vec3(-dar*ballVel[k]+dw*windVel+collAvoidance+velMatching+centering+obsAvoidance);
                newballVel = glm::vec3(ballVel[k][0]+ballAcc[k][0]*dt, ballVel[k][1]+ballAcc[k][1]*dt, ballVel[k][2]+ballAcc[k][2]*dt);
                
                
                ballPos[k] = newballPos;
                //get new velocity angle
                agentAngle = atan2(-1.0f*newballVel[1]/glm::length(newballVel),glm::dot(glm::vec3(-1.0f,0.0f,0.0f),newballVel/glm::length(newballVel)));
                ballVel[k] = newballVel;
                
                ballAcc[k] = newballAcc;
                ballAge[k] = ballAge[k]+dt;
                
                
                simTime = simTime + dt;
                timeStep++;
                
                count++;
            }
            
            //triangle transformation
            glm::mat4 rotationMatrix(1);
            rotationMatrix = glm::rotate(rotationMatrix, agentAngle, glm::vec3(0.0, 0.0, 1.0));
            for (int i = 0; i<6; i++) {
                glm::vec3 filler_tri = glm::vec3(tri_base[3*i],tri_base[3*i+1],tri_base[3*i+2]);
                filler_tri = glm::vec3(rotationMatrix*glm::vec4(filler_tri,1.0f));
                tri_vertex[3*i+k*6*3] = filler_tri[0] + ballPos[k][0];
                tri_vertex[3*i+1+k*6*3] = filler_tri[1] + ballPos[k][1];
                tri_vertex[3*i+2+k*6*3] = filler_tri[2] + ballPos[k][2];
            }
        }
        
        // draw sphere
        glGenBuffers(1, &vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(fsphere_vertex), fsphere_vertex, GL_STATIC_DRAW);
        
        glGenBuffers(1, &colorbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_color), sphere_color, GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
                              0,
                              3,
                              GL_FLOAT,
                              GL_FALSE,
                              0,
                              (void*)0
                              );
        
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
        glVertexAttribPointer(
                              1,
                              3,
                              GL_FLOAT,
                              GL_FALSE,
                              0,
                              (void*)0
                              );
        
        glDrawArrays(GL_TRIANGLES,0,tSphere.getVertexCount()*3);
        
        glGenBuffers(1, &svertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, svertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(tri_vertex), tri_vertex, GL_STATIC_DRAW);
        
        glGenBuffers(1, &scolorbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, scolorbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(fsphere_color), fsphere_color, GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, svertexbuffer);
        glVertexAttribPointer(
                              0,
                              3,
                              GL_FLOAT,
                              GL_FALSE,
                              0,
                              (void*)0
                              );
        
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, scolorbuffer);
        glVertexAttribPointer(
                              1,
                              3,
                              GL_FLOAT,
                              GL_FALSE,
                              0,
                              (void*)0
                              );
        
        glDrawArrays(GL_LINES,0,3*3*numberOfBalls);
        
        
        // Swap buffers
        glfwSwapInterval(1);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
          glfwWindowShouldClose(window) == 0 );
    
    // Cleanup VBO and shader
    glDeleteBuffers(1, &svertexbuffer);
    glDeleteBuffers(1, &scolorbuffer);
    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &VertexArrayID);
    
    // Close OpenGL window and terminate GLFW
    glfwTerminate();
    
    return 0;
}
