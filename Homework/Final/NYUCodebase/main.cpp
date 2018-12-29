 #ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <vector>
#include "ShaderProgram.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "FlareMap.h"
#include "Enemy.hpp"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
#include "Entity.hpp"

#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6

//GLobals
bool done = false;
SDL_Event event;

glm::mat4 projectionMatrix = glm::mat4(1.0f);
glm::mat4 modelMatrix = glm::mat4(1.0f);
glm::mat4 viewMatrix = glm::mat4(1.0f);

ShaderProgram untexteredShader;

GLuint spriteSheetTexture;
ShaderProgram texteredShader;
std::vector<Entity> entities;
std::vector<Enemy> enemies;

Mix_Chunk *hurt;
int health = 3;
float startX = 0;
float startY = 0;
float startX2 = 0;
float startY2 = 0;
float startX3 = 0;
float startY3 = 0;
int score = 0;
float hurtTime = 1.0;
int level = 1;
int pID = 80;
float playermotion = .1;
bool changeLevel = false;
GLuint fontTexture;
enum GameMode { STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_GAME_OVER};
GameMode mode = STATE_MAIN_MENU;
std::vector<Entity> respawn;
std::vector<float> vertexData;
std::vector<float> texCoordData;
//Sprite
int sc_x = 16;
int sc_y = 8;
float tilesize = .1;
float scale = .1;
FlareMap map;
std::vector<int> solids = {17, 33, 34, 35, 32, 1, 3, 100, 101};

//Physics
float gravity = .017f;
bool canJump = false;
float jumpPower = .9;
int jumps = 2;

//Time
float elapsed;
float accumulator = 0.0f;
float lastFrameTicks = 0.0f;

