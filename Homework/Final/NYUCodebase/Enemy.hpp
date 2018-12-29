#pragma once
#ifndef Enemy_hpp
#define Enemy_hpp

#include <stdio.h>
#include "ShaderProgram.h"
#include "SheetSprite.hpp"
#include "Entity.hpp"
#include "FlareMap.h"
#include <vector>
class Enemy : public Entity{
    int state = 0;
    std::vector<int>* solids;
    FlareMap* map;
    Entity* player;
public:
    Enemy(float x, float y, float velocity_x, float velocity_y, float width, float height, float u, float v, int textureID, float size, FlareMap* map, std::vector<int>* solids, Entity* player);
    void checkState();
    void update(float elapsed);
    bool collision();
    int travelDirection = 1;
    void testingPoints();
    std::vector<bool> collidePoint {false, false, false, false};
};

#endif

