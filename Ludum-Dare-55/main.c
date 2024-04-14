#define _CRT_SECURE_NO_WARNINGS

#include "raylib.h"
#include "raymath.h"
#include <assert.h>
#include <stdio.h>   
#include <stdlib.h> 
#include <math.h>



//------------------------------------------------------------------------------------
// C Macros
//------------------------------------------------------------------------------------

#define DEBUG_MODE true
#define NULLID -1
#define foreach(item, array) \
    for(int keep = 1, \
            count = 0,\
            size = sizeof (array) / sizeof *(array); \
        keep && count != size; \
        keep = !keep, count++) \
      for(item = (array) + count; keep; keep = !keep)


#define ITERATE(IDX, MAX) (int IDX = 0; IDX < MAX; IDX++)


//------------------------------------------------------------------------------------
// C Utils
//------------------------------------------------------------------------------------

// Dynamic Int Array
// from https://stackoverflow.com/questions/3536153/c-dynamically-growing-array
typedef struct IntArray {
    int* array;
    size_t used;
    size_t size;
} IntArray;

void initIntArray(IntArray * a, size_t initialSize) {
    a->array = malloc(initialSize * sizeof(int));
    a->used = 0;
    a->size = initialSize;
}

void insertIntArray(IntArray * a, int element) {
    if (a->used == a->size) {
        a->size *= 2;
        a->array = realloc(a->array, a->size * sizeof(int));
    }
    a->array[a->used++] = element;
}

void freeIntArray(IntArray * a) {
    free(a->array);
    a->array = NULL;
    a->used = a->size = 0;
}



// Min Max
inline int imin(int i1, int i2) {
    return i1 < i2 ? i1 : i2;
}

inline int imax(int i1, int i2) {
    return i1 > i2 ? i1 : i2;
}

//------------------------------------------------------------------------------------
// C Consts
//------------------------------------------------------------------------------------

#define TILE_SIZE 30

const float MINION_SPEED = 50.0;
const float MINION_ACCELERATION = 5.0;
const Vector2 SCREEN_SIZE = { 900, 18 * 30 };
const float SPAWN_PERIOD = 0.01;


// Colors
const unsigned int GROUND_COLOR = 0xF1BB87FF;
const unsigned int GROUND_COLOR_2 = 0xF2B47AFF;
const unsigned int PLACEABLE_COLOR = 0x2E86ABFF;
const unsigned int PLACEABLE_COLOR_2 = 0x207295FF;
const unsigned int PLAYER_COLOR = 0x2E86ABFF;
const unsigned int ENEMY_COLOR = 0xA4243BFF;

//------------------------------------------------------------------------------------
// C Structs
//------------------------------------------------------------------------------------

typedef struct Entity {
    Vector2 position;
    float height;
    bool isSpawned;
} Entity;

typedef struct Minion {
    Entity entity;
    Vector2 velocity;
    int targetTowerId;
    bool isPlayer;
    bool isTargted;
} Minion;


typedef struct Tower {
    Entity entity;
    int health;
    int value;
    float attackCooldown;
} Tower;

typedef struct Projectile {
    Entity entity;
    Vector2 startPosition;
    Vector2 targetPosition;
    int targetMinionId;
    float aliveTime;
    float totalAliveTime;
} Projectile;

typedef struct EntityClass {
    //void (*spawnCallback);
    //void (*destroyCallback);
    void (*update)(int, float);
    void (*draw)(int);
    void* bank;
    int bankSize;
    int structSize;
    int lastSpawnedId;
} EntityClass;

typedef struct TileData {
    unsigned int type;
    IntArray minionIds;
} TileData;

typedef struct TileMap {
    int width;
    int height;
    TileData* tiles;
} TileMap;



#define MINION_TYPE 0
#define TOWER_TYPE 1
#define PROJECTILE_TYPE 2
#define TYPE_COUNT 3


EntityClass entityClasses[TYPE_COUNT];


//------------------------------------------------------------------------------------
// C Func
//------------------------------------------------------------------------------------

void damageTower(int id, int damageAmount);
void updateMinion(int id, float delta);
void updateTower(int id, float delta);
void updateProjectile(int id, float delta);
void drawMinion(int id);
void drawTower(int id);
void drawProjectile(int id);
Entity* getEntity(int type, int id);
TileData* getTile(TileMap* tileMap, int x, int y);
TileMap loadTileMap(Image* mapImage);
TileData* getTileAt(TileMap* tileMap, Vector2 position);
void getMinionIdsInRange(IntArray* result, TileMap* tileMap, Vector2 position, float radius);
int spawnProjectile(Vector2 startPosition, int targetMinionId, float totalAliveTime);
float calculateProjectileHeight(float timePercent);