SDL_Window* displayWindow;

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
void motionPlayer(){
    if(pID == 80){
        pID = 81;
    }
    else{
        pID = 80;
    }
    entities[0].sprite.u = (float)(((int)pID) % sc_x) / (float) sc_x;
    entities[0].sprite.v = (float)(((int)pID) / sc_x) / (float) sc_y;
}
void DrawText(ShaderProgram &program, int fontTexture, std::string text, float x, float y, float size, float spacing) {
    float texture_size = 1.0 / 16.0f;
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    glm::mat4 textModelMatrix = glm::mat4(1.0f);
    textModelMatrix = glm::translate(textModelMatrix, glm::vec3(x, y, 1.0f));
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
    glUseProgram(program.programID);
    program.SetModelMatrix(textModelMatrix);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program.positionAttribute);
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program.texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);
    
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
}
void renderEntities(){
    for(FlareMapEntity e: map.entities){
        if(e.type == "enemy"){
            float u = (float)((99) % sc_x) / (float) sc_x;
            float v = (float)((99) / sc_x) / (float) sc_y;
            float spriteWidth = 1.0f/(float)sc_x;
            float spriteHeight = 1.0f/(float)sc_y;
            SheetSprite enemy = SheetSprite(spriteSheetTexture,u, v,spriteWidth, spriteHeight, tilesize);
            enemies.push_back(Enemy(e.x*tilesize,e.y*-tilesize, .07,0,enemy.width,enemy.height, u, v, enemy.textureID, enemy.size, &map, &solids, &entities[0]));
        }
        else if(e.type == "end"){
            float u = (float)((38) % sc_x) / (float) sc_x;
            float v = (float)((38) / sc_x) / (float) sc_y;
            float spriteWidth = 1.0f/(float)sc_x;
            float spriteHeight = 1.0f/(float)sc_y;
            SheetSprite powerSprite = SheetSprite(spriteSheetTexture,u, v,spriteWidth, spriteHeight, tilesize);
            respawn.push_back(Entity(e.x*tilesize,e.y*-tilesize,0,0,powerSprite.width,powerSprite.height,0,0,0,powerSprite.u,powerSprite.v,powerSprite.textureID, powerSprite.size));
        }
        else if(e.type == "start"){
            startX = e.x;
            startY = e.y;
            
        }
        else if(e.type == "start2"){
            startX2 = e.x;
            startY2 = e.y;
            
        }
        else if(e.type == "start3"){
            startX3 = e.x;
            startY3 = e.y;
            
        }
    }
}
void drawMap(){
    
    glUseProgram(texteredShader.programID);
    glm::mat4 mapModelMatrix = glm::mat4(1.0);
    texteredShader.SetModelMatrix(mapModelMatrix);
    glEnable(GL_BLEND);
    
    glVertexAttribPointer(texteredShader.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(texteredShader.positionAttribute);
    glVertexAttribPointer(texteredShader.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(texteredShader.texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, spriteSheetTexture);
    glDrawArrays(GL_TRIANGLES, 0, vertexData.size()/2);
    
    glDisableVertexAttribArray(texteredShader.positionAttribute);
    glDisableVertexAttribArray(texteredShader.texCoordAttribute);
}
void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY) {
    *gridX = (int)(worldX / tilesize);
    *gridY = (int)(worldY / -tilesize);
}
bool playerCollideBottom(){
    int gridX =0;
    int gridY = 0;
    
    gridX = (int)(entities[0].position.x / tilesize);
    gridY = (int)((entities[0].position.y - (tilesize/ 2)) / -tilesize);
    if(gridX < map.mapWidth && gridY < map.mapHeight){
        for(int solidBlocks: solids){
            if(map.mapData[gridY][gridX] == solidBlocks){
                if(map.mapData[gridY][gridX] == 100 || map.mapData[gridY][gridX] == 101){
                    if(hurtTime<0){
                        health--;
                        hurtTime = 3;
                        Mix_PlayChannel( -1, hurt, 0);
                    }
                }
                entities[0].collidedBottom = true;
                entities[0].position.y += fabs((-tilesize * gridY) - (entities[0].position.y - tilesize/2))+.00001;
                jumps = 2;
                return true;
            }
        }
    }
    entities[0].collidedBottom = false;
    return false;
}
bool playerCollideTop(){
    int gridX =0;
    int gridY = 0;
    
    gridX = (int)(entities[0].position.x / tilesize);
    gridY = (int)((entities[0].position.y + (tilesize / 2)) / -tilesize);
    if(gridX < map.mapWidth && gridY < map.mapHeight){
        for(int solidBlocks: solids){
            if(map.mapData[gridY][gridX] == solidBlocks){
                entities[0].position.y -= fabs(((-tilesize * gridY) -tilesize) - (entities[0].position.y + tilesize/2))+.001;
                return true;
            }
        }
    }
    entities[0].collidedBottom = false;
    return false;
}
bool playerCollideLeft(){
    int gridX =0;
    int gridY = 0;
    
    gridX = (int)((entities[0].position.x - (tilesize / 2))/ tilesize);
    gridY = (int)(entities[0].position.y / -tilesize);
    if(gridX < map.mapWidth && gridY < map.mapHeight){
        for(int solidBlocks: solids){
            if(map.mapData[gridY][gridX] == solidBlocks){
                entities[0].position.x += fabs(((tilesize * gridX) +tilesize) - (entities[0].position.x - tilesize/2))+.001;
                return true;
            }
        }
    }
    entities[0].collidedBottom = false;
    return false;
}
bool playerCollideRight(){
    int gridX =0;
    int gridY = 0;
    
    gridX = (int)((entities[0].position.x + (tilesize / 2))/ tilesize);
    gridY = (int)(entities[0].position.y / -tilesize);
    if(gridX < map.mapWidth && gridY < map.mapHeight){
        for(int solidBlocks: solids){
            if(map.mapData[gridY][gridX] == solidBlocks){
                entities[0].position.x -= fabs(((tilesize * gridX)) - (entities[0].position.x + tilesize/2))+.001;
                return true;
            }
        }
    }
    return false;
}
bool eCollisionBottom(Entity& e){
    int gridX =0;
    int gridY = 0;
    
    gridX = (int)(e.position.x / tilesize);
    gridY = (int)((e.position.y - (tilesize/ 2)) / -tilesize);
    if(gridX < map.mapWidth && gridY < map.mapHeight){
        for(int solidBlocks: solids){
            if(map.mapData[gridY][gridX] == solidBlocks){
                e.collidedBottom = true;
                e.position.y += fabs((-tilesize * gridY) - (e.position.y - tilesize/2))+.00001;
                return true;
            }
        }
    }
    e.collidedBottom = false;
    return false;
}
bool eCollisionTop(Entity& e){
    int gridX =0;
    int gridY = 0;
    gridX = (int)(e.position.x / tilesize);
    gridY = (int)((e.position.y + (tilesize / 2)) / -tilesize);
    if(gridX < map.mapWidth && gridY < map.mapHeight){
        for(int solidBlocks: solids){
            if(map.mapData[gridY][gridX] == solidBlocks){
                e.collidedTop = true;
                e.position.y -= fabs(((-tilesize * gridY) -tilesize) - (e.position.y + tilesize/2))+.001;
                return true;
            }
        }
    }
    e.collidedTop = false;
    return false;
}

bool eCollisionLeft(Entity& e){
    int gridX =0;
    int gridY = 0;
    gridX = (int)((e.position.x - (tilesize / 2))/ tilesize);
    gridY = (int)(e.position.y / -tilesize);
    if(gridX < map.mapWidth && gridY < map.mapHeight){
        for(int solidBlocks: solids){
            if(map.mapData[gridY][gridX] == solidBlocks){
                e.collidedLeft = true;
                e.position.x += fabs(((tilesize * gridX) +tilesize) - (e.position.x - tilesize/2))+.001;
                return true;
            }
        }
    }
    e.collidedLeft = false;
    return false;
}

bool eCollisionRight(Entity& e){
    int gridX =0;
    int gridY = 0;
    gridX = (int)((e.position.x + (tilesize / 2))/ tilesize);
    gridY = (int)(e.position.y / -tilesize);
    if(gridX < map.mapWidth && gridY < map.mapHeight){
        for(int solidBlocks: solids){
            if(map.mapData[gridY][gridX] == solidBlocks){
                e.collidedRight = true;
                e.position.x -= fabs(((tilesize * gridX)) - (e.position.x + tilesize/2))+.001;
                return true;
            }
        }
    }
    e.collidedRight = false;
    return false;
}

class MainMenu {
public:
    void Render() {
        DrawText(texteredShader, fontTexture, "derp derp jump - Justin Lin", -1.12,0,0.05, .01);
    }
    void Update() {
        glClear(GL_COLOR_BUFFER_BIT);
    }
    void ProcessEvents(){
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
            else if (event.type == SDL_KEYDOWN){
                if(event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                    mode = STATE_GAME_LEVEL;
                    lastFrameTicks = (float)SDL_GetTicks()/1000.0f;
                }
            }
        }
    }
};
    

