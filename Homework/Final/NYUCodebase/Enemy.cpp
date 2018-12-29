#include "Enemy.hpp"
#include "ShaderProgram.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Entity.hpp"
#include "FlareMap.h"
#include <vector>

Enemy::Enemy(float x, float y, float velocity_x, float velocity_y, float width, float height, float u, float v, int textureID, float size, FlareMap* map, std::vector<int>* Solids, Entity* player): Entity(x, y, velocity_x, velocity_y, width, height , 1, 1, 1,u, v, textureID, size), map(map), solids(Solids), player(player){}
void Enemy::testingPoints(){
    collidePoint = {false, false, false, false};
    int gridX =0;
    int gridY = 0;

    gridX = (int)((position.x -.01 - (sprite.size/ 2)) / sprite.size);
    gridY = (int)((position.y -.01 - (sprite.size/ 2)) / -sprite.size);
    if(gridX < map->mapWidth && gridY < map->mapHeight){
        for(int solidID: *solids){
            if(map->mapData[gridY][gridX] == solidID){
                collidePoint[0] = true;
                break;
            }
            
        }
    }
    
    gridX = (int)((position.x +.01 + (sprite.size/2)) / sprite.size);
    gridY = (int)((position.y  - .001 - (sprite.size/ 2)) / -sprite.size);
    if(gridX < map->mapWidth && gridY < map->mapHeight){
        for(int solidID: *solids){
            if(map->mapData[gridY][gridX] == solidID){
                collidePoint[1] = true;
                break;
            }
            
        }
    }
    
    gridX = (int)((position.x -.05 - (sprite.size/2)) / sprite.size);
    gridY = (int)((position.y + .01 + (sprite.size/ 2)) / -sprite.size);
    if(gridX < map->mapWidth && gridY < map->mapHeight){
        for(int solidID: *solids){
            if(map->mapData[gridY][gridX] == solidID){
                collidePoint[2] = true;
                break;
            }
            
        }
    }
    
    gridX = (int)((position.x +.01 + (sprite.size/2)) / sprite.size);
    gridY = (int)((position.y + .01 + (sprite.size/ 2)) / -sprite.size);
    if(gridX < map->mapWidth && gridY < map->mapHeight){
        for(int solidID: *solids){
            if(map->mapData[gridY][gridX] == solidID){
                collidePoint[3] = true;
                break;
            }
            
        }
    }
}
void Enemy::update(float elapsed){
    testingPoints();
    checkState();
    if(state == 0){
        if(travelDirection==1){
            if(collidePoint[1] == false || collidedRight){
                travelDirection = -1;
                velocity.x *=-1;
            }
        }
        else if(travelDirection==-1){
            if(collidePoint[0]==false  || collidedLeft){
                travelDirection = 1;
                velocity.x *=-1;
            }
        }
        
    }
    else{
        if(player->position.x < position.x){
            travelDirection = -1;
            if(velocity.x>0){
                velocity.x *= -1;
            }
        }
        else{
            travelDirection = 1;
            if(velocity.x<0){
                velocity.x *= -1;
            }
        }
        if(travelDirection==1){
            if(collidePoint[1] == false){
                travelDirection = -1;
                velocity.x *=-1;
            }
        }
        else if(travelDirection==-1){
            if(collidePoint[0]==false){
                travelDirection = 1;
                velocity.x *=-1;
            }
        }
    }
    position.x += velocity.x *elapsed;
    position.y += velocity.y * elapsed;
}
bool Enemy::collision(){
    float x_distance = abs(player->position.x-position.x)-((player->sprite.size+sprite.size)/2);
    float y_distance = abs(player->position.y-position.y)-((player->sprite.size+sprite.size)/2);
    if(x_distance< 0 && y_distance < 0){
        return true;
    }
    return false;
}
void Enemy::checkState(){
    if(abs(player->position.y-position.y) <= .05&&abs(player->position.x-position.x)<=.3){
        state = 1;
        velocity.x = .13 * travelDirection;
    }
    else {
        state = 0;
        velocity.x = .07 * travelDirection;
    }
}

