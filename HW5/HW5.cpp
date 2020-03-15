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
    window = glfwCreateWindow( 1024, 768, "HW4", NULL, NULL);
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
                                       glm::vec3(-25,0,0), // Camera is at (4,3,-3), in World Space
                                       glm::vec3(0,0,0), // and looks at the origin
                                       glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                                       );
    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model      = glm::mat4(1.0f);
    // Our ModelViewProjection : multiplication of our 3 matrices
    glm::mat4 MVP        = Projection * View * Model; // Remember, matrix multiplication is the other way around
    
    GLuint vertexbuffer, colorbuffer, scolorbuffer, svertexbuffer, hcolorbuffer, hvertexbuffer, vvertexbuffer;
    float sphereRadius = 0.5f;
    Icosphere tSphere;
    tSphere.setRadius(sphereRadius);
    tSphere.setSubdivision(2);
    tSphere.setSmooth(false);
    float dt = 0.015;
    float epsilon = 0.7;
    float tau = 0.1;
    
    //set up obj reader
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals; // Won't be used at the moment.
    bool res = loadOBJ("cube.obj", vertices, uvs, normals);
    
    glm::vec3 cubeVertex;
    GLfloat cube_vertices[vertices.size()*3];
    std::vector <glm::vec3> trimmed_cube_vertices;
    std::vector <glm::vec3> trimmed_cube_edges;
    
    for(int i=0; i<vertices.size(); i++) {
        cubeVertex = 2.0f*vertices[i];
        cube_vertices[3*i] = cubeVertex[0];
        cube_vertices[3*i+1] = cubeVertex[1];
        cube_vertices[3*i+2] = cubeVertex[2];
        trimmed_cube_vertices.push_back(cubeVertex);
        
        if(i>1) {
            bool match = false;
            for(int j = 0; j<trimmed_cube_vertices.size()-1; j++) {
                if(trimmed_cube_vertices[j][0]==cube_vertices[3*i] && trimmed_cube_vertices[j][1]==cube_vertices[3*i+1] && trimmed_cube_vertices[j][2]==cube_vertices[3*i+2]) {
                    match = true;
                }
            }
            if(match) {
                trimmed_cube_vertices.pop_back();
            }
        }
    }
    for(int i=0; i<vertices.size(); i+=3) {
        glm::vec3 p0 = glm::vec3(2.0f*vertices[i][0],2.0f*vertices[i][1],2.0f*vertices[i][2]);
        glm::vec3 p1 = glm::vec3(2.0f*vertices[i+1][0],2.0f*vertices[i+1][1],2.0f*vertices[i+1][2]);
        glm::vec3 p2 = glm::vec3(2.0f*vertices[i+2][0],2.0f*vertices[i+2][1],2.0f*vertices[i+2][2]);
        
        trimmed_cube_edges.push_back(p0);
        trimmed_cube_edges.push_back(p1);
        trimmed_cube_edges.push_back(p1);
        trimmed_cube_edges.push_back(p2);
        trimmed_cube_edges.push_back(p2);
        trimmed_cube_edges.push_back(p0);
    }
    
    std::vector <int> trimIndex;
    for(int i = 0; i<trimmed_cube_edges.size(); i+=2) {
        glm::vec3 edgeA_p1 = trimmed_cube_edges[i];
        glm::vec3 edgeA_p2 = trimmed_cube_edges[i+1];
        
        for(int j = i+2; j<trimmed_cube_edges.size(); j+=2) {
            glm::vec3 edgeB_p1 = trimmed_cube_edges[j];
            glm::vec3 edgeB_p2 = trimmed_cube_edges[j+1];
            
            glm::vec3 p1check = glm::equal(edgeA_p1,edgeB_p1);
            glm::vec3 p2check = glm::equal(edgeA_p2,edgeB_p2);
            bool match1 = p1check[0]&&p1check[1]&&p1check[2]&&p2check[0]&&p2check[1]&&p2check[2];
            
            glm::vec3 p1p2check = glm::equal(edgeA_p1,edgeB_p2);
            glm::vec3 p2p1check = glm::equal(edgeA_p2,edgeB_p1);
            bool match2 = p1p2check[0]&&p1p2check[1]&&p1p2check[2]&&p2p1check[0]&&p2p1check[1]&&p2p1check[2];
            
            if(match1 || match2) {
                trimIndex.push_back(i);
                trimIndex.push_back(i+1);
            }
        }
    }
    std::reverse(trimIndex.begin(),trimIndex.end());
    for(int i=0; i<trimIndex.size(); i++) {
        trimmed_cube_edges.erase(trimmed_cube_edges.begin() + trimIndex[i]);
    }
    
    GLfloat plane_vertices[] = {
        10.0f, -5.0f, 20.0f,
        -10.0f, -5.0f, 20.0f,
        -10.0f, -5.0f, -20.0f,
        10.0f, -5.0f, 20.0f,
        -10.0f, -5.0f, -20.0f,
        10.0f, -5.0f, -20.0f
    };
    
    std::vector <glm::vec3> plane_edges;
    glm::vec3 planeVertex;
    
    for(int i=0; i<sizeof(plane_vertices)/4/3/3; i++) {
        glm::vec3 p0 = glm::vec3(plane_vertices[9*i],plane_vertices[9*i+1],plane_vertices[9*i+2]);
        glm::vec3 p1 = glm::vec3(plane_vertices[9*i+3],plane_vertices[9*i+4],plane_vertices[9*i+5]);
        glm::vec3 p2 = glm::vec3(plane_vertices[9*i+6],plane_vertices[9*i+7],plane_vertices[9*i+8]);
        plane_edges.push_back(p0);
        plane_edges.push_back(p1);
        plane_edges.push_back(p1);
        plane_edges.push_back(p2);
        plane_edges.push_back(p2);
        plane_edges.push_back(p0);
    }
    
    // create cube rigidObject
    std::vector <object> cubes;
    std::vector <AABB> bV;
    object cube;
    
    cube.CoM = centerOfMass(cube_vertices, sizeof(cube_vertices)/4/3/3);
    cube.MoI = boxMoI(1.0f, 4.0f, 4.0f, 4.0f);
    cube.objState.objPos = glm::vec3(5.0f,15.0f,5.0f);
    
    cube.objState.objP = glm::vec3(0.0f,0.0f,0.0f);
    cube.objState.objL = glm::vec3(0.3f,0.0f,0.0f);
    cubes.push_back(cube);
    
    cube.objState.objPos = glm::vec3(5.0f,10.0f,6.0f);
    cube.objState.objL = glm::vec3(-0.3f,0.0f,0.0f);
    cubes.push_back(cube);
    
    cube.objState.objPos = glm::vec3(5.0f,30.0f,-6.0f);
    cube.objState.objL = glm::vec3(0.0f,0.0f,0.0f);
    cubes.push_back(cube);
    
    cube.objState.objPos = glm::vec3(5.0f,20.0f,-15.0f);
    cube.objState.objL = glm::vec3(0.3f,0.0f,0.0f);
    cubes.push_back(cube);
    
    cube.objState.objPos = glm::vec3(5.0f,5.0f,10.0f);
    cube.objState.objL = glm::vec3(0.0f,0.3f,0.0f);
    cubes.push_back(cube);
    
    rigidBodyState K1, K2, K3, K4;
    
    GLfloat cubeface_vertices[3*3*12*cubes.size()], cubeface_color[3*3*12*cubes.size()], plane_color[3*3*2], hitRecord[3*tSphere.getVertexCount()], hitRecord_color[3*tSphere.getVertexCount()], vertex_velocity[3*12*2*cubes.size()], vertex_velocity_color[3*12*2*cubes.size()];
    glm::vec3 check_coll_hitpoint;
    
    bool AABBshow = false;
    
    for(int i=0; i<sizeof(vertex_velocity_color)/4; i++) {vertex_velocity_color[i] = 0.0f;}
    
    do{
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Use our shader
        glUseProgram(programID);
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        
        if (glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS) {
            AABBshow = true;
        }
        if (glfwGetKey( window, GLFW_KEY_E ) == GLFW_PRESS) {
            AABBshow = false;
        }
        
        // set mouse ctrl callback
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        
        glm::vec3 newballPos, newballVel, newballAcc;
        
        std::vector <rigidBodyState> oldState;
        rigidBodyState newState;
        
        int count = 0;
        while(count<0.15/dt) {
            for(int k=0; k<cubes.size(); k++) {
                //store current state
                oldState.push_back(cubes[k].objState);
                
                // calculate K-states for RK integration
                K1 = deriveRBState(cubes[k].objState, cubes[k].objState, 0.0f);
                K2 = deriveRBState(cubes[k].objState, K1, 0.5*dt);
                K3 = deriveRBState(cubes[k].objState, K2, 0.5*dt);
                K4 = deriveRBState(cubes[k].objState, K3, dt);
                
                //RK4 update state
                newState.objPos = cubes[k].objState.objPos + dt/6.0f*(K1.objPos+2.0f*K2.objPos+2.0f*K3.objPos+K4.objPos);
                newState.objOrient = cubes[k].objState.objOrient + dt/6.0f*(K1.objOrient+2.0f*K2.objOrient+2.0f*K3.objOrient+K4.objOrient);
                newState.objP = cubes[k].objState.objP + dt/6.0f*(K1.objP+2.0f*K2.objP+2.0f*K3.objP+K4.objP);
                newState.objL = cubes[k].objState.objL + dt/6.0f*(K1.objL+2.0f*K2.objL+2.0f*K3.objL+K4.objL);
                
                //normalize orientation
                glm::mat3 checkRot= newState.objOrient;
                glm::vec3 checkU;
                for(int i=0; i<3; i++) {
                    checkU = glm::vec3(newState.objOrient[0][i],newState.objOrient[1][i],newState.objOrient[2][i]);
                    checkU = checkU/glm::length(checkU);
                    newState.objOrient[0][i] = checkU[0];
                    newState.objOrient[1][i] = checkU[1];
                    newState.objOrient[2][i] = checkU[2];
                }
                //check for face-vertex collisions
                glm::vec3 check_coll_normal;
                glm::vec3 check_coll_pos;
                glm::vec3 coll_vel;
                //for each cube vertex
                for(int i=0; i<trimmed_cube_vertices.size(); i++) {
                    //for each plane face
                    for(int j=0; j<sizeof(plane_vertices)/3/3/4; j++) {
                        // get current position of vertex i
                        check_coll_pos = newState.objPos + newState.objOrient*trimmed_cube_vertices[i];
                        //calculate ra = pa - object center of mass
                        glm::vec3 ra = check_coll_pos - (newState.objPos);
                        //calculate local velocity of vertex
                        glm::mat3 iworldMoI = newState.objOrient*glm::inverse(cubes[k].MoI)*glm::transpose(newState.objOrient);
                        coll_vel = newState.objP + glm::cross(iworldMoI*newState.objL, ra);
                        
                        //get plane triangle
                        glm::vec3 p0 = glm::vec3(plane_vertices[9*j],plane_vertices[9*j+1],plane_vertices[9*j+2]);
                        glm::vec3 p1 = glm::vec3(plane_vertices[9*j+3],plane_vertices[9*j+4],plane_vertices[9*j+5]);
                        glm::vec3 p2 = glm::vec3(plane_vertices[9*j+6],plane_vertices[9*j+7],plane_vertices[9*j+8]);
                        check_coll_normal = triNormal(p0, p2, p1);
                        GLfloat vminus = glm::dot(coll_vel, check_coll_normal);
                        
                        float hitPoint = intersect(coll_vel, check_coll_pos, p0, p2, p1);
                        if(glm::length(-coll_vel*hitPoint) < 0.5f && vminus<0) { //collision w/triangle
                            // calculate j
                            GLfloat collj = (-(1+epsilon)*vminus)/(1+glm::dot(check_coll_normal,glm::cross(iworldMoI*glm::cross(ra,check_coll_normal),ra)));
                            glm::vec3 Jn = collj*check_coll_normal;
                            newState.objP = newState.objP + Jn;
                            newState.objL = newState.objL + glm::cross(ra,Jn);
                            //reset position/rotation
                            newState.objPos = cubes[k].objState.objPos+newState.objP*dt;
                            newState.objOrient = cubes[k].objState.objOrient;
                        }
                    }
                }
                // check for edge-edge collisions
                for(int i = 0; i<trimmed_cube_edges.size()/2; i++) {
                    for(int j = 0; j<plane_edges.size()/2; j++) {
                        glm::vec3 p1 = trimmed_cube_edges[2*i];
                        p1 = newState.objPos + newState.objOrient*p1;
                        glm::vec3 p2 = trimmed_cube_edges[2*i+1];
                        p2 = newState.objPos + newState.objOrient*p2;
                        glm::vec3 a = p2 - p1;
                        glm::vec3 an = a/glm::length(a);
                        glm::vec3 q2 = plane_edges[2*j];
                        glm::vec3 q1 = plane_edges[2*j+1];
                        glm::vec3 b = q2 - q1;
                        glm::vec3 bn = b/glm::length(b);
                        check_coll_normal = glm::cross(a,b)/glm::length(glm::cross(a,b));
                        glm::vec3 r = q1 - p1;
                        GLfloat s = glm::dot(r, glm::cross(bn,check_coll_normal))/glm::dot(a,glm::cross(bn, check_coll_normal));
                        GLfloat t = glm::dot(-r, glm::cross(an,check_coll_normal))/glm::dot(b,glm::cross(an, check_coll_normal));
                        glm::vec3 pa = p1+s*a;
                        glm::vec3 qa = q1+t*b;
                        // check for valid s/t
                        if(s>0 && s<1 && t>0 && t<1) {
                            if(glm::length(pa - qa)<0.05f) {
                                //char junk; std::cin>>junk;
                                check_coll_pos = pa;
                                glm::vec3 ra = check_coll_pos - (newState.objPos);
                                glm::mat3 iworldMoI = newState.objOrient*glm::inverse(cubes[k].MoI)*glm::transpose(newState.objOrient);
                                coll_vel = newState.objP + glm::cross(iworldMoI*newState.objL, ra);
                                GLfloat vminus = glm::dot(coll_vel, check_coll_normal);
                                if(vminus<0) {
                                    GLfloat collj = (-(1+epsilon)*vminus)/(1+glm::dot(check_coll_normal,glm::cross(iworldMoI*glm::cross(ra,check_coll_normal),ra)));
                                    glm::vec3 Jn = collj*check_coll_normal;
                                    newState.objP = newState.objP + Jn;
                                    newState.objL = newState.objL + glm::cross(ra,Jn);
                                    //reset position/rotation
                                    newState.objPos = cubes[k].objState.objPos+newState.objP*dt;
                                    newState.objOrient = cubes[k].objState.objOrient;
                                }
                            }
                        }
                    }
                }
                cubes[k].objState = newState;
            }
            
            //update AABBs
            bV.clear();
            for(int k=0; k<cubes.size(); k++) {
                AABB tempbV;
                GLfloat minx = cubes[k].objState.objPos[0];
                GLfloat miny = cubes[k].objState.objPos[1];
                GLfloat minz = cubes[k].objState.objPos[2];
                GLfloat maxx = cubes[k].objState.objPos[0];
                GLfloat maxy = cubes[k].objState.objPos[1];
                GLfloat maxz = cubes[k].objState.objPos[2];
                for(int i = 0; i<trimmed_cube_vertices.size(); i++) {
                    glm::vec3 tempV = cubes[k].objState.objPos + cubes[k].objState.objOrient*trimmed_cube_vertices[i];
                    if(minx>tempV[0]) {minx = tempV[0];}
                    if(miny>tempV[1]) {miny = tempV[1];}
                    if(minz>tempV[2]) {minz = tempV[2];}
                    if(maxx<tempV[0]) {maxx = tempV[0];}
                    if(maxy<tempV[1]) {maxy = tempV[1];}
                    if(maxz<tempV[2]) {maxz = tempV[2];}
                }
                tempbV.minX=minx-0.2f;
                tempbV.minY=miny-0.2f;
                tempbV.minZ=minz-0.2f;
                tempbV.maxX=maxx+0.2f;
                tempbV.maxY=maxy+0.2f;
                tempbV.maxZ=maxz+0.2f;
                bV.push_back(tempbV);
            }
            
            // check for vertex-face collisions against other moving objects
            for(int k=0; k<cubes.size(); k++) {
                glm::vec3 check_coll_normal;
                glm::vec3 check_coll_pos;
                glm::vec3 coll_vel;
                for(int i = 0; i<trimmed_cube_vertices.size(); i++) {
                    for(int z=0; z<cubes.size(); z++) {
                        if(z!=k) {
                            //AABB check
                            if(AABBcol(bV[k],bV[z])==true) {
                                for(int j=0; j<sizeof(cube_vertices)/4/3/3; j++) {
                                    //get object a position & velocity
                                    check_coll_pos = cubes[k].objState.objPos + cubes[k].objState.objOrient*trimmed_cube_vertices[i];
                                    glm::vec3 ra = check_coll_pos - (cubes[k].objState.objPos);
                                    glm::mat3 iworldMoIa = cubes[k].objState.objOrient*glm::inverse(cubes[k].MoI)*glm::transpose(cubes[k].objState.objOrient);
                                    coll_vel = cubes[k].objState.objP + glm::cross(iworldMoIa*cubes[k].objState.objL, ra);
                                    
                                    //get object b face vertices & normal
                                    glm::vec3 p0 = glm::vec3(cube_vertices[9*j],cube_vertices[9*j+1],cube_vertices[9*j+2]);
                                    p0 = cubes[z].objState.objPos + cubes[z].objState.objOrient*p0;
                                    glm::vec3 p1 = glm::vec3(cube_vertices[9*j+3],cube_vertices[9*j+4],cube_vertices[9*j+5]);
                                    p1 = cubes[z].objState.objPos + cubes[z].objState.objOrient*p1;
                                    glm::vec3 p2 = glm::vec3(cube_vertices[9*j+6],cube_vertices[9*j+7],cube_vertices[9*j+8]);
                                    p2 = cubes[z].objState.objPos + cubes[z].objState.objOrient*p2;
                                    check_coll_normal = triNormal(p0, p1, p2);
                                    
                                    //check for intersect
                                    float hitPoint = intersect(coll_vel, check_coll_pos, p0, p1, p2);
                                    //broad collision check
                                    if(glm::length(-coll_vel*hitPoint) < 0.1f) {
                                        //calculate pb
                                        glm::vec3 pb = check_coll_pos+hitPoint*coll_vel;
                                        glm::vec3 rb = pb - cubes[z].objState.objPos;
                                        // get object b I(-1)
                                        glm::mat3 iworldMoIb = cubes[z].objState.objOrient*glm::inverse(cubes[z].MoI)*glm::transpose(cubes[z].objState.objOrient);
                                        glm::vec3 vb = cubes[z].objState.objP + glm::cross(iworldMoIb*cubes[z].objState.objL, rb);
                                        // determine relative velocity
                                        GLfloat vrel = glm::dot(check_coll_normal,coll_vel-vb);
                                        // check objects moving towards each other
                                        if (vrel<0) {
                                            GLfloat collj = (-(1+epsilon)*vrel)/(1+1+glm::dot(check_coll_normal, glm::cross(iworldMoIa*glm::cross(ra,check_coll_normal),ra)+glm::cross(iworldMoIb*glm::cross(rb,check_coll_normal),rb)));
                                            
                                            //update cube k state
                                            cubes[k].objState.objP = cubes[k].objState.objP + collj*check_coll_normal;
                                            cubes[k].objState.objL = cubes[k].objState.objL + collj*glm::cross(ra,check_coll_normal);
                                            //reset position/rotation
                                            cubes[k].objState.objPos = oldState[k].objPos+cubes[k].objState.objP*dt;
                                            cubes[k].objState.objOrient = oldState[k].objOrient;
                                            
                                            //update cube z state
                                            cubes[z].objState.objP = cubes[z].objState.objP + -collj*check_coll_normal;
                                            cubes[z].objState.objL = cubes[z].objState.objL + -collj*glm::cross(rb,check_coll_normal);
                                            //reset position/rotation
                                            cubes[z].objState.objPos = oldState[z].objPos + cubes[z].objState.objP*dt;
                                            cubes[z].objState.objOrient = oldState[z].objOrient;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            
            //update AABBs
            bV.clear();
            for(int k=0; k<cubes.size(); k++) {
                AABB tempbV;
                GLfloat minx = cubes[k].objState.objPos[0];
                GLfloat miny = cubes[k].objState.objPos[1];
                GLfloat minz = cubes[k].objState.objPos[2];
                GLfloat maxx = cubes[k].objState.objPos[0];
                GLfloat maxy = cubes[k].objState.objPos[1];
                GLfloat maxz = cubes[k].objState.objPos[2];
                for(int i = 0; i<trimmed_cube_vertices.size(); i++) {
                    glm::vec3 tempV = cubes[k].objState.objPos + cubes[k].objState.objOrient*trimmed_cube_vertices[i];
                    if(minx>tempV[0]) {minx = tempV[0];}
                    if(miny>tempV[1]) {miny = tempV[1];}
                    if(minz>tempV[2]) {minz = tempV[2];}
                    if(maxx<tempV[0]) {maxx = tempV[0];}
                    if(maxy<tempV[1]) {maxy = tempV[1];}
                    if(maxz<tempV[2]) {maxz = tempV[2];}
                }
                tempbV.minX=minx-0.2f;
                tempbV.minY=miny-0.2f;
                tempbV.minZ=minz-0.2f;
                tempbV.maxX=maxx+0.2f;
                tempbV.maxY=maxy+0.2f;
                tempbV.maxZ=maxz+0.2f;
                
                vertex_velocity[0+k*12*2*3] = minx;
                vertex_velocity[1+k*12*2*3] = miny;
                vertex_velocity[2+k*12*2*3] = minz;
                vertex_velocity[3+k*12*2*3] = maxx;
                vertex_velocity[4+k*12*2*3] = miny;
                vertex_velocity[5+k*12*2*3] = minz;
                vertex_velocity[6+k*12*2*3] = maxx;
                vertex_velocity[7+k*12*2*3] = miny;
                vertex_velocity[8+k*12*2*3] = minz;
                vertex_velocity[9+k*12*2*3] = maxx;
                vertex_velocity[10+k*12*2*3] = miny;
                vertex_velocity[11+k*12*2*3] = maxz;
                vertex_velocity[12+k*12*2*3] = maxx;
                vertex_velocity[13+k*12*2*3] = miny;
                vertex_velocity[14+k*12*2*3] = maxz;
                vertex_velocity[15+k*12*2*3] = minx;
                vertex_velocity[16+k*12*2*3] = miny;
                vertex_velocity[17+k*12*2*3] = maxz;
                vertex_velocity[18+k*12*2*3] = minx;
                vertex_velocity[19+k*12*2*3] = miny;
                vertex_velocity[20+k*12*2*3] = maxz;
                vertex_velocity[21+k*12*2*3] = minx;
                vertex_velocity[22+k*12*2*3] = miny;
                vertex_velocity[23+k*12*2*3] = minz;
                
                vertex_velocity[24+k*12*2*3] = minx;
                vertex_velocity[25+k*12*2*3] = maxy;
                vertex_velocity[26+k*12*2*3] = minz;
                vertex_velocity[27+k*12*2*3] = maxx;
                vertex_velocity[28+k*12*2*3] = maxy;
                vertex_velocity[29+k*12*2*3] = minz;
                vertex_velocity[30+k*12*2*3] = maxx;
                vertex_velocity[31+k*12*2*3] = maxy;
                vertex_velocity[32+k*12*2*3] = minz;
                vertex_velocity[33+k*12*2*3] = maxx;
                vertex_velocity[34+k*12*2*3] = maxy;
                vertex_velocity[35+k*12*2*3] = maxz;
                vertex_velocity[36+k*12*2*3] = maxx;
                vertex_velocity[37+k*12*2*3] = maxy;
                vertex_velocity[38+k*12*2*3] = maxz;
                vertex_velocity[39+k*12*2*3] = minx;
                vertex_velocity[40+k*12*2*3] = maxy;
                vertex_velocity[41+k*12*2*3] = maxz;
                vertex_velocity[42+k*12*2*3] = minx;
                vertex_velocity[43+k*12*2*3] = maxy;
                vertex_velocity[44+k*12*2*3] = maxz;
                vertex_velocity[45+k*12*2*3] = minx;
                vertex_velocity[46+k*12*2*3] = maxy;
                vertex_velocity[47+k*12*2*3] = minz;
                
                vertex_velocity[48+k*12*2*3] = minx;
                vertex_velocity[49+k*12*2*3] = miny;
                vertex_velocity[50+k*12*2*3] = minz;
                vertex_velocity[51+k*12*2*3] = minx;
                vertex_velocity[52+k*12*2*3] = maxy;
                vertex_velocity[53+k*12*2*3] = minz;
                vertex_velocity[54+k*12*2*3] = maxx;
                vertex_velocity[55+k*12*2*3] = miny;
                vertex_velocity[56+k*12*2*3] = minz;
                vertex_velocity[57+k*12*2*3] = maxx;
                vertex_velocity[58+k*12*2*3] = maxy;
                vertex_velocity[59+k*12*2*3] = minz;
                vertex_velocity[60+k*12*2*3] = maxx;
                vertex_velocity[61+k*12*2*3] = miny;
                vertex_velocity[62+k*12*2*3] = maxz;
                vertex_velocity[63+k*12*2*3] = maxx;
                vertex_velocity[64+k*12*2*3] = maxy;
                vertex_velocity[65+k*12*2*3] = maxz;
                vertex_velocity[66+k*12*2*3] = minx;
                vertex_velocity[67+k*12*2*3] = miny;
                vertex_velocity[68+k*12*2*3] = maxz;
                vertex_velocity[69+k*12*2*3] = minx;
                vertex_velocity[70+k*12*2*3] = maxy;
                vertex_velocity[71+k*12*2*3] = maxz;
                
                bV.push_back(tempbV);
            }
            
            for(int k = 0; k<cubes.size(); k++) {
                glm::vec3 check_coll_normal;
                glm::vec3 check_coll_pos;
                glm::vec3 coll_vel;
                // check for edge-edge collisions against other objects
                for(int i = 0; i<trimmed_cube_edges.size()/2; i++) {
                    for(int z = 0; z<cubes.size(); z++) {
                        //AABB check
                        if(z!=k && AABBcol(bV[k],bV[z])==true) {
                            for(int j = 0; j<trimmed_cube_edges.size()/2; j++) {
                                // get cube k edge info
                                glm::vec3 p1 = trimmed_cube_edges[2*i];
                                p1 = cubes[k].objState.objPos + cubes[k].objState.objOrient*p1;
                                glm::vec3 p2 = trimmed_cube_edges[2*i+1];
                                p2 = cubes[k].objState.objPos + cubes[k].objState.objOrient*p2;
                                glm::vec3 a = p2 - p1;
                                glm::vec3 an = a/glm::length(a);
                                
                                // get cube z edge info
                                glm::vec3 q1 = trimmed_cube_edges[2*j];
                                q1 = cubes[z].objState.objPos + cubes[z].objState.objOrient*q1;
                                glm::vec3 q2 = trimmed_cube_edges[2*j+1];
                                q2 = cubes[z].objState.objPos + cubes[z].objState.objOrient*q2;
                                glm::vec3 b = q2 - q1;
                                glm::vec3 bn = b/glm::length(b);
                                
                                //get collision points & normal
                                check_coll_normal = glm::cross(a,b)/glm::length(glm::cross(a,b));
                                glm::vec3 r = q1 - p1;
                                GLfloat s = glm::dot(r, glm::cross(bn,check_coll_normal))/glm::dot(a,glm::cross(bn, check_coll_normal));
                                GLfloat t = glm::dot(-r, glm::cross(an,check_coll_normal))/glm::dot(b,glm::cross(an, check_coll_normal));
                                glm::vec3 pa = p1+s*a;
                                glm::vec3 pb = q1+t*b;
                                
                                // check for valid s/t
                                if(s>0 && s<1 && t>0 && t<1) {
                                    if(glm::length(pa - pb)<0.05f) {
                                        //get pa velocity
                                        check_coll_pos = pa;
                                        glm::vec3 ra = check_coll_pos - cubes[k].objState.objPos;
                                        glm::mat3 iworldMoIa = cubes[k].objState.objOrient*glm::inverse(cubes[k].MoI)*glm::transpose(cubes[k].objState.objOrient);
                                        coll_vel = cubes[k].objState.objP + glm::cross(iworldMoIa*cubes[k].objState.objL, ra);
                                        
                                        //get pb velocity
                                        glm::vec3 rb = pb - cubes[z].objState.objPos;
                                        glm::mat3 iworldMoIb = cubes[z].objState.objOrient*glm::inverse(cubes[z].MoI)*glm::transpose(cubes[z].objState.objOrient);
                                        glm::vec3 vb = cubes[z].objState.objP + glm::cross(iworldMoIb*cubes[z].objState.objL, rb);
                                        
                                        // determine relative velocity
                                        GLfloat vrel = glm::dot(check_coll_normal,coll_vel-vb);
                                        if(vrel<0) {
                                            GLfloat collj = (-(1+epsilon)*vrel)/(1+1+glm::dot(check_coll_normal, glm::cross(iworldMoIa*glm::cross(ra,check_coll_normal),ra)+glm::cross(iworldMoIb*glm::cross(rb,check_coll_normal),rb)));
                                            
                                            //update cube k state
                                            cubes[k].objState.objP = cubes[k].objState.objP + collj*check_coll_normal;
                                            cubes[k].objState.objL = cubes[k].objState.objL + collj*glm::cross(ra,check_coll_normal);
                                            //reset position/rotation
                                            cubes[k].objState.objPos = oldState[k].objPos+cubes[k].objState.objP*dt;
                                            cubes[k].objState.objOrient = oldState[k].objOrient;
                                            
                                            //update cube z state
                                            cubes[z].objState.objP = cubes[z].objState.objP + -collj*check_coll_normal;
                                            cubes[z].objState.objL = cubes[z].objState.objL + -collj*glm::cross(rb,check_coll_normal);
                                            //reset position/rotation
                                            cubes[z].objState.objPos = oldState[z].objPos + cubes[z].objState.objP*dt;
                                            cubes[z].objState.objOrient = oldState[z].objOrient;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            count++;
        }
        
        for(int k = 0; k< cubes.size(); k++) {
            for(int i = 0; i<sizeof(cube_vertices)/4/3; i++) {
                glm::vec3 updateVertex = cubes[k].objState.objPos + cubes[k].objState.objOrient*glm::vec3(cube_vertices[3*i],cube_vertices[3*i+1],cube_vertices[3*i+2]);
                cubeface_vertices[3*i+k*sizeof(cube_vertices)/4] = updateVertex[0];
                cubeface_vertices[3*i+1+k*sizeof(cube_vertices)/4] = updateVertex[1];
                cubeface_vertices[3*i+2+k*sizeof(cube_vertices)/4] = updateVertex[2];
                cubeface_color[3*i+k*sizeof(cube_vertices)/4] = 0.02f*i;
                cubeface_color[3*i+1+k*sizeof(cube_vertices)/4] = 0.1f;
                cubeface_color[3*i+2+k*sizeof(cube_vertices)/4] = 0.1f;
            }
        }
        
        for(int k = 0; k<sizeof(plane_vertices)/4/3; k++) {
            plane_vertices[3*k] = plane_vertices[3*k];
            plane_vertices[3*k+1] = plane_vertices[3*k+1];
            plane_vertices[3*k+2] = plane_vertices[3*k+2];
            plane_color[3*k] = 0.2f;
            plane_color[3*k+1] = 0.5f;
            plane_color[3*k+2] = 0.7f;
        }
        
        // draw AABB lines
        if(AABBshow) {
            glGenBuffers(1, &vvertexbuffer);
            glBindBuffer(GL_ARRAY_BUFFER, vvertexbuffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_velocity), vertex_velocity, GL_STATIC_DRAW);
            
            glGenBuffers(1, &colorbuffer);
            glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_velocity_color), vertex_velocity_color, GL_STATIC_DRAW);
            
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, vvertexbuffer);
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
            
            glDrawArrays(GL_LINES,0,12*2*cubes.size());
        }
        
        // draw plane
        glGenBuffers(1, &vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(plane_vertices), plane_vertices, GL_STATIC_DRAW);
        
        glGenBuffers(1, &colorbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(plane_color), plane_color, GL_STATIC_DRAW);
        
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
        
        glDrawArrays(GL_TRIANGLES,0,3*3*2);
        
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
        
        glDrawArrays(GL_TRIANGLES,0,3*3*12*cubes.size());
        
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