class Game{
    public:
    
    void Setup(){
        SDL_Init(SDL_INIT_VIDEO);
        displayWindow = SDL_CreateWindow("Derp Derp", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
        SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
        SDL_GL_MakeCurrent(displayWindow, context);
        #ifdef _WINDOWS
                glewInit();
        #endif
        glViewport(0, 0, 1280, 720);
        projectionMatrix = glm::ortho(-1.77f,1.77f, -1.0f, 1.0f, -1.0f, 1.0f); //Sets up orthographic projection.Ratio of 16:9
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 4096 );
        texteredShader.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
        untexteredShader.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
        untexteredShader.SetViewMatrix(viewMatrix);
        untexteredShader.SetProjectionMatrix(projectionMatrix);
        untexteredShader.SetModelMatrix(modelMatrix);
        texteredShader.SetViewMatrix(viewMatrix);
        texteredShader.SetProjectionMatrix(projectionMatrix);
        texteredShader.SetModelMatrix(modelMatrix);
        
        spriteSheetTexture = LoadTexture(RESOURCE_FOLDER"sheet.png");
        glBindTexture(GL_TEXTURE_2D, spriteSheetTexture);
        fontTexture = LoadTexture(RESOURCE_FOLDER"pixel_font.png");

        map.Load(RESOURCE_FOLDER"untitled3.txt");
                for(int y=0; y < map.mapHeight; y++) {
                    for(int x=0; x < map.mapWidth; x++) {
                        if(map.mapData[y][x] != 0){
                            float u = (float)(((int)map.mapData[y][x]) % sc_x) / (float) sc_x;
                            float v = (float)(((int)map.mapData[y][x]) / sc_x) / (float) sc_y;
                            float spriteWidth = 1.0f/(float)sc_x;
                            float spriteHeight = 1.0f/(float)sc_y;
                            vertexData.insert(vertexData.end(), {
                                tilesize * x, -tilesize * y,
                                tilesize * x, (-tilesize * y)-tilesize,
                                (tilesize * x)+tilesize, (-tilesize * y)-tilesize,
                                tilesize * x, -tilesize * y,
                                (tilesize * x)+tilesize, (-tilesize * y)-tilesize,
                                (tilesize * x)+tilesize, -tilesize * y
                            });
                            texCoordData.insert(texCoordData.end(), {
                                u, v,
                                u, v+(spriteHeight),
                                u+spriteWidth, v+(spriteHeight),
                                u, v,
                                u+spriteWidth, v+(spriteHeight),
                                u+spriteWidth, v
                            });
                        }
                    }
                }
    }
    void ProcessEvents(){
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
            else if (event.type == SDL_KEYDOWN){
                if(event.key.keysym.scancode == SDL_SCANCODE_SPACE &&  jumps > 0) {
                    entities[0].position.y+=.02;
                    entities[0].velocity.y = jumpPower;
                    jumps--;
                }
                if(event.key.keysym.scancode == SDL_SCANCODE_O) {
                    if(level == 1){
                        level = 2;
                    }
                    else if(level == 2){
                        level = 3;
                    }
                    else if(level == 3){
                        level = 1;
                    }
                    changeLevel = true;
                }
            }
        }
    }
    
    void Update(float elapsedUpdate = elapsed){
        glClear(GL_COLOR_BUFFER_BIT);
        //Level Load if need:
        float u = (float)(((int)80) % sc_x) / (float) sc_x;
        float v = (float)(((int)80) / sc_x) / (float) sc_y;
        float spriteWidth = 1.0f/(float)sc_x;
        float spriteHeight = 1.0f/(float)sc_y;
        SheetSprite myS = SheetSprite(spriteSheetTexture,u, v,spriteWidth, spriteHeight, tilesize);
        if(changeLevel){
            //level++
            if(level == 1){
                entities[0] = Entity(startX*tilesize,startY*(-tilesize),0,0,myS.width,myS.height,0,0,0,myS.u,myS.v,myS.textureID, myS.size);
            }
            if(level == 2){
                entities[0] = Entity(startX2*tilesize,startY2*(-tilesize),0,0,myS.width,myS.height,0,0,0,myS.u,myS.v,myS.textureID, myS.size);
            }
            if(level == 3){
                entities[0] = Entity(startX3*tilesize,startY3*(-tilesize),0,0,myS.width,myS.height,0,0,0,myS.u,myS.v,myS.textureID, myS.size);
            }
            changeLevel = false;
        }

        //Movement
        modelMatrix = glm::mat4(1.0f);
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        if(keys[SDL_SCANCODE_A]) {
            entities[0].velocity.x = -.6;
            playermotion -= elapsedUpdate;
            if(playermotion < 0){
                motionPlayer();
                playermotion = .2;
            }
        } else if(keys[SDL_SCANCODE_D]) {
            entities[0].velocity.x = .6;
            playermotion -= elapsedUpdate;
            if(playermotion < 0){
                motionPlayer();
                playermotion = .2;
            }
        }
        else{
            entities[0].velocity.x = 0;
        }
        
        entities[0].velocity.y -= gravity;
        if(playerCollideBottom()||playerCollideTop()){
            entities[0].velocity.y = 0;
        }
        playerCollideLeft();
        playerCollideRight();
        
        for(Enemy& e: enemies){
            e.velocity.y -= gravity;
            if(eCollisionBottom(e)||eCollisionTop(e)){
                e.velocity.y = 0;
            }
            eCollisionLeft(e);
            eCollisionRight(e);
        }
        
        for(Entity& respawn: respawn){
            if(entities[0].collision(respawn)){
                level++;
                changeLevel = true;

                }
                
        }
        entities[0].update(elapsedUpdate);
        for(Enemy& e: enemies){
            if(e.collision() && hurtTime < 0){
                health--;
                hurtTime = 1.0;
                e.position.x=-10000;
                Mix_PlayChannel( -1, hurt, 0);

            }
            e.update(elapsedUpdate);
        }
        hurtTime -= elapsedUpdate;

        
        viewMatrix = glm::mat4(1.0f);
        viewMatrix = glm::translate(viewMatrix, glm::vec3(-entities[0].position.x,-entities[0].position.y,0.0f));
        texteredShader.SetViewMatrix(viewMatrix);
        if(health <= 0){
             mode = STATE_GAME_OVER;
        }
        
    }
    void Render(){
        drawMap();
        for(Entity& e: entities){
            e.Draw(texteredShader, elapsed);
        }
        for(Entity& respawn: respawn){
            respawn.Draw(texteredShader, elapsed);
        }
        for(Enemy& enemy: enemies){
            enemy.Draw(texteredShader, elapsed);
        }
        DrawText(texteredShader, fontTexture, "Health: "+ std::to_string(health), entities[0].position.x-1.7,entities[0].position.y+.9,.05, .01);

    }
};