//------------------------------------------------------------------------------------
// C EntityClass
//------------------------------------------------------------------------------------

void initClass(int type) {
    EntityClass* entityClass = &entityClasses[type];

    switch(type) {
        case MINION_TYPE:
            entityClass->bankSize = 1000;
            entityClass->structSize = sizeof(Minion);
            entityClass->update = &updateMinion;
            entityClass->draw = &drawMinion;
            break;
        case TOWER_TYPE:
            entityClass->bankSize = 10;
            entityClass->structSize = sizeof(Tower);
            entityClass->update = &updateTower;
            entityClass->draw = &drawTower;
            break;
        case PROJECTILE_TYPE:
            entityClass->bankSize = 1000;
            entityClass->structSize = sizeof(Projectile);
            entityClass->update = &updateProjectile;
            entityClass->draw = &drawProjectile;
            break;
    }
    

    int allocSize = entityClass->bankSize * entityClass->structSize;
    entityClass->bank = malloc(allocSize);
    
    for ITERATE(id, entityClass->bankSize) {
        Entity* entity = getEntity(type, id);
        entity->isSpawned = false;
    }

    entityClass->lastSpawnedId = -1;


}

void destroyClass(int type) {
    EntityClass* entityClass = &entityClasses[type];
    free(entityClass->bank);
}


//------------------------------------------------------------------------------------
// C Entity
//------------------------------------------------------------------------------------

Entity* getEntity(int type, int id) {
    return ((intptr_t)entityClasses[type].bank + id * entityClasses[type].structSize);;
}


int createEntity(int type) {
    EntityClass* entityClass = &entityClasses[type];
    for(int i = (entityClass->lastSpawnedId + 1) % entityClass->bankSize; 
        i != entityClass->lastSpawnedId; i++)
    {
        Entity* entity = getEntity(type, i);
        if (!entity->isSpawned)
        {
            entity->isSpawned = true;
            return i;
        }
    }
    return NULLID;
}


void destroyEntity(int type, int id) {
    Entity* entity = getEntity(type, id);
    entity->isSpawned = false;
}


//------------------------------------------------------------------------------------
// C Vars
//------------------------------------------------------------------------------------

#define MINION_ATTACK_RANGE 4;


bool isMinionTargetRecalculationPending = false;
int minionInventoryCount = 1000;
TileMap currentTileMap;
IntArray minionIdsInRange;

const int spawnDeltaDis = 10;
Vector2 lastSpawnPoint;






//------------------------------------------------------------------------------------
// C Utils
//------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------
// C Minions
//------------------------------------------------------------------------------------


void updateMinion(int id, float delta) {
    Minion* minion = getEntity(MINION_TYPE, id);

    if (isMinionTargetRecalculationPending) {
        minion->targetTowerId = calculateMinionTarget(id);
    }

    bool inRange = minion->targetTowerId != NULLID
        && Vector2Distance(minion->entity.position, getEntity(TOWER_TYPE, minion->targetTowerId)->position) < MINION_ATTACK_RANGE;
    // UPDATE VELOCITY
    if (minion->targetTowerId != NULLID && !inRange) {
        Entity* targetEntity = getEntity(TOWER_TYPE, minion->targetTowerId);
        Vector2 moveDirection = Vector2Normalize(
            Vector2Subtract(targetEntity->position, minion->entity.position)
        );

        minion->velocity = Vector2Scale(moveDirection, MINION_SPEED);
    }
    else {
        minion->velocity = Vector2Zero();
    }

    

    // ATTACK
    if (minion->targetTowerId != NULLID && inRange) {
        damageTower(minion->targetTowerId, 1);
        destroyEntity(MINION_TYPE, id);
        return;
    }

    // UPDATE POSITION
    minion->entity.position = Vector2Add(minion->entity.position, Vector2Scale(minion->velocity, delta));
}

void drawMinion(int id) {
    Minion* minion = getEntity(MINION_TYPE, id);
    if (!minion->entity.isSpawned) return;

    Vector2 p = minion->entity.position;
    DrawRectangle(p.x - 5, p.y - 5, 10, 10, RED);
}


