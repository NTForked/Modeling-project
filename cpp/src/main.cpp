#include <stdlib.h>
#include <stdio.h>
#include <iostream>
// Include GLEW. Always include it before gl.h and glfw.h, since it's a bit magic.
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <shader.h>

static void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}




const int N_ROWS = 2;
const int N_COLS = 2;

const int N_TYPE1 = (N_ROWS-1)*N_COLS; // |
const int N_TYPE2 = (N_ROWS-1)*(N_COLS-1); // /
const int N_TYPE3 = N_ROWS*(N_COLS-1); // _
const int N_TYPE4 = N_TYPE2; // \

const int N_MASSES = N_ROWS*N_COLS;
const int N_CONNECTIONS = N_TYPE1+N_TYPE2+N_TYPE3+N_TYPE4;

float masses[N_MASSES];
glm::vec2 positions[N_MASSES][2];
glm::vec2 velocities[N_MASSES][2];
glm::vec2 forces[N_MASSES];

int connected_masses[N_CONNECTIONS][2];
float spring_constants[N_CONNECTIONS];
float damper_constants[N_CONNECTIONS];
float spring_lengths[N_CONNECTIONS];

const float g = 9.82f;



int main(void)
{
    /* INIT GLFW */
    GLFWwindow* window;
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        exit(EXIT_FAILURE);
    window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    /* INIT SIMULATION */

    // Set start values to masses, positions velocities and forces
    for (int i = 0; i < N_MASSES; ++i)
    {
        masses[i] = 1.0f;
        int row = i%N_COLS;
        int col = floor(i/N_COLS);
        positions[i][0] = glm::vec2(1.0f*row - 0.5f,1.0f*col - 0.5f);
        positions[i][1] = glm::vec2(0,0);
        velocities[i][0] = glm::vec2(0,0);
        velocities[i][1] = glm::vec2(0,0);
        forces[i] = glm::vec2(0,0);
    }
    positions[0][0] = glm::vec2(0.0f,-1.0f);


    // Set values for springs, dampers, and lengths
    for (int i = 0; i < N_CONNECTIONS; ++i)
    {
        spring_constants[i] = 500.0f;
        damper_constants[i] = 5.0f;
        spring_lengths[i] = 1.0f;
    }

    // Set type_2 connections to legnth sqrt(2)
    for (int i = N_TYPE1; i < N_TYPE1+N_TYPE2; ++i)
    {
        spring_lengths[i] = sqrt(2);
    }

    // Set type_4 connections to legnth sqrt(2)
    for (int i = N_TYPE1+N_TYPE2+N_TYPE3; i < N_CONNECTIONS; ++i)
    {
        spring_lengths[i] = sqrt(2);
    }

    /*
    Mass indices:
    2---------3
    |  \   /  |
    |    X    |
    |  /   \  |
    0---------1

    Connection indices:
    #----4----#
    |  \  2/  |
    0    X    1
    |  /   \5 |
    #----3----#
    */

    // Hard coded connections
    connected_masses[0][0] = 0;
    connected_masses[0][1] = 2;
    connected_masses[1][0] = 1;
    connected_masses[1][1] = 3;
    connected_masses[2][0] = 0;
    connected_masses[2][1] = 3;
    connected_masses[3][0] = 0;
    connected_masses[3][1] = 1;
    connected_masses[4][0] = 2;
    connected_masses[4][1] = 3;
    connected_masses[5][0] = 1;
    connected_masses[5][1] = 2;

    int read_buffer = 0;
    int write_buffer = 1;

    float T = 0.01f;

    glEnable( GL_POINT_SMOOTH );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glPointSize( 6.0 );


    while (!glfwWindowShouldClose(window))
    {
        /* SIMULATION */
        for (int connection_index = 0; connection_index < N_CONNECTIONS; ++connection_index)
        {
            //connection properties
            float k = spring_constants[connection_index];
            float b = damper_constants[connection_index];
            float l = spring_lengths[connection_index];

            //sosition
            int mass_index1 = connected_masses[connection_index][0];
            int mass_index2 = connected_masses[connection_index][1];
            glm::vec2 p1 = positions[mass_index1][read_buffer];
            glm::vec2 p2 = positions[mass_index2][read_buffer];
            glm::vec2 delta_p = p1 - p2;
            glm::vec2 delta_p_hat = glm::normalize(delta_p);//Not tested

            //velocities
            glm::vec2 v1 = velocities[mass_index1][read_buffer];
            glm::vec2 v2 = velocities[mass_index2][read_buffer];
            glm::vec2 delta_v = v1 - v2;

            //calculate force from connection
            glm::vec2 force_from_connection = ( -k*(glm::length(delta_p) - l) - b*(delta_v*delta_p_hat) )*delta_p_hat;
            forces[mass_index1] += force_from_connection;
            forces[mass_index2] -= force_from_connection;
        }

        //Calculate acceleration, velocity and position
        for (int mass_index = 0; mass_index < N_MASSES; ++mass_index)
        {
            glm::vec2 a = forces[mass_index]/masses[mass_index];// - glm::vec2(0.f,-1.f)*g;
            glm::vec2 v = velocities[mass_index][read_buffer] + a*T;
            glm::vec2 p = positions[mass_index][read_buffer] + v*T;

            //Check collision with y = 0
            if (p[1] < 0.0f)
            {
                //p[1] = 0.0f;
                //v[1] = -v[1];
            }

            //Store information in backbuffer
            velocities[mass_index][write_buffer] = v;
            positions[mass_index][write_buffer] = p;

            //reset force
            forces[mass_index] = glm::vec2(0.0f, 0.0f);
        }




        /* DRAW */
        float ratio;
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);

        //Init gl points
        glEnable( GL_POINT_SMOOTH );
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glPointSize( 6.0 );

        glBegin(GL_POINTS);
        glColor3f(1.f, 0.f, 0.f);
        for (int i = 0; i < N_MASSES; ++i)
        {
            glVertex3f(positions[i][read_buffer][0],
                       positions[i][read_buffer][1], 0.f);
        }
        glEnd();

        //Swap simulation buffers
        read_buffer = (read_buffer+1)%2;
        write_buffer = (write_buffer+1)%2;

        //Swap draw buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    /* CLEAN UP GLFW */
    glfwDestroyWindow(window);
    glfwTerminate();

    exit(EXIT_SUCCESS);
}