class GameOver {
public:
    void Render() {
        DrawText(texteredShader, fontTexture, "Game Over!", entities[0].position.x-.4,entities[0].position.y,0.05, .01);
    }
    void Update() {
        glClear(GL_COLOR_BUFFER_BIT);
    }
    void ProcessEvents(){
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
            else if (event.type == SDL_KEYDOWN){
                if(event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                    mode = STATE_GAME_LEVEL;
                    lastFrameTicks = (float)SDL_GetTicks()/1000.0f;
                }
                
            }
        }
    }
};

MainMenu menu;
Game game;
GameOver gameOver;


void Setup(){
    game.Setup();
}
void ProcessEvents(){
    switch(mode){
        case STATE_MAIN_MENU:
            menu.ProcessEvents();
            break;
        case STATE_GAME_LEVEL:
            game.ProcessEvents();
            break;
        case STATE_GAME_OVER:
            gameOver.ProcessEvents();
            break;
    }
}
void Update(float elapsedUpdate = elapsed){
    switch(mode){
        case STATE_MAIN_MENU:
            menu.Update();
            break;
        case STATE_GAME_LEVEL:
            game.Update(elapsedUpdate);
            break;
        case STATE_GAME_OVER:
            gameOver.Update();
            break;
    }
}
void Render(){
    switch(mode){
        case STATE_MAIN_MENU:
            menu.Render();
            break;
        case STATE_GAME_LEVEL:
            game.Render();
            break;
        case STATE_GAME_OVER:
            gameOver.Render();
            break;
    }
}

