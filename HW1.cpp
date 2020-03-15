// Include standard headers
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

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

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
}

float intersect(glm::vec3 ray, glm::vec3 startPoint, glm::vec3 centerPoint, float radius) {
    float intersectPoint=0;
    float b = glm::dot(ray, glm::vec3(centerPoint-startPoint));
    float c = glm::dot(glm::vec3(centerPoint - startPoint),glm::vec3(centerPoint-startPoint))-radius*radius;
    intersectPoint = b - sqrt(b*b-c);
    return intersectPoint;
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
    
    // cube stuff
    static GLfloat g_vertex_buffer_data[] = {
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f,
        -1.0f, 1.0f,-1.0f,
        1.0f, 1.0f,-1.0f,
        1.0f, 1.0f,-1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f,-1.0f,
        -1.0f, -1.0f,-1.0f,
        1.0f, -1.0f,-1.0f,
        1.0f, -1.0f,-1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f,
        -1.0f, -1.0f,-1.0f,
        1.0f, 1.0f,-1.0f,
        1.0f, -1.0f,-1.0f
    };
    
    float cube_centers[] = {
        0.0f, -1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, 1.0f,
        -1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
    };
    
    float cube_normals[] = {
        0.0, 1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, -1.0f,
        1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
    };
    
    static const GLfloat g_color_buffer_data[] = {
        0.1f,  0.1f,  0.1f,
        0.1f,  0.1f,  0.1f,
        0.1f,  0.1f,  0.1f,
        0.1f,  0.1f,  0.1f,
        0.1f,  0.1f,  0.1f,
        0.1f,  0.1f,  0.1f,
        0.1f,  0.1f,  0.1f,
        0.1f,  0.1f,  0.1f,
        0.1f,  0.1f,  0.1f,
        0.1f,  0.1f,  0.1f,
        0.1f,  0.1f,  0.1f,
        0.1f,  0.1f,  0.1f,
        0.1f,  0.1f,  0.1f,
        0.1f,  0.1f,  0.1f,
        0.1f,  0.1f,  0.1f,
        0.1f,  0.1f,  0.1f,
        0.1f,  0.1f,  0.1f,
        0.1f,  0.1f,  0.1f,
        0.1f,  0.1f,  0.1f,
        0.1f,  0.1f,  0.1f,
        0.1f,  0.1f,  0.1f,
        0.1f,  0.1f,  0.1f,
        0.1f,  0.1f,  0.1f,
        0.1f,  0.1f,  0.1f
    };
    
    GLuint vertexbuffer;
    GLuint colorbuffer;
    GLuint ccolorbuffer;
    GLuint cvertexbuffer;
    
    
    // sphere stuff
    int numberOfBalls=1;
    float sphereRadius = 0.1;
    Icosphere tSphere;
    tSphere.setRadius(sphereRadius);
    tSphere.setSubdivision(2);
    tSphere.setSmooth(false);
    tSphere.printSelf();
    
    GLfloat fsphere_vertex[3*tSphere.getVertexCount()*numberOfBalls];
    GLfloat fsphere_color[3*tSphere.getVertexCount()*numberOfBalls];
    GLfloat fcube_vertex[sizeof(g_vertex_buffer_data)];
    GLfloat fcube_normals[6*3];
    GLfloat fcube_centers[6*3];
    GLfloat fcube_normallines[12*3];
    
    
    // set sphere color
    for (int i = 0; i<sizeof(fsphere_vertex)/4/3; i++) {
        fsphere_color[3*i] = 0.3f;
        fsphere_color[3*i+1] = 0.0f;
        fsphere_color[3*i+2] = 0.8f;
    }
    
    GLuint scolorbuffer;
    GLuint svertexbuffer;
    glGenBuffers(1, &scolorbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, scolorbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fsphere_color), fsphere_color, GL_STATIC_DRAW);
    
    // set initial conditions
    std::vector <glm::vec3> ballPos;
    std::vector <glm::vec3> ballVel;
    std::vector <glm::vec3> ballAcc;
    for(int i = 0; i < numberOfBalls; i++) {
        float hVel = std::rand() % 100;
        hVel = hVel/50;
        ballPos.push_back(glm::vec3(0.5*i-0.5, 1.0f-sphereRadius, 0.0f));
        ballVel.push_back(glm::vec3(0.0f, hVel, 0.0f));
        ballAcc.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
    }
    
    float g = 0.07f;
    float m = 1.0f;     //ball mass
    float dt = 0.02;
    float epsilon = 0.9;
    float tau = 0.1;
    float d = g/10;     //air res coeff
    
    for(int i = 0; i < numberOfBalls; i++) {
        ballAcc[i][1] = -g;
    }
    
    glm::vec3 cubeVertex;
    glm::vec3 cubeNormals;
    glm::vec3 cubeCenters;
    glm::vec3 ncubeNormals;
    glm::vec3 ncubeCenters;
    
    glm::mat4 rotationMatrix(1);
    bool rotateCube = true;
    
    float simTime = 0;
    int timeStep = 0;
    
    
    // loop
    do{
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
        
        if (glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS){
            dt = 0.2;
        }
        
        if (glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS){
            dt = 0.01;
        }
        
        // cube transformation
        if(rotateCube) {
            //std::cout << rotationMatrix[0][1] << ", " << (rotateCube) << "\n";
            for(int i=0; i<6; i++) {
                cubeCenters = glm::vec3(cube_centers[3*i],cube_centers[3*i+1],cube_centers[3*i+2]);
                cubeNormals = glm::vec3(cube_normals[3*i],cube_normals[3*i+1],cube_normals[3*i+2]);
                cubeCenters = glm::vec3(rotationMatrix * glm::vec4(cubeCenters,1.0));
                cubeNormals = glm::vec3(rotationMatrix * glm::vec4(cubeNormals,1.0));
                fcube_centers[3*i] = cubeCenters[0];
                fcube_centers[3*i+1] = cubeCenters[1];
                fcube_centers[3*i+2] = cubeCenters[2];
                fcube_normals[3*i] = cubeNormals[0];
                fcube_normals[3*i+1] = cubeNormals[1];
                fcube_normals[3*i+2] = cubeNormals[2];
                fcube_normallines[6*i] = cubeCenters[0];
                fcube_normallines[6*i+1] = cubeCenters[1];
                fcube_normallines[6*i+2] = cubeCenters[2];
                fcube_normallines[6*i+3] = cubeCenters[0]+0.5*cubeNormals[0];
                fcube_normallines[6*i+4] = cubeCenters[1]+0.5*cubeNormals[1];
                fcube_normallines[6*i+5] = cubeCenters[2]+0.5*cubeNormals[2];
            }
            for(int i=0; i<sizeof(g_vertex_buffer_data)/3; i++) {
                cubeVertex =    glm::vec3(g_vertex_buffer_data[3*i],g_vertex_buffer_data[3*i+1],g_vertex_buffer_data[3*i+2]);
                cubeVertex = glm::vec3(rotationMatrix * glm::vec4(cubeVertex,1.0));
                fcube_vertex[3*i] = cubeVertex[0];
                fcube_vertex[3*i+1] = cubeVertex[1];
                fcube_vertex[3*i+2] = cubeVertex[2];
            }
            rotateCube = false;
        }
        
        // set mouse ctrl callback
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        
        int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
        if(state) {
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);
            for (int i = 0; i < numberOfBalls; i++){
                ballPos[i][0] = 0.5*i-0.5;
                ballPos[i][1] = fmax(fmin((768/2-mouseY)/100,0.9),-0.9);
                ballAcc[i] = glm::vec3(0.0,-g,0.0);
            }
            epsilon = 0.9;
            tau = 0.1;
            simTime = 0;
            timeStep = 0;
        }
        
        for(int k = 0; k <numberOfBalls; k++) {
            std::cout << k << "\n";
            int count = 0;
            while (count<0.2/dt) {
                
                glm::vec3 windVel = glm::vec3(3*sin(simTime/20), 0.0f, 0.0f);
                
                glm::vec3 newballPos, newballVel, newballAcc;
                newballPos = glm::vec3(ballPos[k][0] + ballVel[k][0]*dt, ballPos[k][1] + ballVel[k][1]*dt, ballPos[k][2] + ballVel[k][2]*dt);
                newballVel = glm::vec3(ballVel[k][0]+ballAcc[k][0]*dt, ballVel[k][1]+ballAcc[k][1]*dt, ballVel[k][2]+ballAcc[k][2]*dt);
                newballAcc = glm::vec3(-d*ballVel[k][0]+d*windVel[0], (-m*g)+(-d*ballVel[k][1]), -d*ballVel[k][2]);
                
                glm::vec3 check_coll_center;
                glm::vec3 check_coll_normal;
                glm::vec3 check_coll_pos = newballPos;
                glm::vec3 coll_vel = newballVel;
                float check = 0;
                // check for collision w/all cube planes
                for(int i = 0; i < 6; i++){
                    //std::cout << fcube_centers[3*i] << ", " << fcube_centers[3*i+1] << ", " << fcube_centers[3*i+2] << "\n";
                    check_coll_center = glm::vec3(fcube_centers[3*i],fcube_centers[3*i+1],fcube_centers[3*i+2]);
                    check_coll_normal = glm::vec3(fcube_normals[3*i],fcube_normals[3*i+1],fcube_normals[3*i+2]);
                    check_coll_pos = newballPos;
                    check = glm::dot(glm::vec3(check_coll_pos - check_coll_center),check_coll_normal)-sphereRadius;
                    if (check<0) { //collision detected
                        //std::cout << "collision detected!\n";
                        coll_vel = glm::reflect(coll_vel, check_coll_normal);
                        //ballVel = coll_vel;
                        glm::vec3 velNormal = glm::vec3(glm::dot(coll_vel, check_coll_normal)*check_coll_normal);
                        glm::vec3 velTangent = glm::vec3(coll_vel-velNormal);
                        velNormal = glm::vec3(epsilon*velNormal); // coeff of restitution
                        velTangent = glm::vec3((1-tau)*velTangent); // friction
                        newballVel = glm::vec3(velNormal+velTangent);
                        //newballVel = glm::vec3(ballVel[0]+ballAcc[0]*dt, (ballVel[1]+ballAcc[1]*dt), ballVel[2]+ballAcc[2]*dt); //updated velocity
                        newballPos = glm::vec3(ballPos[k][0] + newballVel[0]*dt, ballPos[k][1] + newballVel[1]*dt, ballPos[k][2] + newballVel[2]*dt);
                    }
                }
                
                std::cout << "\ntime: " << simTime << "\ntimestep: " << timeStep << "\ntimestepsize: " << dt << "\nwind velocity: " << windVel[0] << "\n";
                
                ballPos[k] = newballPos;
                ballVel[k] = newballVel;
                ballAcc[k] = newballAcc;
                
                //check for collision w/other particles
                for(int i = 0; i<numberOfBalls; i++) {
                    if(i!=k) {
                        float distanceCheck = glm::length(glm::vec3(ballPos[k] - ballPos[i]));
                        if(distanceCheck<2*sphereRadius){   // ball collision occurred
                            glm::vec3 vel1 = ballVel[k];
                            glm::vec3 vel2 = ballVel[i];
                            glm::vec3 rVel1 = glm::reflect(vel1, glm::normalize(glm::vec3(vel2 - vel1)));
                            glm::vec3 rVel2 = glm::reflect(vel2, glm::normalize(glm::vec3(vel1 - vel2)));
                            glm::vec3 nVel1 = glm::vec3(glm::dot(rVel1, glm::normalize(glm::vec3(vel2 - vel1))*glm::normalize(glm::vec3(vel2 - vel1))));
                            glm::vec3 nVel2 = glm::vec3(glm::dot(rVel2, glm::normalize(glm::vec3(vel1 - vel2))*glm::normalize(glm::vec3(vel1 - vel2))));
                            glm::vec3 tVel1 = glm::vec3(rVel1 - nVel1);
                            glm::vec3 tVel2 = glm::vec3(rVel2 - nVel2);
                            ballVel[k] = glm::vec3((1-tau)*tVel1+epsilon*nVel1);
                            ballVel[i] = glm::vec3((1-tau)*tVel2+epsilon*nVel2);
                            ballPos[k] = glm::vec3(ballPos[k][0] + ballVel[k][0]*dt, ballPos[k][1] + ballVel[k][1]*dt, ballPos[k][2] + ballVel[k][2]*dt);
                            ballPos[i] = glm::vec3(ballPos[i][0] + ballVel[i][0]*dt, ballPos[i][1] + ballVel[i][1]*dt, ballPos[i][2] + ballVel[i][2]*dt);
                        }
                    }
                }
                
                
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
            }
        }
        // draw cube
        glGenBuffers(1, &vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(fcube_vertex), fcube_vertex, GL_STATIC_DRAW);
        
        glGenBuffers(1, &colorbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);
        
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
        
        // 2nd attribute buffer : colors
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
        glVertexAttribPointer(
                              1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
                              3,                                // size
                              GL_FLOAT,                         // type
                              GL_FALSE,                         // normalized?
                              0,                                // stride
                              (void*)0                          // array buffer offset
                              );
        
        glDrawArrays(GL_LINES, 0, 12*2);
        
        // draw cube centers
        glGenBuffers(1, &cvertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, cvertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(fcube_normallines), fcube_normallines, GL_STATIC_DRAW);
        
        glGenBuffers(1, &ccolorbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, ccolorbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);
        
        // 1st attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, cvertexbuffer);
        glVertexAttribPointer(
                              0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
                              3,                  // size
                              GL_FLOAT,           // type
                              GL_FALSE,           // normalized?
                              0,                  // stride
                              (void*)0            // array buffer offset
                              );
        
        // 2nd attribute buffer : colors
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, ccolorbuffer);
        glVertexAttribPointer(
                              1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
                              3,                                // size
                              GL_FLOAT,                         // type
                              GL_FALSE,                         // normalized?
                              0,                                // stride
                              (void*)0                          // array buffer offset
                              );
        
        glDrawArrays(GL_LINES, 0, 12);
        
        // draw sphere
        glGenBuffers(1, &svertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, svertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(fsphere_vertex), fsphere_vertex, GL_STATIC_DRAW);
        
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
