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

/*
 •
 •
 • • •
 Create a simple 2D scene using textured and untextured polygons.
 You can use any images you want, but feel free to use the assets in the class github repo.
 At least one element must be animated.
 You must use at least 3 different textures.
 Commit the source to your github repository and email me the link.
 */

SDL_Window* displayWindow;
             
const float width = 640;
const float height = 360;

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

int main(int argc, char *argv[])
{
    
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
    glm::mat4 projectionMatrix5 = glm::mat4(1.0f);
    glm::mat4 modelMatrix5 = glm::mat4(1.0f);
    glm::mat4 viewMatrix5 = glm::mat4(1.0f);
    projectionMatrix5 = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
    
    glViewport(0, 0, width, height);
    ShaderProgram program;
    
    //LOADING TEXTURES
    program.Load(RESOURCE_FOLDER "vertex_textured.glsl", RESOURCE_FOLDER "fragment_textured.glsl");
    
    ShaderProgram program2;
    program2.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
    
    GLuint emojiTexture = LoadTexture(RESOURCE_FOLDER"smile.png");
    GLuint pikaTexture = LoadTexture(RESOURCE_FOLDER"Pikachu.png");
    GLuint pokeballTexture = LoadTexture(RESOURCE_FOLDER"pokeball.png");
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    float lastFrameTicks = 0.0f;
    
    //Initializing the Model Matrix
    projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-1.0f, 0.0f, 0.0f));
    
    //Initializing model Matrix 2
    glm::mat4 modelMatrix2 = glm::mat4(1.0f);
    modelMatrix2 = glm::translate(modelMatrix2, glm::vec3(1.0f, 0.0f, 0.0f));

    //Matrix 3
    //Initializing model Matrix 3
    glm::mat4 modelMatrix3 = glm::mat4(1.0f);
    modelMatrix3 = glm::translate(modelMatrix3, glm::vec3(0.0f, 0.0f, 0.0f));
    
    glUseProgram(program2.programID);
    glUseProgram(program.programID);

#ifdef _WINDOWS
    glewInit();
#endif

    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
        glClear(GL_COLOR_BUFFER_BIT);
        //Working on the first model Matrix
        program.SetModelMatrix(modelMatrix);
        program.SetProjectionMatrix(projectionMatrix);
        program.SetViewMatrix(viewMatrix);
        //Drawing Object
        glBindTexture(GL_TEXTURE_2D, emojiTexture);
        //Square
        float vertices[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        //First object
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        float texCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        float angle = 10.0f * (3.1415926f / 180.0f);
        modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0.0f, 0.0f, 1.0f));
        
        //Working on Model Matrix 2
        program.SetModelMatrix(modelMatrix2);
        glBindTexture(GL_TEXTURE_2D, pikaTexture);
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        //Working on Model Matrix 3
        program.SetModelMatrix(modelMatrix3);
        glBindTexture(GL_TEXTURE_2D, pokeballTexture);
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
         
        
        //Working on Model Matrix 4
        
        program2.SetModelMatrix(modelMatrix5);
        program2.SetProjectionMatrix(projectionMatrix5);
        program2.SetViewMatrix(viewMatrix5);
        float vertices2[] = {0.5f, -0.5f, 0.0f, 0.5f, -0.5f, -0.5f};
        glVertexAttribPointer(program2.positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
        glEnableVertexAttribArray(program2.positionAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glDisableVertexAttribArray(program2.positionAttribute);
        
        SDL_GL_SwapWindow(displayWindow);


    }
    
    SDL_Quit();
    return 0;
}
