#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <glm/glm.hpp>


static void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

/*
int main(void)
{
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
    while (!glfwWindowShouldClose(window))
    {
        float ratio;
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glRotatef((float) glfwGetTime() * 50.f, 0.f, 0.f, 1.f);
        glBegin(GL_TRIANGLES);
        glColor3f(1.f, 0.f, 0.f);
        glVertex3f(-0.6f, -0.4f, 0.f);
        glColor3f(0.f, 1.f, 0.f);
        glVertex3f(0.6f, -0.4f, 0.f);
        glColor3f(0.f, 0.f, 1.f);
        glVertex3f(0.f, 0.6f, 0.f);
        glEnd();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
*/




 
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
    glUseProgram(programID);
    float speed = 50.0f;
 
    glm::mat4 scene_mat = glm::rotate( glm::mat4(1.0f), static_cast<float>( curr_time.getVal() ) * speed, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 MVP = gEngine->getActiveModelViewProjectionMatrix() * scene_mat;
 
    //BIND SHADER HERE
    // ------
 
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
    glfwDestroyWindow(window);
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