int main(int argc, char *argv[])
{
    
    Setup();
    
    float u = (float)(((int)80) % sc_x) / (float) sc_x;
    float v = (float)(((int)80) / sc_x) / (float) sc_y;
    float spriteWidth = 1.0f/(float)sc_x;
    float spriteHeight = 1.0f/(float)sc_y;
    SheetSprite myS = SheetSprite(spriteSheetTexture,u, v,spriteWidth, spriteHeight, tilesize);
    entities.push_back(Entity(0*tilesize,0*(-tilesize),0,0,myS.width,myS.height,0,0,0,myS.u,myS.v,myS.textureID, myS.size));
    renderEntities();
    entities[0] = Entity(startX*tilesize,startY*(-tilesize),0,0,myS.width,myS.height,0,0,0,myS.u,myS.v,myS.textureID, myS.size);
    
    //Enemy
    u = (float)(((int)102) % sc_x) / (float) sc_x;
    v = (float)(((int)102) / sc_x) / (float) sc_y;

    u = (float)(((int)52) % sc_x) / (float) sc_x;
    v = (float)(((int)52) / sc_x) / (float) sc_y;
    
    Mix_Music *music;
    music = Mix_LoadMUS("epic.mp3");
    hurt = Mix_LoadWAV("hurt.wav");
    
    Mix_VolumeMusic(30);
    Mix_PlayMusic(music, -1);
    
    while (!done) {
        float ticks = (float)SDL_GetTicks()/1000.0f;
        elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        elapsed += accumulator;
        if(elapsed < FIXED_TIMESTEP) {
            accumulator = elapsed;
            continue; }
        while(elapsed >= FIXED_TIMESTEP) {
            Update(FIXED_TIMESTEP);
            elapsed -= FIXED_TIMESTEP;
        }
        accumulator = elapsed;
        ProcessEvents();
        Update();
        Render();
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}
