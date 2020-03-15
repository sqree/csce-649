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
    window = glfwCreateWindow( 1024, 768, "Tutorial 04 - Colored Cube", NULL, NULL);
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
    
    // Projection matrix : 45¡ Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
    // Camera matrix
    glm::mat4 View       = glm::lookAt(
                                       glm::vec3(4,1.5,-3), // Camera is at (4,3,-3), in World Space
                                       glm::vec3(0,0,0), // and looks at the origin
                                       glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                                       );
    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model      = glm::mat4(1.0f);
    // Our ModelViewProjection : multiplication of our 3 matrices
    glm::mat4 MVP        = Projection * View * Model; // Remember, matrix multiplication is the other way around
    
    // Load the texture
    GLuint Texture = loadDDS("uvmap.DDS");
    
    // Get a handle for our "myTextureSampler" uniform
    GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");
    
    
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals; // Won't be used at the moment.
    bool res = loadOBJ("waterfall.obj", vertices, uvs, normals);
    
    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
    
    GLuint normalbuffer;
    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
    
    glUseProgram(programID);
    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
    
    
    
    GLuint vertexbuffer;
    GLuint colorbuffer;
    
    glm::vec3 cubeVertex;
    glm::vec3 cubeNormals;
    glm::vec3 cubeCenters;
    glm::vec3 ncubeNormals;
    glm::vec3 ncubeCenters;
    GLfloat fcube_vertex[vertices.size()*3];
    GLfloat g_color_buffer_data[vertices.size()*3];
    GLuint scolorbuffer;
    GLuint svertexbuffer;
    
    float sphereRadius = 0.015;
    Icosphere tSphere;
    tSphere.setRadius(sphereRadius);
    tSphere.setSubdivision(0);
    tSphere.setSmooth(false);
    tSphere.printSelf();
    
    glm::mat4 rotationMatrix(1);
    bool rotateCube = true;
    
    float simTime = 0;
    int timeStep = 0;
    
    bool init = true;
    float g = 0.07;
    float m = 1.0f;     //ball mass
    float dt = 0.1;
    float epsilon = 0.4;
    float tau = 0.2;
    float d = g/5;     //air res & wind coeff
    
    std::vector <glm::vec3> ballPos;
    std::vector <glm::vec3> ballVel;
    std::vector <glm::vec3> ballAcc;
    std::vector <glm::vec3> ballColor;
    std::vector <float> ballAge;
    std::vector <float> ballLifespan;
    int numberOfBalls = 0;
    int birthRate = 20;
    
    glm::vec3 windVel;
    bool vortex = false;
    
    // loop
    do{
        numberOfBalls = numberOfBalls + birthRate;
        
        //generate new particles
        for(int i = 0; i < birthRate; i++) {
            float xVel = std::rand() % 100;
            xVel = xVel/1000;
            //xVel = xVel/50-1;
            float yVel = std::rand() % 100;
            yVel = yVel/100-1;
            float zVel = std::rand() % 100;
            zVel = zVel/1000;
            //zVel = zVel/50-1;
            ballPos.push_back(glm::vec3(0.0f, 2.0f, -0.3f));
            ballVel.push_back(glm::vec3(xVel/3.0f, yVel/5.0f, zVel/3.0f));
            ballAcc.push_back(glm::vec3(0.0f, -g, 0.0f));
            float rContent = (std::rand() % 100);
            float gContent = (std::rand() % 100);
            float bContent = 214.0f/255.0f*100.0f;
            ballColor.push_back(glm::vec3(fmin(rContent/100,79.0f/255.0f), gContent/100, bContent/100));
            ballAge.push_back(0);
            float lifeSpan = fmax(std::rand() % 100,50);
            ballLifespan.push_back(lifeSpan/80*20);
        }
        
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Use our shader
        glUseProgram(programID);
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        
        // cube rotation interface
        if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
            rotationMatrix = glm::rotate(rotationMatrix, -0.1f, glm::vec3(1.0, 0.0, 0.0));
            rotateCube = true;
            //std::cout << "test\n";
        }
        if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS){
            rotationMatrix = glm::rotate(rotationMatrix, 0.1f, glm::vec3(1.0, 0.0, 0.0));
            rotateCube = true;
        }
        if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS){
            rotationMatrix = glm::rotate(rotationMatrix, 0.1f, glm::vec3(0.0, 0.0, 1.0));
            rotateCube = true;
        }
        if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS){
            rotationMatrix = glm::rotate(rotationMatrix, -0.1f, glm::vec3(0.0, 0.0, 1.0));
            rotateCube = true;
        }
        
        if(rotateCube) {
            for(int i=0; i<vertices.size(); i++) {
                cubeVertex =    glm::vec3(vertices[i][0],vertices[i][1],vertices[i][2]);
                cubeVertex = glm::vec3(rotationMatrix * glm::vec4(cubeVertex,1.0));
                fcube_vertex[3*i] = cubeVertex[0];
                fcube_vertex[3*i+1] = cubeVertex[1];
                fcube_vertex[3*i+2] = cubeVertex[2];
                g_color_buffer_data[3*i] =0.1f;
                g_color_buffer_data[3*i+1] =0.1f;
                g_color_buffer_data[3*i+2] =0.1f;
            }
            rotateCube = false;
        }
        
        // set mouse ctrl callback
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        
        // cull old particles
        int deadBalls = 0;
        std::vector<int> dB;
        for(int k = 0; k < numberOfBalls; k++) {
            if(ballAge[k] > ballLifespan[k]) {
                dB.push_back(k);
                deadBalls++;
            }
        }
        
        std::reverse(dB.begin(),dB.end());
        
        for(int i = 0; i < deadBalls; i++) {
            ballPos.erase(ballPos.begin()+dB[i]);
            ballVel.erase(ballVel.begin()+dB[i]);
            ballAcc.erase(ballAcc.begin()+dB[i]);
            ballColor.erase(ballColor.begin()+dB[i]);
            ballAge.erase(ballAge.begin()+dB[i]);
            ballLifespan.erase(ballLifespan.begin()+dB[i]);
            numberOfBalls = numberOfBalls-1;
        }
        
        std::cout << "# of particles: " << numberOfBalls << "\n";
        
        GLfloat fsphere_vertex[3*tSphere.getVertexCount()*numberOfBalls];
        GLfloat fsphere_color[3*tSphere.getVertexCount()*numberOfBalls];
        
        
        for(int k = 0; k < numberOfBalls; k++) {
            int count = 0;
            while (count<0.2/dt) {
                
                
                if (glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS){
                    vortex = false;
                }
                
                if (glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS){
                    vortex = true;
                }
                
                if(vortex) {
                    windVel = glm::vec3(glm::vec3(0.0,ballPos[k][1]+0.05f,-0.5)-ballPos[k]);
                    windVel = 5.0f*windVel/glm::length(windVel);
                }
                else {
                    windVel = glm::vec3(0.0f, 0.0f, 0.0f);
                }
                
                glm::vec3 newballPos, newballVel, newballAcc;
                newballPos = glm::vec3(ballPos[k][0] + ballVel[k][0]*dt, ballPos[k][1] + ballVel[k][1]*dt, ballPos[k][2] + ballVel[k][2]*dt);
                newballVel = glm::vec3(ballVel[k][0]+ballAcc[k][0]*dt, ballVel[k][1]+ballAcc[k][1]*dt, ballVel[k][2]+ballAcc[k][2]*dt);
                newballAcc = glm::vec3(-d*ballVel[k][0]+d*windVel[0], (-m*g)+(-d*ballVel[k][1])+d*windVel[1], -d*ballVel[k][2]+d*windVel[2]);
                
                glm::vec3 check_coll_center;
                glm::vec3 check_coll_normal;
                glm::vec3 check_coll_pos = newballPos;
                glm::vec3 coll_vel = newballVel;
                // check for collision w/all polygon planes
                if(ballPos[k][1]>-2.0) {
                    for(int i=0; i<vertices.size()/3; i++) {
                        glm::vec3 p0 = glm::vec3(fcube_vertex[9*i],fcube_vertex[9*i+1],fcube_vertex[9*i+2]);
                        glm::vec3 p1 = glm::vec3(fcube_vertex[9*i+3],fcube_vertex[9*i+4],fcube_vertex[9*i+5]);;
                        glm::vec3 p2 = glm::vec3(fcube_vertex[9*i+6],fcube_vertex[9*i+7],fcube_vertex[9*i+8]);;
                        check_coll_normal = triNormal(p0, p1, p2);
                        float hitPoint = intersect(coll_vel, check_coll_pos, p0, p1, p2);
                        if(glm::length(glm::vec3(coll_vel*hitPoint))<sphereRadius) { //collision w/triangle
                            coll_vel = glm::reflect(coll_vel, check_coll_normal);
                            glm::vec3 velNormal = glm::vec3(glm::dot(coll_vel, check_coll_normal)*check_coll_normal);
                            glm::vec3 velTangent = glm::vec3(coll_vel-velNormal);
                            velNormal = glm::vec3(epsilon*velNormal); // coeff of restitution
                            velTangent = glm::vec3((1-tau)*velTangent); // friction
                            newballVel = glm::vec3(velNormal+velTangent);
                            newballPos = glm::vec3(ballPos[k][0] + newballVel[0]*dt, ballPos[k][1] + newballVel[1]*dt, ballPos[k][2] + newballVel[2]*dt);
                        }
                    }
                }
                
                ballPos[k] = newballPos;
                ballVel[k] = newballVel;
                ballAcc[k] = newballAcc;
                ballAge[k] = ballAge[k]+dt;
                
                
                simTime = simTime + dt;
                timeStep++;
                
                count++;
            }
            
            //sphere transformation
            float const* p = tSphere.getVertices();
            
            for (int i = 0; i<tSphere.getVertexCount(); i++) {
                fsphere_vertex[3*i+k*tSphere.getVertexCount()*3] = *p + ballPos[k][0]; //x
                ++p;
                fsphere_vertex[3*i+1+k*tSphere.getVertexCount()*3] = *p + ballPos[k][1]; //y
                ++p;
                fsphere_vertex[3*i+2+k*tSphere.getVertexCount()*3] = *p + ballPos[k][2]; //z
                ++p;
                fsphere_color[3*i+k*tSphere.getVertexCount()*3] = (1-ballAge[k]/ballLifespan[k])*ballColor[k][0]+ballAge[k]/ballLifespan[k]*(205.f/255.0f);
                fsphere_color[3*i+1+k*tSphere.getVertexCount()*3] = (1-ballAge[k]/ballLifespan[k])*ballColor[k][1]+ballAge[k]/ballLifespan[k]*(205.f/255.0f);
                fsphere_color[3*i+2+k*tSphere.getVertexCount()*3] = (1-ballAge[k]/ballLifespan[k])*ballColor[k][2]+ballAge[k]/ballLifespan[k]*(205.f/255.0f);
            }
        }
        
        glm::vec3 lightPos = glm::vec3(4,4,4);
        glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glUniform1i(TextureID, 0);
        
        
        // draw cube
        glGenBuffers(1, &vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(fcube_vertex), fcube_vertex, GL_STATIC_DRAW);
        
        // 1st attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
                              0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
                              3,                  // size
                              GL_FLOAT,           // type
                              GL_FALSE,           // normalized?
                              0,                  // stride
                              (void*)0            // array buffer offset
                              );
        
        // 2nd attribute buffer : UVs
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glVertexAttribPointer(
                              1,                                // attribute
                              2,                                // size
                              GL_FLOAT,                         // type
                              GL_FALSE,                         // normalized?
                              0,                                // stride
                              (void*)0                          // array buffer offset
                              );
        
        // 3rd attribute buffer : normals
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
        glVertexAttribPointer(
                              2,                                // attribute
                              3,                                // size
                              GL_FLOAT,                         // type
                              GL_FALSE,                         // normalized?
                              0,                                // stride
                              (void*)0                          // array buffer offset
                              );
        
        
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());
        
        // draw sphere
        glGenBuffers(1, &svertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, svertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(fsphere_vertex), fsphere_vertex, GL_STATIC_DRAW);
        
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
        
        glDrawArrays(GL_TRIANGLES,0,tSphere.getVertexCount()*3*numberOfBalls);
        
        // Swap buffers
        glfwSwapInterval(1);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
          glfwWindowShouldClose(window) == 0 );
    
    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &colorbuffer);
    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &VertexArrayID);
    
    // Close OpenGL window and terminate GLFW
    glfwTerminate();
    
    return 0;
}