int calculateMinionTarget(int id) {
    Minion* minion = getEntity(MINION_TYPE, id);

    bool towerExists = false;
    int closestTowerId = NULLID;
    float sqrDistance = INFINITY;
    for ITERATE(i, entityClasses[TOWER_TYPE].bankSize) {
        Tower* tower = getEntity(TOWER_TYPE, i);
        if (!tower->entity.isSpawned) continue;

        float sqrDistance2 = Vector2DistanceSqr(minion->entity.position, tower->entity.position);
        if (sqrDistance2 < sqrDistance)
        {
            closestTowerId = i;
            sqrDistance = sqrDistance2;
        }
    }

    return closestTowerId;
}


int spawnMinionAt(Vector2 position) {
    
    int id = createEntity(MINION_TYPE);
    if (id == NULLID) return;

    Minion* minion = getEntity(MINION_TYPE, id);

    minion->entity.position = position;
    minion->velocity = (Vector2){ 0, 0 };
    minion->targetTowerId = calculateMinionTarget(id);
    minion->isPlayer = true;
    minion->isTargted = false;

    return id;
}


void getMinionIdsInRange(IntArray* result, TileMap* tileMap, Vector2 position, float radius) {
    int minX = imax((position.x - radius) / TILE_SIZE, 0);
    int maxX = imin((position.x + radius) / TILE_SIZE, tileMap->width - 1);
    int minY = imax((position.y - radius) / TILE_SIZE, 0);
    int maxY = imin((position.y + radius) / TILE_SIZE, tileMap->height - 1);

    float radiusSqr = radius * radius;


    
    result->used = 0;

    for(int x = minX; x <= maxX; x++) {
        for (int y = minY; y <= maxY; y++) {
            TileData* tile = getTile(tileMap, x, y);
            for ITERATE(i, tile->minionIds.used) {
                int id = tile->minionIds.array[i];
                Entity* entity = getEntity(MINION_TYPE, id);
                if (!entity->isSpawned) continue;

                if (Vector2DistanceSqr(entity->position, position) <= radiusSqr) {
                    insertIntArray(result, id);
                }
            }
        }
    }
}


//------------------------------------------------------------------------------------
// C Towers
//------------------------------------------------------------------------------------

#define TOWER_ATTACK_RADIUS 160
#define TOWER_ATTACK_PERIOD 0.2
#define TOWER_PROJECTILE_SPEED 80

int spawnTower(Vector2 position, float health) {
    assert(health > 0);

    int id = createEntity(TOWER_TYPE);
    if (id == NULLID) return;
    
    Tower* tower = getEntity(TOWER_TYPE, id);
    tower->health = health;
    tower->entity.position = position;
    tower->value = health * 2;
    tower->attackCooldown = TOWER_ATTACK_PERIOD;

    isMinionTargetRecalculationPending = true;

    return id;
}

void drawTower(int id) {
    Tower* tower = getEntity(TOWER_TYPE, id);
    DrawCircleV(tower->entity.position, 10, YELLOW);

    char str[8];
    sprintf(str, "%d", (int) ceil(tower->health));
    DrawText(str, tower->entity.position.x, tower->entity.position.y, 12, BLACK);
}

void damageTower(int id, int damageAmount) {
    Tower* tower = getEntity(TOWER_TYPE, id);
    tower->health -= damageAmount;
}



void updateTower(int id, float delta) {
    Tower* tower = getEntity(TOWER_TYPE, id);
    if (tower->health <= 0) {
        minionInventoryCount += tower->value;
        isMinionTargetRecalculationPending = true;
        destroyEntity(TOWER_TYPE, id);
    }

    tower->attackCooldown -= delta;

    if (tower->attackCooldown <= 0) {
        tower->attackCooldown += TOWER_ATTACK_PERIOD;

        getMinionIdsInRange(&minionIdsInRange, &currentTileMap, tower->entity.position, TOWER_ATTACK_RADIUS);

        // Filter to only non targted minions
        int count = minionIdsInRange.used;
        int j = 0;
        for ITERATE(i, count) {
            int i = GetRandomValue(0, minionIdsInRange.used - 1);
            int id = minionIdsInRange.array[i];
            Minion* minion = getEntity(MINION_TYPE, id);

            if (!minion->isTargted) {
                minionIdsInRange.array[j++] = minionIdsInRange.array[i];
            } else {
                minionIdsInRange.used--;
            }
        }

        if (minionIdsInRange.used > 0) {
            int i = GetRandomValue(0, minionIdsInRange.used - 1);
            int id = minionIdsInRange.array[i];
            Minion* minion = getEntity(MINION_TYPE, id);
            float distanceToMinion = Vector2Distance(tower->entity.position, minion->entity.position);
            float attackTime = distanceToMinion / TOWER_PROJECTILE_SPEED;
            minion->isTargted = true;
            spawnProjectile(tower->entity.position, id, max(attackTime, 0.1));
        }
    }

    
}


