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
        y += y_input * elapsed;
        player = glm::translate(player, glm::vec3(0.0f, y_input * elapsed, 0.0f));
    }
    
    float getWidth(){
        return width * 2;
    }
    
    float getHeight(){
        return height * 2;
    }
    
    float getX(){
        return x;
    }
    
    float getY(){
        return y;
    }
    
private:
    glm::mat4 player;
};

class Ball : public Entity{
public:
    Ball(){
        player = glm::mat4(1.0f);
        initialState = player;
        x_velocity = 0.9f;
        y_velocity = 0.9f;

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
    
    void resetBall(){
        player = initialState;
        x = 0;
        y = 0;
        x_velocity = 0.9f;
        y_velocity = 0.9f;
    }
    
    void translate(float x_input, float y_input){
        x += x_input;
        y += y_input;
        player = glm::translate(player, glm::vec3(x_input, y_input, 0.0f));
    }
    
    void move(float y_input){
        y += y_input * elapsed;
        player = glm::translate(player, glm::vec3(0.0f, y_input * elapsed, 0.0f));
    }
    
    void updateBall(){
        x += x_velocity * elapsed;
        y += y_velocity * elapsed;
        player = glm::translate(player, glm::vec3(x_velocity * elapsed, y_velocity * elapsed, 0.0f));
        
    }
    
    void reverseVelocity(){
        x_velocity = -1*x_velocity;
        y_velocity = -1 * y_velocity;
    }
    
    void reverseYVelocity(){
        y_velocity = -1 * y_velocity;
    }
    
    float getWidth(){
        return width * 2;
    }
    
    float getHeight(){
        return height * 2;
    }
    
    float getX(){
        return x;
    }
    
    float getY(){
        return y;
    }
private:
    glm::mat4 initialState;
};


//Entities
Player* player1 = new Player();
Player* player2 = new Player();
Ball* ball = new Ball();

bool collide(Ball* ball, Player* player){
    float x_distance = abs(ball->getX() - player->getX()) - (ball->getWidth() + player->getWidth())/2;
    float y_distance = abs(ball->getY() - player->getY()) - (ball->getHeight() + player->getHeight())/2;
    std::cout << "x distance: " + std::to_string(x_distance) + " ; y distance : " + std::to_string(y_distance)<< std::endl;
    if(x_distance < 0 && y_distance < ball->getWidth()){
        return true;
    }
    return false;
}

void clamp(Ball* ball){
    if(ball->getY() + ball->getWidth()/2 > openGL_height || ball->getY() + ball->getWidth()/2 < openGL_height * -1){
        ball->reverseYVelocity();
    }
}

int winCondition(Ball *ball){
    //1 = left side wins
    //0 = no one
    // -1 = right wins
    if( ball->getX() > openGL_width + ball->getWidth()){ //goes off the right side; left wins
        return 1;
    }
    else if(ball->getX() < -1 * openGL_width - ball->getWidth()){
        //right wins
        return -1;
    }
    else{
        return 0;
    }
}


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
                player1->move(5.0f);
            }
            if(event.key.keysym.scancode == SDL_SCANCODE_S) {
                player1->move(-5.0f);
            }
            
            //Player 2
            if(event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
                player2->move(-5.0f);
            }
            if(event.key.keysym.scancode == SDL_SCANCODE_UP) {
                player2->move(5.0f);
            }
        }
    }
}

void Update(){
    //Timer:
    float ticks = (float)SDL_GetTicks()/1000.0f;
    elapsed = ticks - lastFrameTicks;
    lastFrameTicks = ticks;
    
    std::cout << elapsed << std::endl;
    
    //Ball Update
    ball->updateBall();
    //Collision Detection
    if(collide(ball, player1) || collide(ball, player2)){
        ball->reverseVelocity();
    }
    //Clamp
    clamp(ball);
    //Check win Condition
    if(winCondition(ball) == -1 || winCondition(ball) == 1){
        winState = winCondition(ball);
        ball->resetBall();
    }
    //Clear Buffer
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer(){
    //1 = left side wins
    //0 = no one
    // -1 = right wins
    
    if(winState == 0){
        player1->Draw(program);
        player2->Draw(program);
        ball->Draw(program);
    }
    if(winState == 1){//left wins
        program.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        player2->Draw(program);
        ball->Draw(program);
        program.SetColor(0.0f, 1.0f, 0.0f, 1.0f);
        player1->Draw(program);
    }
    else if(winState == -1){//left wins
        program.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        player1->Draw(program);
        ball->Draw(program);
        program.SetColor(0.0f, 1.0f, 0.0f, 1.0f);
        player2->Draw(program);
    }

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
