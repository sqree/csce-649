// Include standard headers
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <limits>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
GLFWwindow* window;
#include <glm/glm.hpp>
#include <common/shader.hpp>
#include <common/controls.hpp>
#include <common/Icosphere.h>
#include <common/objloader.hpp>
#include <common/texture.hpp>
#include <common/AF.hpp>

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
    window = glfwCreateWindow( 1024, 768, "HW4 - springy meshes", NULL, NULL);
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
    
    // Projection matrix : 45∞ Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 Projection = glm::perspective(glm::radians(90.0f), 4.0f / 3.0f, 0.1f, 100.0f);
    // Camera matrix
    glm::mat4 View       = glm::lookAt(
                                       glm::vec3(-5,0,0), // Camera is at (4,3,-3), in World Space
                                       glm::vec3(0,0,0), // and looks at the origin
                                       glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                                       );
    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model      = glm::mat4(1.0f);
    // Our ModelViewProjection : multiplication of our 3 matrices
    glm::mat4 MVP        = Projection * View * Model; // Remember, matrix multiplication is the other way around
    
    GLuint cvertexbuffer, vertexbuffer, colorbuffer, scolorbuffer, svertexbuffer;
    float sphereRadius = 0.03;
    Icosphere tSphere;
    tSphere.setRadius(sphereRadius);
    tSphere.setSubdivision(2);
    tSphere.setSmooth(false);
    float dt = 0.025;
    float epsilon = 0.7;
    float tau = 0.1;
    int numberOfBalls = 8;
    
    GLfloat g_vertex_buffer_data[] = {
        2.0f, 2.0f, 2.0f,
        -2.0f, 2.0f, 2.0f,
        -2.0f, 2.0f, 2.0f,
        -2.0f, 2.0f,-2.0f,
        -2.0f, 2.0f,-2.0f,
        2.0f, 2.0f,-2.0f,
        2.0f, 2.0f,-2.0f,
        2.0f, 2.0f, 2.0f,
        2.0f, -2.0f, 2.0f,
        -2.0f, -2.0f, 2.0f,
        -2.0f, -2.0f, 2.0f,
        -2.0f, -2.0f,-2.0f,
        -2.0f, -2.0f,-2.0f,
        2.0f, -2.0f,-2.0f,
        2.0f, -2.0f,-2.0f,
        2.0f, -2.0f, 2.0f,
        2.0f, 2.0f, 2.0f,
        2.0f, -2.0f, 2.0f,
        -2.0f, 2.0f, 2.0f,
        -2.0f, -2.0f, 2.0f,
        -2.0f, 2.0f,-2.0f,
        -2.0f, -2.0f,-2.0f,
        2.0f, 2.0f,-2.0f,
        2.0f, -2.0f,-2.0f
    };
    
    GLfloat cube_vertices[] = {
        2.0f, 2.0f, 2.0f,
        -2.0f, 2.0f, 2.0f,
        -2.0f, -2.0f, 2.0f,
        2.0f, 2.0f, 2.0f,
        -2.0f, -2.0f, 2.0f,
        2.0f, -2.0f, 2.0f,
        2.0f, 2.0f,-2.0f,
        2.0f, 2.0f, 2.0f,
        2.0f, -2.0f, 2.0f,
        2.0f, 2.0f,-2.0f,
        2.0f, -2.0f, 2.0f,
        2.0f, -2.0f, -2.0f,
        -2.0f, 2.0f, -2.0f,
        2.0f, 2.0f, -2.0f,
        2.0f, -2.0f, -2.0f,
        -2.0f, 2.0f, -2.0f,
        2.0f, -2.0f, -2.0f,
        -2.0f, -2.0f, -2.0f,
        -2.0f, 2.0f, 2.0f,
        -2.0f, 2.0f, -2.0f,
        -2.0f, -2.0f, -2.0f,
        -2.0f, 2.0f, 2.0f,
        -2.0f, -2.0f, -2.0f,
        -2.0f, -2.0f, 2.0f,
        2.0f, 2.0f, -2.0f,
        -2.0f, 2.0f, -2.0f,
        -2.0f, 2.0f, 2.0f,
        2.0f, 2.0f, -2.0f,
        -2.0f, 2.0f, 2.0f,
        2.0f, 2.0f, 2.0f,
        2.0f, -2.0f, 2.0f,
        -2.0f, -2.0f, 2.0f,
        -2.0f, -2.0f, -2.0f,
        2.0f, -2.0f, 2.0f,
        -2.0f, -2.0f, -2.0f,
        2.0f, -2.0f, -2.0f
    };
    
    GLfloat test_vertices[] = {
        -0.1f,-0.1f,0.1f,
        -0.1f,-0.1f,-0.1f,
        0.1f,-0.1f,-0.1f,
        0.1f,-0.1f,0.1f,
        -0.1f,0.1f,0.1f,
        -0.1f,0.1f,-0.1f,
        0.1f,0.1f,-0.1f,
        0.1f,0.1f,0.1f
    };
    
    std::string mode = "default";
    struts tempS;
    std::vector <struts> test;
    vertexMass tempVM;
    std::vector <vertexMass> test1, K1, K2, K3, K4;
    faces face1;
    std::vector <faces> cubeFaces;
    object cube1;
    std::vector <object> cube;
    std::vector <object> oldCube;
    std::vector <std::vector<glm::vec3> > faceCheck;
    std::vector <std::vector<glm::vec3> > edgeCheck;
    std::vector <std::vector<glm::vec3> > face2Check;
    GLfloat lSpring;
    GLfloat lScale = 3.5f;
    GLfloat kSpring = 1.0f;
    GLfloat dSpring = 0.3f;
    
    // for each cube
    for(int k = 0; k < 2; k++) {
        // create cube data structures
        for (int i = 0; i < sizeof(test_vertices)/4/3; i++) {
            tempVM.vertexForce = glm::vec3(0,0,0);
            tempVM.vertexVel = glm::vec3(0,0,0);
            tempVM.vertexPos = glm::vec3(test_vertices[3*i]+0.5f,test_vertices[3*i+1]+1.5f*k,test_vertices[3*i+2]+2.0f*(k-0.5));
            test1.push_back(tempVM);
        }
        // torsional springs version
        for (int i = 0; i<4; i++) {
            //top
            lSpring = glm::length(test1[i].vertexPos - test1[(i+1)%4].vertexPos);
            tempS.parameters = glm::vec3(kSpring,dSpring,lSpring*lScale);
            tempS.vertex_indices = glm::vec2(i,(i+1)%4);
            tempS.type = "cross";
            test.push_back(tempS);
            //mid
            tempS.vertex_indices = glm::vec2(i,i+4);
            test.push_back(tempS);
            //bot
            tempS.vertex_indices = glm::vec2(i+4,(i+4+1)%4+4);
            test.push_back(tempS);
            //diag -- > torsional
            lSpring = glm::length(test1[i].vertexPos - test1[(i+1)%4 + 4].vertexPos);
            tempS.parameters = glm::vec3(kSpring,dSpring,lSpring*lScale);
            tempS.vertex_indices = glm::vec2(i,(i+1)%4 + 4);
            tempS.type = "torsional";
            test.push_back(tempS);
        }
        //top, bottom diag springs --> torsional
        lSpring = glm::length(test1[3].vertexPos - test1[1].vertexPos);
        tempS.parameters = glm::vec3(kSpring,dSpring,lSpring*lScale);
        tempS.vertex_indices = glm::vec2(3,1);
        tempS.type = "cross";
        test.push_back(tempS);
        
        tempS.vertex_indices = glm::vec2(4,6);
        test.push_back(tempS);
        
        //side faces
        int faceCount = 0;
        for(int i = 0; i < 4; i++) {
            face1.strut_indices = glm::vec3(4*i+1, 4*i+2, 4*i+3);
            cubeFaces.push_back(face1);
            test[4*i+1].face_indices[0] = test[4*i+2].face_indices[0] = test[4*i+3].face_indices[0] = faceCount;
            faceCount++;
            face1.strut_indices = glm::vec3(4*i+3, 4*i, (4*i+5)%16);
            cubeFaces.push_back(face1);
            test[4*i].face_indices[1] = test[4*i+3].face_indices[1] = test[(4*i+5)%16].face_indices[1] = faceCount;
            faceCount++;
        }
        //top & bottom faces
        face1.strut_indices = glm::vec3(12, 0, 16);
        cubeFaces.push_back(face1);
        test[12].face_indices[1] = test[0].face_indices[1] = test[16].face_indices[1] = faceCount;
        faceCount++;
        face1.strut_indices = glm::vec3(8, 16, 4);
        cubeFaces.push_back(face1);
        test[8].face_indices[1] = test[16].face_indices[1] = test[4].face_indices[1] = faceCount;
        faceCount++;
        face1.strut_indices = glm::vec3(14, 10, 17);
        cubeFaces.push_back(face1);
        test[14].face_indices[1] = test[10].face_indices[1] = test[17].face_indices[1] = faceCount;
        faceCount++;
        face1.strut_indices = glm::vec3(6, 17, 2);
        cubeFaces.push_back(face1);
        test[2].face_indices[1] = test[17].face_indices[1] = test[6].face_indices[1] = faceCount;
        faceCount++;
        
        cube1.objSprings = test;
        cube1.objVertices = test1;
        cube1.objFaces = cubeFaces;
        cube.push_back(cube1);
        test.clear();
        test1.clear();
        cubeFaces.clear();
    }
    // create collsion check variables
    std::vector <glm::vec3> tempCheck;
    for(int i=0; i < cube.size(); i++) {
        for (int j=0; j < cube[i].objVertices.size(); j++) {
            glm::vec3 tempFaceCheck = glm::vec3(0.0f, 0.0f, 0.0f);
            tempCheck.push_back(tempFaceCheck);
        }
        faceCheck.push_back(tempCheck);
        face2Check.push_back(tempCheck);
    }
    tempCheck.clear();
    for(int i=0; i < cube.size(); i++) {
        for (int j=0; j < cube[i].objSprings.size(); j++) {
            glm::vec3 tempEdgeCheck = glm::vec3(0.0f, 0.0f, 0.0f);
            tempCheck.push_back(tempEdgeCheck);
        }
        edgeCheck.push_back(tempCheck);
    }
    tempCheck.clear();
    
    GLfloat fsphere_vertex[3*tSphere.getVertexCount()*numberOfBalls*cube.size()], sphere_color[3*tSphere.getVertexCount()*numberOfBalls*cube.size()], test2[3*2*cube[0].objSprings.size()*cube.size()], test2Color[3*2*cube[0].objSprings.size()*cube.size()], cubeface_vertices[3*3*12*cube.size()], cubeface_color[3*3*12*cube.size()], cs_holder[3*2*10];
    glm::mat4 rotationMatrix(1);
    bool rotateCube = true;
    glm::vec3 cubeVertex;
    bool init = false;
    int numberOfCubes = 2;
    
    do{
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Use our shader
        glUseProgram(programID);
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        
        // cube rotation interface
        if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
            rotationMatrix = glm::rotate(rotationMatrix, -0.01f, glm::vec3(0.0, 1.0, 0.0));
            rotateCube = true;
        }
        if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS){
            rotationMatrix = glm::rotate(rotationMatrix, 0.01f, glm::vec3(0.0, 1.0, 0.0));
            rotateCube = true;
        }
        if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS){
            rotationMatrix = glm::rotate(rotationMatrix, 0.01f, glm::vec3(0.0, 0.0, 1.0));
            rotateCube = true;
        }
        if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS){
            rotationMatrix = glm::rotate(rotationMatrix, -0.01f, glm::vec3(0.0, 0.0, 1.0));
            rotateCube = true;
        }
        if(rotateCube) {
            for(int i=0; i<sizeof(cube_vertices)/3/4; i++) {
                cubeVertex = glm::vec3(cube_vertices[3*i],cube_vertices[3*i+1],cube_vertices[3*i+2]);
                cubeVertex = glm::vec3(rotationMatrix * glm::vec4(cubeVertex,1.0));
                cube_vertices[3*i] = cubeVertex[0];
                cube_vertices[3*i+1] = cubeVertex[1];
                cube_vertices[3*i+2] = cubeVertex[2];
            }
            // also rotate displayed outer cube
            for(int i=0; i<sizeof(g_vertex_buffer_data)/3/4; i++) {
                cubeVertex = glm::vec3(g_vertex_buffer_data[3*i],g_vertex_buffer_data[3*i+1],g_vertex_buffer_data[3*i+2]);
                cubeVertex = glm::vec3(rotationMatrix * glm::vec4(cubeVertex,1.0));
                g_vertex_buffer_data[3*i] = cubeVertex[0];
                g_vertex_buffer_data[3*i+1] = cubeVertex[1];
                g_vertex_buffer_data[3*i+2] = cubeVertex[2];
            }
            rotateCube = false;
        }
        
        if (glfwGetKey( window, GLFW_KEY_R ) == GLFW_PRESS) {
            cube.clear();
            mode = "default";
            init = true;
            numberOfCubes = 2;
            numberOfBalls = 8;
            epsilon = 0.7f;
        }
        if (glfwGetKey( window, GLFW_KEY_N ) == GLFW_PRESS) {
            cube.clear();
            mode = "Euler";
            init = true;
        }
        if (glfwGetKey( window, GLFW_KEY_C ) == GLFW_PRESS) {
            cube.clear();
            mode = "Cross";
            init = true;
        }
        if (glfwGetKey( window, GLFW_KEY_F ) == GLFW_PRESS) {
            cube.clear();
            mode = "Face";
            init = true;
        }
        if (glfwGetKey( window, GLFW_KEY_E ) == GLFW_PRESS) {
            cube.clear();
            mode = "Edge";
            init = true;
        }
        if (glfwGetKey( window, GLFW_KEY_P ) == GLFW_PRESS) {
            cube.clear();
            mode = "CrossP";
            init = true;
            epsilon = 1.0;
            numberOfCubes = 1;
            numberOfBalls = 8;
        }
        
        // set mouse ctrl callback
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        
        glm::vec3 newballPos, newballVel, newballAcc;
        
        if(init) {
            for(int k = 0; k < numberOfCubes; k++) {
                GLfloat lScale;
                GLfloat lSpace;
                GLfloat lHeight;
                GLfloat crossL;
                GLfloat crossD;
                
                if(mode.compare("Edge")==0) {
                    lScale = 5.0f-k*2.0f;
                    lSpace = 0.7f;
                    lHeight = 1.5f*k;
                }
                else if (mode.compare("Face")==0) {
                    lScale = 5.0f-k*2.0f;
                    lSpace = 0.15f;
                    lHeight = 1.5f*k;
                }
                else {
                    lScale = 3.5f;
                    lSpace = 2.5f;
                    lHeight = 1.5f;
                    crossL = 1.0f;
                    crossD = 0.3f;
                    if(mode.compare("CrossP")==0) {
                        crossL = 1.0f;
                        crossD = 0.1f;
                    }
                }
                // create cube data structures
                for (int i = 0; i < sizeof(test_vertices)/4/3; i++) {
                    tempVM.vertexForce = glm::vec3(0,0,0);
                    tempVM.vertexVel = glm::vec3(0,0,0);
                    tempVM.vertexPos = glm::vec3(test_vertices[3*i]+0.5f,test_vertices[3*i+1]+lHeight,test_vertices[3*i+2]+lSpace*(k-0.5));
                    test1.push_back(tempVM);
                }
                // cross springs version
                if(k==0 && (mode.compare("Cross")==0||mode.compare("CrossP")==0)) {
                    for (int i = 0; i < sizeof(test_vertices)/4/3; i++) {
                        for (int j = i+1; j < sizeof(test_vertices)/4/3; j++) {
                            //create strut
                            lSpring = glm::length(test1[i].vertexPos - test1[j].vertexPos);
                            tempS.parameters = glm::vec3(crossL,crossD,lSpring*lScale);
                            tempS.vertex_indices = glm::vec2(i,j);
                            tempS.type = "cross";
                            test.push_back(tempS);
                        }
                    }
                }
                else {
                    // torsional springs version
                    for (int i = 0; i<4; i++) {
                        //top
                        lSpring = glm::length(test1[i].vertexPos - test1[(i+1)%4].vertexPos);
                        tempS.parameters = glm::vec3(kSpring,dSpring,lSpring*lScale);
                        tempS.vertex_indices = glm::vec2(i,(i+1)%4);
                        tempS.type = "cross";
                        test.push_back(tempS);
                        //mid
                        tempS.vertex_indices = glm::vec2(i,i+4);
                        test.push_back(tempS);
                        //bot
                        tempS.vertex_indices = glm::vec2(i+4,(i+4+1)%4+4);
                        test.push_back(tempS);
                        //diag -- > torsional
                        lSpring = glm::length(test1[i].vertexPos - test1[(i+1)%4 + 4].vertexPos);
                        tempS.parameters = glm::vec3(kSpring,dSpring,lSpring*lScale);
                        tempS.vertex_indices = glm::vec2(i,(i+1)%4 + 4);
                        tempS.type = "torsional";
                        test.push_back(tempS);
                    }
                    //top, bottom diag springs --> torsional
                    lSpring = glm::length(test1[3].vertexPos - test1[1].vertexPos);
                    tempS.parameters = glm::vec3(kSpring,dSpring,lSpring*lScale);
                    tempS.vertex_indices = glm::vec2(3,1);
                    tempS.type = "cross";
                    test.push_back(tempS);
                    
                    tempS.vertex_indices = glm::vec2(4,6);
                    test.push_back(tempS);
                }
                //side faces
                int faceCount = 0;
                for(int i = 0; i < 4; i++) {
                    face1.strut_indices = glm::vec3(4*i+1, 4*i+2, 4*i+3);
                    cubeFaces.push_back(face1);
                    test[4*i+1].face_indices[0] = test[4*i+2].face_indices[0] = test[4*i+3].face_indices[0] = faceCount;
                    faceCount++;
                    face1.strut_indices = glm::vec3(4*i+3, 4*i, (4*i+5)%16);
                    cubeFaces.push_back(face1);
                    test[4*i].face_indices[1] = test[4*i+3].face_indices[1] = test[(4*i+5)%16].face_indices[1] = faceCount;
                    faceCount++;
                }
                //top & bottom faces
                face1.strut_indices = glm::vec3(12, 0, 16);
                cubeFaces.push_back(face1);
                test[12].face_indices[1] = test[0].face_indices[1] = test[16].face_indices[1] = faceCount;
                faceCount++;
                face1.strut_indices = glm::vec3(8, 16, 4);
                cubeFaces.push_back(face1);
                test[8].face_indices[1] = test[16].face_indices[1] = test[4].face_indices[1] = faceCount;
                faceCount++;
                face1.strut_indices = glm::vec3(14, 10, 17);
                cubeFaces.push_back(face1);
                test[14].face_indices[1] = test[10].face_indices[1] = test[17].face_indices[1] = faceCount;
                faceCount++;
                face1.strut_indices = glm::vec3(6, 17, 2);
                cubeFaces.push_back(face1);
                test[2].face_indices[1] = test[17].face_indices[1] = test[6].face_indices[1] = faceCount;
                faceCount++;
                
                cube1.objSprings = test;
                cube1.objVertices = test1;
                cube1.objFaces = cubeFaces;
                cube.push_back(cube1);
                test.clear();
                test1.clear();
                cubeFaces.clear();
            }
            init = false;
        }
        
        int count = 0;
        while(count<0.15/dt) {
            oldCube = cube;
            // for each cube
            for(int k = 0; k<cube.size(); k++) {
                // calculate K-states for RK integration; inputs: (springs, vertices, K-state, time step)
                K1 = deriveState(cube[k].objSprings, cube[k].objVertices, cube[k].objFaces, cube[k].objVertices, 0.0f);
                K2 = deriveState(cube[k].objSprings, cube[k].objVertices, cube[k].objFaces, K1, 0.5*dt);
                K3 = deriveState(cube[k].objSprings, cube[k].objVertices, cube[k].objFaces, K2, 0.5*dt);
                K4 = deriveState(cube[k].objSprings, cube[k].objVertices, cube[k].objFaces, K3, dt);
                
                // calculate solution for each vertex
                for (int i=0; i < cube[k].objVertices.size(); i++) {
                    if(k==1 && mode.compare("Euler")==0) {
                        //Euler
                        newballPos = cube[k].objVertices[i].vertexPos + K1[i].vertexVel*dt;
                        newballVel = cube[k].objVertices[i].vertexVel + K1[i].vertexForce*dt;
                    }
                    else {
                        // Rk4
                        newballPos = cube[k].objVertices[i].vertexPos + dt/6.0f*(K1[i].vertexVel+2.0f*K2[i].vertexVel+2.0f*K3[i].vertexVel+K4[i].vertexVel);
                        newballVel = cube[k].objVertices[i].vertexVel + dt/6.0f*(K1[i].vertexForce+2.0f*K2[i].vertexForce+2.0f*K3[i].vertexForce+K4[i].vertexForce);
                    }
                    //check for collisions against environment
                    glm::vec3 check_coll_center;
                    glm::vec3 check_coll_normal;
                    glm::vec3 check_coll_pos = newballPos;
                    glm::vec3 coll_vel = newballVel;
                    for(int j=0; j<sizeof(cube_vertices)/3/3/4; j++) {
                        glm::vec3 p0 = glm::vec3(cube_vertices[9*j],cube_vertices[9*j+1],cube_vertices[9*j+2]);
                        glm::vec3 p1 = glm::vec3(cube_vertices[9*j+3],cube_vertices[9*j+4],cube_vertices[9*j+5]);
                        glm::vec3 p2 = glm::vec3(cube_vertices[9*j+6],cube_vertices[9*j+7],cube_vertices[9*j+8]);
                        check_coll_normal = triNormal(p0, p1, p2);
                        float hitPoint = intersect(coll_vel, check_coll_pos, p0, p1, p2);
                        float coll_check = glm::dot(face2Check[k][i],-coll_vel*hitPoint);
                        if(hitPoint<std::numeric_limits<float>::max()) {
                            face2Check[k][i] = -coll_vel*hitPoint;
                        }
                        if(glm::length(glm::vec3(coll_vel*hitPoint))<sphereRadius && coll_check<0) { //collision w/triangle
                            newballVel = collisionVel(coll_vel,check_coll_normal,epsilon,tau);
                            newballPos = cube[k].objVertices[i].vertexPos + newballVel*dt;
                        }
                    }
                    cube[k].objVertices[i].vertexPos = newballPos;
                    cube[k].objVertices[i].vertexVel = newballVel;
                }
            }
            for(int k = 0; k<cube.size(); k++) {
                // check face-vertex collisions w/other objects
                for (int i=0; i < cube[k].objVertices.size(); i++) {
                    glm::vec3 check_coll_pos = cube[k].objVertices[i].vertexPos;
                    glm::vec3 coll_vel = cube[k].objVertices[i].vertexVel;
                    glm::vec3 check_coll_normal;
                    std::vector <int> x;
                    int tempX;
                    //bool match = false;
                    for(int z=0; z<cube.size(); z++) {
                        // if not current object
                        if(z!=k) {
                            //check against each face of other cube
                            for(int j=0; j<cube[z].objFaces.size(); j++){
                                //get each associated face vertex
                                tempX = cube[z].objSprings[cube[z].objFaces[j].strut_indices[0]].vertex_indices[0];
                                x.push_back(tempX);
                                tempX = cube[z].objSprings[cube[z].objFaces[j].strut_indices[0]].vertex_indices[1];
                                x.push_back(tempX);
                                while(x.size()<3) {
                                    //for each face strut
                                    for(int m=0; m<3; m++) {
                                        // for each vertec
                                        for(int n=0; n<2; n++) {
                                            tempX = cube[z].objSprings[cube[z].objFaces[j].strut_indices[m]].vertex_indices[n];
                                            if(tempX!=x[0] && tempX!=x[1]) {
                                                x.push_back(tempX);
                                            }
                                        }
                                    }
                                }
                                glm::vec3 p0 = cube[z].objVertices[x[0]].vertexPos;
                                glm::vec3 p1 = cube[z].objVertices[x[1]].vertexPos;
                                glm::vec3 p2 = cube[z].objVertices[x[2]].vertexPos;
                                glm::vec3 p0Vel = cube[z].objVertices[x[0]].vertexVel;
                                glm::vec3 p1Vel = cube[z].objVertices[x[1]].vertexVel;
                                glm::vec3 p2Vel = cube[z].objVertices[x[2]].vertexVel;
                                check_coll_normal = triNormal(p0, p1, p2);
                                float hitPoint = intersect(coll_vel, check_coll_pos, p0, p1, p2);
                                //check sign
                                float coll_check = glm::dot(faceCheck[k][i],-coll_vel*hitPoint);
                                // update face check vector if valid hitPoint returned
                                if(hitPoint<std::numeric_limits<float>::max()) {
                                    faceCheck[k][i] = -coll_vel*hitPoint;
                                }
                                if(glm::length(-coll_vel*hitPoint)<0.05f && coll_check<0) { //within area of interest
                                    //std::cout << "hit\n";
                                    // calculate position of hitpoint
                                    glm::vec3 hitPos = check_coll_pos+hitPoint*coll_vel;
                                    // determine barycentric weights
                                    glm::vec3 uvw = barycentric3(hitPos, p0, p1, p2);
                                    // mass of vertex is 1, need to calculate effective mass/velocity of face point
                                    GLfloat mp = 1.0f;
                                    glm::vec3 vp = coll_vel;
                                    GLfloat mq = (uvw[0]+uvw[1]+uvw[2])/(uvw[0]*uvw[0]+uvw[1]*uvw[1]+uvw[2]*uvw[2]);
                                    glm::vec3 vq = uvw[0]*p0Vel + uvw[1]*p1Vel + uvw[2]*p2Vel;
                                    // calculate center of momentum
                                    glm::vec3 cm = (mp*vp + mq*vq)/(mp+mq);
                                    vp = vp-cm;
                                    vq = vq-cm;
                                    vp = collisionVel(vp, check_coll_normal, epsilon, tau);
                                    vq = collisionVel(vq, check_coll_normal, epsilon, tau);
                                    vp = vp + cm;
                                    vq = vq + cm;
                                    
                                    // update current cube
                                    cube[k].objVertices[i].vertexVel = vp;
                                    cube[k].objVertices[i].vertexPos = oldCube[k].objVertices[i].vertexPos+cube[k].objVertices[i].vertexVel*dt;
                                    oldCube[k].objVertices[i] = cube[k].objVertices[i];
                                    
                                    // update collided face vertices
                                    for(int m=0; m<3; m++) {
                                        cube[z].objVertices[x[m]].vertexVel = uvw[m]*vq;
                                        cube[z].objVertices[x[m]].vertexPos = oldCube[z].objVertices[x[m]].vertexPos + cube[z].objVertices[x[m]].vertexVel*dt;
                                        oldCube[z].objVertices[x[m]] = cube[z].objVertices[x[m]];
                                    }
                                }
                                x.clear();
                            }
                        }
                    }
                }
                //check for edge-edge
                for(int i = 0; i < cube[k].objSprings.size(); i++) {
                    GLuint p1 = cube[k].objSprings[i].vertex_indices[0];
                    GLuint p2 = cube[k].objSprings[i].vertex_indices[1];
                    glm::vec3 a = (cube[k].objVertices[p2].vertexPos - cube[k].objVertices[p1].vertexPos);
                    glm::vec3 an = a/glm::length(a);
                    for(int j = 0; j<cube.size(); j++) {
                        if(j!=k) {
                            for(int m = 0; m < cube[j].objSprings.size(); m++) {
                                GLuint q1 = cube[j].objSprings[m].vertex_indices[0];
                                GLuint q2 = cube[j].objSprings[m].vertex_indices[1];
                                glm::vec3 b = cube[j].objVertices[q2].vertexPos - cube[j].objVertices[q1].vertexPos;
                                glm::vec3 bn = b/glm::length(b);
                                glm::vec3 edgeN = glm::cross(a,b)/glm::length(glm::cross(a,b));
                                glm::vec3 r = (cube[j].objVertices[q1].vertexPos - cube[k].objVertices[p1].vertexPos);
                                GLfloat s = glm::dot(r, glm::cross(bn, edgeN))/glm::dot(a,glm::cross(bn, edgeN));
                                GLfloat t = glm::dot(-r, glm::cross(an, edgeN))/glm::dot(b,glm::cross(an, edgeN));
                                glm::vec3 pa = cube[k].objVertices[p1].vertexPos + s*a;
                                glm::vec3 qa = cube[j].objVertices[q1].vertexPos + t*b;
                                
                                // check for valid s/t
                                if(s>0 && s<1 && t>0 && t<1) {
                                    float coll_check = glm::dot(edgeCheck[k][i],qa - pa);
                                    edgeCheck[k][i] = qa-pa;
                                    if(glm::length(pa - qa)<0.01f && coll_check<0) {
                                        // edge edge collision occured
                                        GLfloat pv = glm::length(s*a)/glm::length(a);
                                        GLfloat pu = 1-pv;
                                        GLfloat qv = glm::length(t*b)/glm::length(b);
                                        GLfloat qu = 1-qv;
                                        GLfloat mp = (pu + pv)/(pu*pu+pv*pv);
                                        glm::vec3 vp = pu*cube[k].objVertices[p1].vertexVel + pv*cube[k].objVertices[p2].vertexVel;
                                        GLfloat mq = (qu + qv)/(qu*qu+qv*qv);
                                        glm::vec3 vq = qu*cube[j].objVertices[q1].vertexVel + qv*cube[j].objVertices[q2].vertexVel;
                                        // calculate center of momentum
                                        glm::vec3 cm = (mp*vp + mq*vq)/(mp+mq);
                                        
                                        vp = vp-cm;
                                        vq = vq-cm;
                                        vp = collisionVel(vp, edgeN, epsilon, tau);
                                        vq = collisionVel(vq, edgeN, epsilon, tau);
                                        vp = vp + cm;
                                        vq = vq + cm;
                                        
                                        cube[k].objVertices[p1].vertexVel = pu*vp;
                                        cube[k].objVertices[p2].vertexVel = pv*vp;
                                        cube[j].objVertices[q1].vertexVel = qu*vq;
                                        cube[j].objVertices[q2].vertexVel = qv*vq;
                                        
                                        cube[k].objVertices[p1].vertexPos = oldCube[k].objVertices[p1].vertexPos + cube[k].objVertices[p1].vertexVel*dt;
                                        cube[k].objVertices[p2].vertexPos = oldCube[k].objVertices[p2].vertexPos + cube[k].objVertices[p2].vertexVel*dt;
                                        cube[j].objVertices[q1].vertexPos = oldCube[j].objVertices[q1].vertexPos + cube[j].objVertices[q1].vertexVel*dt;
                                        cube[j].objVertices[q2].vertexPos = oldCube[j].objVertices[q2].vertexPos + cube[j].objVertices[q2].vertexVel*dt;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            count++;
        }
        
        for(int k = 0; k<cube.size(); k++) {
            // for each edge strut (line)
            for(int i = 0; i < cube[k].objSprings.size(); i++) {
                for (int j=0; j<2; j++) {
                    test2[3*j+6*i + k*cube[k].objSprings.size()*6] = cube[k].objVertices[cube[k].objSprings[i].vertex_indices[j]].vertexPos[0];
                    test2[3*j+1+6*i + k*cube[k].objSprings.size()*6] = cube[k].objVertices[cube[k].objSprings[i].vertex_indices[j]].vertexPos[1];
                    test2[3*j+2+6*i + k*cube[k].objSprings.size()*6] = cube[k].objVertices[cube[k].objSprings[i].vertex_indices[j]].vertexPos[2];
                    test2Color[3*j+6*i + k*cube[k].objSprings.size()*6] = test2Color[3*j+1+6*i + k*cube[k].objSprings.size()*6] = test2Color[3*j+2+6*i + k*cube[k].objSprings.size()*6] = 0.0f;
                }
            }
            if(mode.compare("Cross")==0 && k==0) {
                for(int i = 18; i < cube[k].objSprings.size(); i++) {
                    for (int j=0; j<2; j++) {
                        cs_holder[3*j + 6*(i-18)] = cube[k].objVertices[cube[k].objSprings[i].vertex_indices[j]].vertexPos[0];
                        cs_holder[3*j+1 + 6*(i-18)] = cube[k].objVertices[cube[k].objSprings[i].vertex_indices[j]].vertexPos[1];
                        cs_holder[3*j+2 + 6*(i-18)] = cube[k].objVertices[cube[k].objSprings[i].vertex_indices[j]].vertexPos[2];
                    }
                }
            }
            //for each face
            for(int i=0; i<cube[k].objFaces.size(); i++){
                //get each associated face vertex
                std::vector <int> x;
                int tempX;
                tempX = cube[k].objSprings[cube[k].objFaces[i].strut_indices[0]].vertex_indices[0];
                x.push_back(tempX);
                tempX = cube[k].objSprings[cube[k].objFaces[i].strut_indices[0]].vertex_indices[1];
                x.push_back(tempX);
                while(x.size()<3) {
                    //for each face strut
                    for(int m=0; m<3; m++) {
                        // for each vertec
                        for(int n=0; n<2; n++) {
                            tempX = cube[k].objSprings[cube[k].objFaces[i].strut_indices[m]].vertex_indices[n];
                            if(tempX!=x[0] && tempX!=x[1]) {
                                x.push_back(tempX);
                            }
                        }
                    }
                }
                glm::vec3 p0 = cube[k].objVertices[x[0]].vertexPos;
                glm::vec3 p1 = cube[k].objVertices[x[1]].vertexPos;
                glm::vec3 p2 = cube[k].objVertices[x[2]].vertexPos;
                cubeface_vertices[9*i + 12*3*3*k] = p0[0];
                cubeface_vertices[9*i + 12*3*3*k + 1] = p0[1];
                cubeface_vertices[9*i + 12*3*3*k + 2] = p0[2];
                cubeface_vertices[9*i + 12*3*3*k + 3] = p1[0];
                cubeface_vertices[9*i + 12*3*3*k + 4] = p1[1];
                cubeface_vertices[9*i + 12*3*3*k + 5] = p1[2];
                cubeface_vertices[9*i + 12*3*3*k + 6] = p2[0];
                cubeface_vertices[9*i + 12*3*3*k + 7] = p2[1];
                cubeface_vertices[9*i + 12*3*3*k + 8] = p2[2];
                cubeface_color[9*i + 12*3*3*k] = cubeface_color[9*i + 12*3*3*k + 3] = cubeface_color[9*i + 12*3*3*k + 6] = 0.0f;
                cubeface_color[9*i + 12*3*3*k + 1] = cubeface_color[9*i + 12*3*3*k + 4] = cubeface_color[9*i + 12*3*3*k + 7] = 0.0f;
                cubeface_color[9*i + 12*3*3*k + 2] = cubeface_color[9*i + 12*3*3*k + 5] = cubeface_color[9*i + 12*3*3*k + 8] = 0.05f*i+0.3;
            }
            //for each vertex (ball)
            for (int i=0; i < cube[k].objVertices.size(); i++) {
                float const* p = tSphere.getVertices();
                for (int j = 0; j<tSphere.getVertexCount(); j++) {
                    fsphere_vertex[3*j + i*tSphere.getVertexCount()*3 + k*cube[k].objVertices.size()*tSphere.getVertexCount()*3] = *p + cube[k].objVertices[i].vertexPos[0];
                    p++;
                    fsphere_vertex[3*j+1 + i*tSphere.getVertexCount()*3 + k*cube[k].objVertices.size()*tSphere.getVertexCount()*3] = *p + cube[k].objVertices[i].vertexPos[1];
                    p++;
                    fsphere_vertex[3*j+2 + i*tSphere.getVertexCount()*3 + k*cube[k].objVertices.size()*tSphere.getVertexCount()*3] = *p + cube[k].objVertices[i].vertexPos[2];
                    p++;
                    sphere_color[3*j + i*tSphere.getVertexCount()*3 + k*cube[k].objVertices.size()*tSphere.getVertexCount()*3] = 0.9f;
                    sphere_color[3*j+1 + i*tSphere.getVertexCount()*3 + k*cube[k].objVertices.size()*tSphere.getVertexCount()*3] = 0.4f;
                    sphere_color[3*j+2 + i*tSphere.getVertexCount()*3 + k*cube[k].objVertices.size()*tSphere.getVertexCount()*3] = 0.3f;
                }
            }
        }
        
        // draw outer cube
        glGenBuffers(1, &cvertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, cvertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, cvertexbuffer);
        glVertexAttribPointer(
                              0,
                              3,
                              GL_FLOAT,
                              GL_FALSE,
                              0,
                              (void*)0
                              );
        
        glDrawArrays(GL_LINES,0,sizeof(g_vertex_buffer_data));
        
        // draw springy cube vertices
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
        
        glDrawArrays(GL_TRIANGLES,0,tSphere.getVertexCount()*cube.size()*numberOfBalls);
        
        // draw springs
        glGenBuffers(1, &svertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, svertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(test2), test2, GL_STATIC_DRAW);
        
        glGenBuffers(1, &scolorbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, scolorbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(test2Color), test2Color, GL_STATIC_DRAW);
        
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
        
        glDrawArrays(GL_LINES,0,2*cube[0].objSprings.size()*cube.size());
        
        if(mode.compare("Cross")==0) {
            glGenBuffers(1, &svertexbuffer);
            glBindBuffer(GL_ARRAY_BUFFER, svertexbuffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(cs_holder), cs_holder, GL_STATIC_DRAW);
            
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
            
            glDrawArrays(GL_LINES,0,2*10);
        }
        
        if(mode.compare("Cross")!=0 && mode.compare("CrossP")!= 0) {
            glGenBuffers(1, &svertexbuffer);
            glBindBuffer(GL_ARRAY_BUFFER, svertexbuffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(cubeface_vertices), cubeface_vertices, GL_STATIC_DRAW);
            
            glGenBuffers(1, &scolorbuffer);
            glBindBuffer(GL_ARRAY_BUFFER, scolorbuffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(cubeface_color), cubeface_color, GL_STATIC_DRAW);
            
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
            
            glDrawArrays(GL_TRIANGLES,0,3*3*cube[0].objFaces.size()*cube.size());
        }
        
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