//------------------------------------------------------------------------------------
// C Projectile
//------------------------------------------------------------------------------------

int spawnProjectile(Vector2 startPosition, int targetMinionId, float totalAliveTime) {
    assert(getEntity(MINION_TYPE, targetMinionId)->isSpawned);

    int id = createEntity(PROJECTILE_TYPE);
    Projectile* projectile = getEntity(PROJECTILE_TYPE, id);

    projectile->startPosition = startPosition;
    projectile->targetMinionId = targetMinionId;
    projectile->aliveTime = 0.0;
    projectile->totalAliveTime = totalAliveTime;
    return id;
}


void drawProjectile(int id) {
    Projectile* projectile = getEntity(PROJECTILE_TYPE, id);
    Vector2 drawPosition = projectile->entity.position;
    drawPosition.y -= projectile->entity.height;
    DrawRectangleV(drawPosition, (Vector2){5, 5}, ORANGE);
}


void updateProjectile(int id, float delta) {
    Projectile* projectile = getEntity(PROJECTILE_TYPE, id);

    // Update target position
    if (projectile->targetMinionId != NULLID) {
        Minion* targetMinion = getEntity(MINION_TYPE, projectile->targetMinionId);
        if (!targetMinion->entity.isSpawned) {
            projectile->targetMinionId = NULLID;
        } else {
            projectile->targetPosition = targetMinion->entity.position;
        }
    }

    // Update alive time
    projectile->aliveTime += delta;
    float totalTime = projectile->totalAliveTime;
    float alivePercentage = projectile->aliveTime / projectile->totalAliveTime;

    if (alivePercentage >= 1.0) {
        if (projectile->targetMinionId != NULLID) {
            destroyEntity(MINION_TYPE, projectile->targetMinionId);
        }
        return destroyEntity(PROJECTILE_TYPE, id);
    }

    // Update position / height
    projectile->entity.position = Vector2Lerp(projectile->startPosition, projectile->targetPosition, alivePercentage);
    projectile->entity.height = calculateProjectileHeight(alivePercentage);

}

float calculateProjectileHeight(float timePercent) {
    static const float startHeight = 10.0;
    static const float peakHeight = 20.0; // this is not actually peak height, but I'm too lazy to make the equation better

    float result = -peakHeight * (timePercent - 1) * (timePercent + startHeight / peakHeight); 

    return result;
}



//------------------------------------------------------------------------------------
// C TileMap
//------------------------------------------------------------------------------------





#define GROUND_TILE 0xffffffff
#define PLACEABLE_TILE 0x3294c4ff


TileMap loadTileMap(Image* mapImage) {
    TileMap tileMap;
    tileMap.width = mapImage->width;
    tileMap.height = mapImage->height;

    tileMap.tiles = malloc(sizeof(TileData) * tileMap.width * tileMap.height);

    for ITERATE(x, mapImage->width) {
        for ITERATE(y, mapImage->height) {
            TileData* tileData = getTile(&tileMap, x, y);

            Color color = GetImageColor(*mapImage, x, y);
            tileData->type = ColorToInt(color);
            initIntArray(& (tileData->minionIds), 1);

            Vector2 position = {
                (x + 0.5) * TILE_SIZE,
                (y + 0.5) * TILE_SIZE
            };

            if (color.g == 0 && color.b == 0) {
                // SPAWN TOWER
                spawnTower(position, color.r * 10);
                tileData->type = GROUND_TILE;
            }

            
        }
    }

    return tileMap;
}

TileData* getTile(TileMap* tileMap, int x, int y) {
    if (x < 0 || y < 0 || x >= tileMap->width || y >= tileMap->height)
        return NULL;
    return & tileMap->tiles[x + y * tileMap->width];
}

