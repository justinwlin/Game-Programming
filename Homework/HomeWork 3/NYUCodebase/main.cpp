#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
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


/*
 ===========================
 Global Variables
 ===========================
 */
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
float vertices[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
float texCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};

/*
 ===========================
 Textures
 ===========================
 */
GLuint spriteSheetTexture;
GLuint defaultTexture;
GLuint playerTexture;
GLuint enemyTexture;
GLuint bulletTexture;

/*
 ===========================
 Load Texture
 ===========================
 */
GLuint LoadTexture(const char *filePath) {
    int w,h,comp;
    unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
    if(image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    GLuint retTexture;
    glGenTextures(1, &retTexture);
    glBindTexture(GL_TEXTURE_2D, retTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(image);
    
    return retTexture;
}

void DrawText(ShaderProgram* program, int fontTexture, std::string text, float size, float spacing) {
    float texture_size = 1.0 / 16.0f;
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    
    for (size_t i = 0; i < text.size(); i++) {
        float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
        float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
        vertexData.insert(vertexData.end(), {
            ((size + spacing) * i) + (-0.5f * size), 0.5f * size,
            ((size + spacing) * i) + (-0.5f * size), -0.5f * size,
            ((size + spacing) * i) + (0.5f * size), 0.5f * size,
            ((size + spacing) * i) + (0.5f * size), -0.5f * size,
            ((size + spacing) * i) + (0.5f * size), 0.5f * size,
            ((size + spacing) * i) + (-0.5f * size), -0.5f * size,
        });
        texCoordData.insert(texCoordData.end(), {
            texture_x, texture_y,
            texture_x, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x + texture_size, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x, texture_y + texture_size,
        });
    }
    glUseProgram(program->programID);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

/*
 ===========================
 Entity
 ===========================
 */
class Entity{
public:
    float u;
    float v;
    int type;
    float x = 0;
    float y = 0;
    float x_scale;
    float y_scale;
    float rotation;
    
    GLuint entity_texture;
    
    float width = 1;
    float height = 1;
    
    float velocity;
    float direction_x;
    float direction_y;
    float size = 1.0f;
    
    Entity(){}
    
    Entity(int input_type){
        type = input_type;
        if(type == 1){//Player
            x_scale = 1;
            y_scale = 1;
            y = -.8;
            
            u = 224.0f / 1024.0f;
            v = 832.0f / 1024.0f;
            width = 99.0f / 1024.0f;
            height = 75.0f / 1024.0f;
            entity_texture = defaultTexture;
        }
        else if(type == 2){//Enemy
            width = 1;
            height = 1;
            x_scale = .25;
            y_scale = .1;
            entity_texture = enemyTexture;
        }
        else{//Bullet
        }

    };
    
    void Draw(ShaderProgram &p){
        glm::mat4 newMatrix = glm::mat4(1.0f);
        newMatrix = glm::translate(newMatrix, glm::vec3(x, y, 1.0f));
        newMatrix = glm::scale(newMatrix, glm::vec3(x_scale, y_scale, 1.0f));
        p.SetModelMatrix(newMatrix);

        std::vector<float> vertexData;
        std::vector<float> texCoordData;
        float texture_x = u;
        float texture_y = v;
        vertexData.insert(vertexData.end(), {
            (-0.1f * size), 0.1f * size,
            (-0.1f * size), -0.1f * size,
            (0.1f * size), 0.1f * size,
            (0.1f * size), -0.1f * size,
            (0.1f * size), 0.1f * size,
            (-0.1f * size), -0.1f * size,
        });
        texCoordData.insert(texCoordData.end(), {
            texture_x, texture_y,
            texture_x, texture_y + height,
            texture_x + width, texture_y,
            texture_x + width, texture_y + height,
            texture_x + width, texture_y,
            texture_x, texture_y + height,
        });
        
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
        glEnableVertexAttribArray(program.positionAttribute);
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
        glEnableVertexAttribArray(program.texCoordAttribute);
        glBindTexture(GL_TEXTURE_2D, spriteSheetTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
    }
    
    void loadTexture(){
        if(type == 1){entity_texture = playerTexture;}
        else if(type == 2){entity_texture = enemyTexture;}
        else{entity_texture = bulletTexture;}
    }
    
    void moveX(float x_input){
        x += x_input;
    }
    
    glm::mat4 player = glm::mat4(1.0f);
    

};

/*
 ===========================
 Main Menu
 ===========================
 */
class MainMenu {
public:
    void Render(ShaderProgram &p) {
        
    }
    void Update() {
    }
    void Process() {
    }
};

/*
 ===========================
 Game Level
 ===========================
 */
class GameLevel {
public:
    GameLevel(){
        float width = -1.3f;
        for(int i = 0; i < 10; i++){
            Entity tempEntity = *new Entity(2);
            tempEntity.x += width;
            std::cout << tempEntity.x << std::endl;
            enemies.push_back(tempEntity);
            width += tempEntity.x_scale + 0.05;
        }
    };
    
    void Render(ShaderProgram &p) {
        for(int i = 0; i < 10; i++){
            enemies[i].Draw(p);
        }
        player.Draw(p);
    }
    void Update() {
        glClear(GL_COLOR_BUFFER_BIT);
    }
    void Process() {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                loop = true;
            }
            if(event.key.keysym.scancode == SDL_SCANCODE_D) {
                player.moveX(0.1f);
            }
            if(event.key.keysym.scancode == SDL_SCANCODE_A) {
                player.moveX(-0.1f);
            }
        }
    }
    std::vector<Entity> enemies;
    Entity player = Entity(1);
    int score;
};



/*
 ===========================
 Functions
 ===========================
 */

/*
 ===========================
 Global Variables
 ===========================
 */
enum GameMode { STATE_MAIN_MENU, STATE_GAME_LEVEL};
GameMode mode;
MainMenu mainMenu;
GameLevel gameLevel;

/*
 ===========================
 Methods
 ===========================
 */

void Startup(){
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    glViewport(0, 0, width, height);
    program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    projectionMatrix = glm::mat4(1.0f);
    viewMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-openGL_width, openGL_width, -openGL_height, openGL_height, -openGL_height, openGL_height);
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(program.programID);
    
    /*
     ===========================
     Initialize Entities
     ===========================
     */
    gameLevel.player.width = 0.1f;
    
    /*
     ===========================
     Load Texture
     ===========================
     */
    spriteSheetTexture = LoadTexture(RESOURCE_FOLDER"sheet.png");
    defaultTexture = LoadTexture(RESOURCE_FOLDER"ball.png");
    playerTexture = LoadTexture(RESOURCE_FOLDER"ball.png");
    enemyTexture = LoadTexture(RESOURCE_FOLDER"ball.png");
    bulletTexture = LoadTexture(RESOURCE_FOLDER"ball.png");

    /*
     ===========================
     Set up GameStates
     ===========================
     */
    mode = STATE_GAME_LEVEL;
    
#ifdef _WINDOWS
    glewInit();
#endif
}

void ProcessEvents(){
    switch(mode) {
        case STATE_MAIN_MENU:
            mainMenu.Process();
            break;
        case STATE_GAME_LEVEL:
            gameLevel.Process();
            break;
    }
}



void Update(){
    switch(mode) {
        case STATE_MAIN_MENU:
            mainMenu.Update();
            break;
        case STATE_GAME_LEVEL:
            gameLevel.Update();
            break;
    }
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer(){
    switch(mode) {
        case STATE_MAIN_MENU:
            mainMenu.Render(program);
            break;
        case STATE_GAME_LEVEL:
            gameLevel.Render(program);
            break;
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
