#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

//GLOBAL VARIABLES
SDL_Window* displayWindow;
ShaderProgram program;

SDL_Event event;

const float width = 640;
const float height = 360;
bool loop = false;

glm::mat4 projectionMatrix;
glm::mat4 viewMatrix;

float openGL_width = 1.77f;
float openGL_height = 1.0f;

//Win Condition
int winState = 0;

//Timer
float lastFrameTicks = 0.0f;
float elapsed = 0.0f;


class Entity{
public:
    void Draw(ShaderProgram &p){
        program.SetModelMatrix(player);
        program.SetProjectionMatrix(projectionMatrix);
        program.SetViewMatrix(viewMatrix);
        float vertices[] = {-width + x,-height+y,width+x,-height+y,width+x,height+y,-width+x,-height+y,-width+x,height+y,width+x,height+y};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    
    glm::mat4 player = glm::mat4(1.0f);
    
    float x;
    float y;
    float x_scale;
    float y_scale;
    float rotation;
    
    int textureID;
    
    float width = 0.5;
    float height = 0.5;
    
    float velocity;
    float direction_x;
    float direction_y;
    
    
};


//Entities
Entity player;


//METHODS

void Startup(){
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    glViewport(0, 0, width, height);
    
    program.Load(RESOURCE_FOLDER "vertex.glsl", RESOURCE_FOLDER "fragment.glsl");
    
    projectionMatrix = glm::mat4(1.0f);
    viewMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-openGL_width, openGL_width, -openGL_height, openGL_height, -openGL_height, openGL_height);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(program.programID);
#ifdef _WINDOWS
    glewInit();
#endif
}

void ProcessEvents(){
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            loop = true;
        }
    }
}

void Update(){
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer(){
    player.Draw(program);
    SDL_GL_SwapWindow(displayWindow);
}


int main(int argc, char *argv[])
{
    Startup();
    while (!loop) {
        ProcessEvents();
        Update();
        Renderer();
    }
    SDL_Quit();
    return 0;
}