void updateTileMap(TileMap* tileMap) {
    for ITERATE(x, tileMap->width) {
        for ITERATE(y, tileMap->height) {
            TileData* tile = getTile(tileMap, x, y);
            tile->minionIds.used = 0;
        }
    }

    for ITERATE(id, entityClasses[MINION_TYPE].bankSize) {
        Entity* entity = getEntity(MINION_TYPE, id);
        if (!entity->isSpawned) continue;
        TileData* tile = getTileAt(tileMap, entity->position);
        if (tile == NULL) continue;
        insertIntArray(& (tile->minionIds), id);
    }
}

void drawTileMap(TileMap* tileMap) {
    for ITERATE(x, tileMap->width) {
        for ITERATE(y, tileMap->height) {
            TileData* tile = getTile(tileMap, x, y);
            bool isDark = ((x / 3) % 2) ^ ((y / 3) % 2);
            Rectangle tileBounds = { 
                x * TILE_SIZE, y * TILE_SIZE,
                (x + 1) * TILE_SIZE, (y + 1) * TILE_SIZE,
            };

            switch(tile->type) {
                case GROUND_TILE:
                    DrawRectangleRec(tileBounds, GetColor(isDark ? GROUND_COLOR_2 : GROUND_COLOR));
                    break;
                case PLACEABLE_TILE:
                    DrawRectangleRec(tileBounds, GetColor(isDark ? PLACEABLE_COLOR_2 : PLACEABLE_COLOR));
                    break;
            }

            if (DEBUG_MODE) {
                char str[8];
                sprintf(str, "%d/%d", tile->minionIds.used, tile->minionIds.size);
                DrawText(str, tileBounds.x, tileBounds.y, 10, BLACK);
            }
        }
    }
}

void destroyTileMap(TileMap* tileMap) {
    for ITERATE(x, tileMap->width) {
        for ITERATE(y, tileMap->height) {
            freeIntArray(&getTile(tileMap, x, y)->minionIds);
        }
    }

    free(tileMap->tiles);
}

TileData* getTileAt(TileMap* tileMap, Vector2 position) {
    return getTile(tileMap, position.x / TILE_SIZE, position.y / TILE_SIZE);
}


//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {
    // Initialization
    //--------------------------------------------------------------------------------------

    InitWindow(SCREEN_SIZE.x, SCREEN_SIZE.y, "raylib [core] example - basic window");

    SetTargetFPS(120);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    for ITERATE(type, TYPE_COUNT) {
        initClass(type);
    }
    initIntArray(&minionIdsInRange, 100);

    Image tilemapImage = LoadImage("Images/Maps/TestLevel.png");
    currentTileMap = loadTileMap(&tilemapImage);
    UnloadImage(tilemapImage);

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // M Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------
        float delta = GetFrameTime();

        TileData* tileAtMouse = getTileAt(&currentTileMap, GetMousePosition());
        bool canSpawn = minionInventoryCount > 0 && tileAtMouse != NULL && tileAtMouse->type == PLACEABLE_TILE;

        if (
            canSpawn
            && (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
            || (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && Vector2Distance(GetMousePosition(), lastSpawnPoint) >= spawnDeltaDis))) {
            Vector2 spawnPoint = GetMousePosition();
            lastSpawnPoint = spawnPoint;
            minionInventoryCount--;
            spawnMinionAt(spawnPoint);
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        {
            spawnTower(GetMousePosition(), 100);
        }

        // Update Entities
        for ITERATE(type, TYPE_COUNT) {
            EntityClass* entityClass = &entityClasses[type];
            for ITERATE(id, entityClass->bankSize) {
                Entity* entity = getEntity(type, id);
                if (!entity->isSpawned) continue;
                entityClass->update(id, delta);
            }
        }

        // Update Tilemap
        updateTileMap(&currentTileMap);


        //----------------------------------------------------------------------------------
        // M Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        // Clear
        ClearBackground(RAYWHITE);

        drawTileMap(&currentTileMap);

        for ITERATE(type, TYPE_COUNT) {
            EntityClass* entityClass = &entityClasses[type];
            for ITERATE(id, entityClass->bankSize) {
                Entity* entity = getEntity(type, id);
                if (!entity->isSpawned) continue;
                entityClass->draw(id);
            }
        }


        // Gui

        char str[8];
        sprintf(str, "%d", minionInventoryCount);
        DrawText(str, 10, 10, 30, BLACK);

        //DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }



    // De-Initialization
    //--------------------------------------------------------------------------------------

    freeIntArray(&minionIdsInRange);
    destroyTileMap(&currentTileMap);

    for ITERATE(type, TYPE_COUNT) {
        destroyClass(type);
    }

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}





