/*

void myInitFun();
void myDrawFun();
void myPreSyncFun();
void myCleanUpFun();
 
//global vars
GLuint vertexArray = GL_FALSE;
GLuint vertexPositionBuffer = GL_FALSE;
GLuint vertexColorBuffer = GL_FALSE;

GLuint programID;

GLint Matrix_Loc = -1;
 
int main( int argc, char* argv[] )
{
    // Init
    myInitFun();








    // Init glfw
    GLFWwindow* window;
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        exit(EXIT_FAILURE);
    window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    

    // Data
    const GLfloat vertex_position_data[] = { 
        -0.5f, -0.5f, 0.0f,
         0.0f, 0.5f, 0.0f,
         0.5f, -0.5f, 0.0f
    };
 
    const GLfloat vertex_color_data[] = { 
        1.0f, 0.0f, 0.0f, //red
        0.0f, 1.0f, 0.0f, //green
        0.0f, 0.0f, 1.0f //blue
    };
 
    //generate the VAO
    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);
 
    //generate VBO for vertex positions
    glGenBuffers(1, &vertexPositionBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexPositionBuffer);
    //upload data to GPU
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_position_data), vertex_position_data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        reinterpret_cast<void*>(0) // array buffer offset
    );
 
    //generate VBO for vertex colors
    glGenBuffers(1, &vertexColorBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexColorBuffer);
    //upload data to GPU
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_color_data), vertex_color_data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        reinterpret_cast<void*>(0) // array buffer offset
    );
 
    glBindVertexArray(0); //unbind
 

    //ADD SHADER HERE
    programID = LoadShaders( "data/shaders/simple.vert", "data/shaders/simple.frag" );
 
    //BIND SHADER HERE
    //glUseProgram(programID);
 
    //GET UNIFORM LOCATION FOR MVP MATRIX HERE
    //Matrix_Loc = sgct::ShaderManager::instance()->getShaderProgram( "xform").getUniformLocation( "MVP" );

    //UNBIND SHADER HERE
    // ------ 









 
    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        float ratio;
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        
        myDrawFun();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    myCleanUpFun();
     
    // Exit program
}
 
void myInitFun()
{
    // Init glfw
    GLFWwindow* window;
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        exit(EXIT_FAILURE);
    window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    

    // Data
    const GLfloat vertex_position_data[] = { 
        -0.5f, -0.5f, 0.0f,
         0.0f, 0.5f, 0.0f,
         0.5f, -0.5f, 0.0f
    };
 
    const GLfloat vertex_color_data[] = { 
        1.0f, 0.0f, 0.0f, //red
        0.0f, 1.0f, 0.0f, //green
        0.0f, 0.0f, 1.0f //blue
    };
 
    //generate the VAO
    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);
 
    //generate VBO for vertex positions
    glGenBuffers(1, &vertexPositionBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexPositionBuffer);
    //upload data to GPU
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_position_data), vertex_position_data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        reinterpret_cast<void*>(0) // array buffer offset
    );
 
    //generate VBO for vertex colors
    glGenBuffers(1, &vertexColorBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexColorBuffer);
    //upload data to GPU
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_color_data), vertex_color_data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        reinterpret_cast<void*>(0) // array buffer offset
    );
 
    glBindVertexArray(0); //unbind
 

    //ADD SHADER HERE
    programID = LoadShaders( "data/shaders/simple.vert", "data/shaders/simple.frag" );
 
    //BIND SHADER HERE
    //glUseProgram(programID);
 
    //GET UNIFORM LOCATION FOR MVP MATRIX HERE
    //Matrix_Loc = sgct::ShaderManager::instance()->getShaderProgram( "xform").getUniformLocation( "MVP" );

    //UNBIND SHADER HERE
    // ------ 
}
 
void myDrawFun()
{
    float speed = 50.0f;
 
    //glm::mat4 scene_mat = glm::mat4(1.0f);///glm::rotate( glm::mat4(1.0f), static_cast<float>( curr_time.getVal() ) * speed, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 MVP = glm::mat4(1.0f);
 
    //BIND SHADER HERE
    glUseProgram(programID);
 
    glUniformMatrix4fv(Matrix_Loc, 1, GL_FALSE, &MVP[0][0]);
 
    glBindVertexArray(vertexArray);
 
    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, 3);
 
    //unbind
    glBindVertexArray(0);
    
    //UNBIND SHADER HERE
    glUseProgram(0);
}
 
void myPreSyncFun()
{

}
 
void myCleanUpFun()
{
    // Terminate glfw
    //glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);

    // Release memory
    if(vertexPositionBuffer)
        glDeleteBuffers(1, &vertexPositionBuffer);
    if(vertexColorBuffer)
        glDeleteBuffers(1, &vertexColorBuffer);
    if(vertexArray)
        glDeleteVertexArrays(1, &vertexArray);
}

*/