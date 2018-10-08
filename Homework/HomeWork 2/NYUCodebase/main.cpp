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

//CLASSES
class Entity {
public:
    virtual void Draw(ShaderProgram &p) = 0;
protected:
    float x, y, x_velocity, y_velocity, speed, width, height;
    glm::mat4 player;
};

class Player : public Entity{
public:
    Player(){
        player = glm::mat4(1.0f);
        height = 0.4;
        width = .125 /2;
        x, y = 0.0f;
        
    }
    void Draw(ShaderProgram &p){
        program.SetModelMatrix(player);
        program.SetProjectionMatrix(projectionMatrix);
        program.SetViewMatrix(viewMatrix);
        float vertices[] = {-width,-height,width,-height,width,height,-width,-height,-width,height,width,height};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    
    void translate(float x_input, float y_input){
        x += x_input;
        y += y_input;
        player = glm::translate(player, glm::vec3(x_input, y_input, 0.0f));
    }
    
    void move(float y_input){
        player = glm::translate(player, glm::vec3(0.0f, y_input, 0.0f));
        y += y_input;
        std::cout << y << std::endl;
    }
    
    float getWidth(){
        return width;
    }
    
    float getHeight(){
        return height;
    }
private:
    glm::mat4 player;
};

class Ball : public Entity{
public:
    Ball(){
        player = glm::mat4(1.0f);
        x_velocity = -0.1f;
        y_velocity = 0.0f;

        height = 0.05;
        width = 0.05;
    }
    
    void Draw(ShaderProgram &p){
        program.SetModelMatrix(player);
        program.SetProjectionMatrix(projectionMatrix);
        program.SetViewMatrix(viewMatrix);
        float vertices[] = {-width,-height,width,-height,width,height,-width,-height,-width,height,width,height};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    
    void translate(float x_input, float y_input){
        x += x_input;
        y += y_input;
        player = glm::translate(player, glm::vec3(x_input, y_input, 0.0f));
    }
    
    void move(float y_input){
        player = glm::translate(player, glm::vec3(0.0f, y_input, 0.0f));
        y += y_input;
    }
    
    float getWidth(){
        return width;
    }
    
    float getHeight(){
        return height;
    }
};


//Entities
Player* player1 = new Player();
Player* player2 = new Player();
Ball* ball = new Ball();

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
    
    player1->translate(-1.77 + player1->getWidth() / 2, 0.0);
    player2->translate(1.77 - player2->getWidth() / 2, 0.0);
    
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
        else if(event.type == SDL_KEYDOWN) {
            //Player 1
            if(event.key.keysym.scancode == SDL_SCANCODE_W) {
                player1->move(0.1);
            }
            if(event.key.keysym.scancode == SDL_SCANCODE_S) {
                player1->move(-0.1);
            }
            
            //Player 2
            if(event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
                player2->move(-0.1);
            }
            if(event.key.keysym.scancode == SDL_SCANCODE_UP) {
                player2->move(0.1);
            }
        }
    }
}

void Update(){
    //Ball Update
    
    //Collision Detection

    //Clear Buffer
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer(){
    player1->Draw(program);
    player2->Draw(program);
    ball->Draw(program);
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
