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

// Blend Color
Color ColorLerp(Color c1, Color c2, float amount) {
    return (Color) {
        Lerp(c1.r, c2.r, amount),
        Lerp(c1.g, c2.g, amount),
        Lerp(c1.b, c2.b, amount),
        Lerp(c1.a, c2.a, amount),
    };
}

// RandRange

// [0, 1]
float randFloat() {
    return GetRandomValue(0, INT_MAX) / INT_MAX;
}

// [min, max]
float randRange(float min, float max) {
    Lerp(min, max, randFloat());
}

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


// Draw anchored

void drawSpriteAnchored(Texture2D texture, Vector2 position, float rotation, Vector2 anchor, Color tint) {
    DrawTexturePro(texture, (Rectangle) { 0, 0, texture.width, texture.height }, 
                            (Rectangle) { position.x, position.y, texture.width, texture.height },
                    Vector2Multiply(anchor, (Vector2){ texture.width , texture.height}), rotation, tint
    );
}

void drawSpriteAnchoredScaled(Texture2D texture, Vector2 position, float rotation, float scale, Vector2 anchor, Color tint) {
    DrawTexturePro(texture, (Rectangle) { 0, 0, texture.width, texture.height },
        (Rectangle) {
        position.x, position.y, texture.width * scale, texture.height * scale
    },
        Vector2Multiply(anchor, (Vector2) { texture.width, texture.height }), rotation, tint
    );
}

void drawTextAnchored(Vector2 position, Vector2 anchor, Font font, const char* text, int fontSize, float spacing, Color color) {
    Vector2 textSize = MeasureTextEx(font, text, fontSize, spacing);
    Vector2 drawPosition = Vector2Subtract(position, Vector2Multiply(textSize, anchor));
    DrawTextEx(font, text, drawPosition, fontSize, spacing, color);
}


//------------------------------------------------------------------------------------
// C Camera
//------------------------------------------------------------------------------------

Camera2D camera;

void setCameraCenter(Camera2D* c, Vector2 center) {

    c->offset = Vector2Subtract((Vector2){ GetScreenWidth()  / 2, GetScreenHeight() / 2}, Vector2Scale(center, c->zoom));
}


//------------------------------------------------------------------------------------
// C Sprites
//------------------------------------------------------------------------------------


Texture2D PLAYER_MINION_SPRITE;
Texture2D ENEMY_MINION_SPRITE;
Texture2D ARCHER_TOWER_SPRITE;
Texture2D BOMB_TOWER_SPRITE;
Texture2D SUMMONER_TOWER_SPRITE;
Texture2D TRAP_SPRITE;
Texture2D ARROW_SPRITE;
Texture2D BOMB_SPRITE;
Texture2D MINION_SHADOW_SPRITE;
Texture2D TRAP_SHADOW_SPRITE;
Texture2D TOWER_SHADOW_SPRITE;
Texture2D DUST_PARTICLE_SPRITE;
Texture2D BRICK_PARTICLE_SPRITE;

void loadSprites() {
    PLAYER_MINION_SPRITE = LoadTexture("Images/Entities/PlayerMinion.png");
    ENEMY_MINION_SPRITE = LoadTexture("Images/Entities/EnemyMinion.png");
    ARCHER_TOWER_SPRITE = LoadTexture("Images/Entities/Tower0.png");
    BOMB_TOWER_SPRITE = LoadTexture("Images/Entities/Tower2.png");
    SUMMONER_TOWER_SPRITE = LoadTexture("Images/Entities/Tower1.png");
    TRAP_SPRITE = LoadTexture("Images/Entities/Trap.png");
    ARROW_SPRITE = LoadTexture("Images/Entities/Arrow.png");
    BOMB_SPRITE = LoadTexture("Images/Entities/Bomb.png");
    MINION_SHADOW_SPRITE = LoadTexture("Images/Entities/MinionShadow.png");
    TRAP_SHADOW_SPRITE = LoadTexture("Images/Entities/TrapShadow.png");
    TOWER_SHADOW_SPRITE = LoadTexture("Images/Entities/TowerShadow.png");
    DUST_PARTICLE_SPRITE = LoadTexture("Images/Entities/DustParticle.png");
    BRICK_PARTICLE_SPRITE = LoadTexture("Images/Entities/BrickParticle.png");
}

void unloadSprites() {
    UnloadTexture(PLAYER_MINION_SPRITE);
    UnloadTexture(ENEMY_MINION_SPRITE);
    UnloadTexture(ARCHER_TOWER_SPRITE);
    UnloadTexture(BOMB_TOWER_SPRITE);
    UnloadTexture(SUMMONER_TOWER_SPRITE);
    UnloadTexture(TRAP_SPRITE);
    UnloadTexture(ARROW_SPRITE);
    UnloadTexture(BOMB_SPRITE);
    UnloadTexture(MINION_SHADOW_SPRITE);
    UnloadTexture(TRAP_SHADOW_SPRITE);
    UnloadTexture(TOWER_SHADOW_SPRITE);
    UnloadTexture(DUST_PARTICLE_SPRITE);
    UnloadTexture(BRICK_PARTICLE_SPRITE);
}


//------------------------------------------------------------------------------------
// C Fonts
//------------------------------------------------------------------------------------


Font MAIN_FONT;

void loadFonts() {
    MAIN_FONT = LoadFontEx("Fonts/LilitaOne-Regular.ttf", 32, 0, 0);
}

void unloadFonts() {
    UnloadFont(MAIN_FONT);
}


//------------------------------------------------------------------------------------
// C Consts
//------------------------------------------------------------------------------------

#define TILE_SIZE 60

const float MINION_SPEED = 100.0;
const float MINION_ACCELERATION = 10.0;
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
    float lifeTime;
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
	int type;
    int health;
    int value;
    float attackCooldown;
} Tower;

typedef struct Projectile {
    Entity entity;
    Vector2 startPosition;
    Vector2 targetPosition;
    int targetMinionId;
    int type;
    float aliveTime;
    float totalAliveTime;
    float angle;
} Projectile;


typedef struct Trap {
    Entity entity;
} Trap;

typedef struct Particle {
    Entity entity;
    Texture2D* sprite;
    Vector3 velocity;
    Vector3 acceleration;
    float duration;
    float dampening;
    Color startColor;
    Color endColor;
    float startScale;
    float endScale;
} Particle;


typedef struct EntityClass {
    //void (*spawnCallback);
    void (*destroyCallback)(int);
    void (*update)(int, float);
    void (*draw)(int);
    void* bank;
    int bankSize;
    int structSize;
    int lastSpawnedId;
    int spawnCount;
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

typedef struct Level {
    char* imagePath;
    int startingMinionCount;
} Level;

typedef struct GlobalId {
    int type;
    int id;
} GlobalId;


#define MINION_TYPE 0
#define TOWER_TYPE 1
#define PROJECTILE_TYPE 2
#define TRAP_TYPE 3
#define PARTICLE_TYPE 4
#define TYPE_COUNT 5

#define ARROW_PROJECTILE_TYPE 0
#define BOMB_PROJECTILE_TYPE 1

#define ARCHER_TOWER_TYPE 0
#define BOMB_TOWER_TYPE 1
#define SUMMONER_TOWER_TYPE 2

EntityClass entityClasses[TYPE_COUNT];


//------------------------------------------------------------------------------------
// C Func
//------------------------------------------------------------------------------------

void damageTower(int id, int damageAmount);
void updateMinion(int id, float delta);
void updateTower(int id, float delta);
void updateProjectile(int id, float delta);
void updateTrap(int id, float delta);
void updateParticle(int id, float delta);
void drawMinion(int id);
void drawTower(int id);
void drawProjectile(int id);
void drawTrap(int id);
void drawParticle(int id);
void onMinionDestroyed(int id);
void onTowerDestroyed(int id);
void onProjectileDestroyed(int id);
void onTrapDestroyed(int id);
void onParticleDestroyed(int id);
Entity* getEntity(int type, int id);
TileData* getTile(TileMap* tileMap, int x, int y);
TileMap loadTileMap(Image* mapImage);
TileData* getTileAt(TileMap* tileMap, Vector2 position);
void getMinionIdsInRange(IntArray* result, TileMap* tileMap, Vector2 position, float radius);
int spawnProjectile(int type, Vector2 startPosition, int targetMinionId, float totalAliveTime);
float calculateProjectileHeight(float timePercent);
void loadLevel(Level* level);
void loadNextLevel();
void reloadLevel();
void resetClass(int type);
void loadPreviousLevel();
void quickSortGlobalId(GlobalId arr[], int low, int high);
int explodeAt(Vector2 position, float radius);
int spawnParticle(
    Vector3 position,
    Texture2D* sprite,
    Vector3 velocity,
    Vector3 acceleration,
    float duration,
    float dampening,
    Color startColor,
    Color endColor,
    float startScale,
    float endScale
);


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
            entityClass->destroyCallback = &onMinionDestroyed;
            break;
        case TOWER_TYPE:
            entityClass->bankSize = 10;
            entityClass->structSize = sizeof(Tower);
            entityClass->update = &updateTower;
            entityClass->draw = &drawTower;
            entityClass->destroyCallback = &onTowerDestroyed;
            break;
        case PROJECTILE_TYPE:
            entityClass->bankSize = 1000;
            entityClass->structSize = sizeof(Projectile);
            entityClass->update = &updateProjectile;
            entityClass->draw = &drawProjectile;
            entityClass->destroyCallback = &onProjectileDestroyed;
            break;
        case TRAP_TYPE:
            entityClass->bankSize = 30;
            entityClass->structSize = sizeof(Trap);
            entityClass->update = &updateTrap;
            entityClass->draw = &drawTrap;
            entityClass->destroyCallback = &onTrapDestroyed;
            break;
        case PARTICLE_TYPE:
            entityClass->bankSize = 1000;
            entityClass->structSize = sizeof(Particle);
            entityClass->update = &updateParticle;
            entityClass->draw = &drawParticle;
            entityClass->destroyCallback = &onParticleDestroyed;
            break;
    }
    

    int allocSize = entityClass->bankSize * entityClass->structSize;
    entityClass->bank = malloc(allocSize);
    
    resetClass(type);

    
    entityClass->lastSpawnedId = -1;


}

void resetClass(int type) {
    EntityClass* entityClass = &entityClasses[type];

    for ITERATE(id, entityClass->bankSize) {
        Entity* entity = getEntity(type, id);
        entity->isSpawned = false;
        entityClass->spawnCount = 0;
    }
}

void destroyClass(int type) {
    EntityClass* entityClass = &entityClasses[type];
    free(entityClass->bank);
}


//------------------------------------------------------------------------------------
// C Entity
//------------------------------------------------------------------------------------



Entity* getEntity(int type, int id) {
    return ((intptr_t)entityClasses[type].bank + id * entityClasses[type].structSize);
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
            entity->lifeTime = 0;
            entityClass->spawnCount++;
            return i;
        }
    }
    return NULLID;
}


void destroyEntity(int type, int id) {
    Entity* entity = getEntity(type, id);
    entity->isSpawned = false;
    entityClasses[type].spawnCount--;
    entityClasses[type].destroyCallback(id);
}

//------------------------------------------------------------------------------------
// C GlobalIdArray
//------------------------------------------------------------------------------------


typedef struct GlobalIdArray {
    GlobalId* array;
    size_t used;
    size_t size;
} GlobalIdArray;


void initGlobalIdArray(GlobalIdArray* a, size_t initialSize) {
    a->array = malloc(initialSize * sizeof(GlobalId));
    a->used = 0;
    a->size = initialSize;
}

void insertGlobalIdArray(GlobalIdArray* a, GlobalId element) {
    if (a->used == a->size) {
        a->size *= 2;
        a->array = realloc(a->array, a->size * sizeof(GlobalId));
    }
    a->array[a->used++] = element;
}

void freeGlobalIdArray(GlobalIdArray* a) {
    free(a->array);
    a->array = NULL;
    a->used = a->size = 0;
}




// QUICK SORT
// from https://www.geeksforgeeks.org/quick-sort-in-c/
void swapGlobalId(GlobalId* a, GlobalId* b)
{
    GlobalId temp = *a;
    *a = *b;
    *b = temp;
}

int partitionGlobalId(GlobalId* arr, int low, int high)
{
    GlobalId pivot = arr[low];
    int i = low;
    int j = high;

    while (i < j) {

        while (getEntity(arr[i].type, arr[i].id)->position.y <= getEntity(pivot.type, pivot.id)->position.y && i <= high - 1) {
            i++;
        }
        while (getEntity(arr[j].type, arr[j].id)->position.y > getEntity(pivot.type, pivot.id)->position.y && j >= low + 1) {
            j--;
        }
        if (i < j) {
            swapGlobalId(&arr[i], &arr[j]);
        }
    }
    swapGlobalId(&arr[low], &arr[j]);
    return j;
}


void quickSortGlobalId(GlobalId* arr, int low, int high)
{
    if (low < high) {
        int partitionIndex = partitionGlobalId(arr, low, high);
        quickSortGlobalId(arr, low, partitionIndex - 1);
        quickSortGlobalId(arr, partitionIndex + 1, high);
    }
}

void sortGlobalIdArrayByDepth(GlobalIdArray* a) {
    quickSortGlobalId(a->array, 0, a->used - 1);
}




//------------------------------------------------------------------------------------
// C Vars
//------------------------------------------------------------------------------------

#define MINION_ATTACK_RANGE 4;


bool isMinionTargetRecalculationPending;
int minionInventoryCount;
TileMap currentTileMap;
IntArray minionIdsInRange;
GlobalIdArray allEntities;

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
    //DrawRectangle(p.x - 5, p.y - 5, 10, 10, RED);
    Vector2 p2 = p;
    p2.y -= abs(sin(minion->entity.lifeTime * 10) * 7);

    
    drawSpriteAnchored(PLAYER_MINION_SPRITE, p2, 0, (Vector2) { 0.5, 1.0 }, GetColor(PLAYER_COLOR));
    
}


void onMinionDestroyed(int id) {
    if (entityClasses[MINION_TYPE].spawnCount == 0 && minionInventoryCount == 0) {
        reloadLevel();
    }
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
    if (id == NULLID) return NULLID;

    Minion* minion = getEntity(MINION_TYPE, id);

    minion->entity.position = position;
    minion->velocity = (Vector2){ 0, 0 };
    minion->targetTowerId = calculateMinionTarget(id);
    minion->isPlayer = true;
    minion->isTargted = false;

    spawnParticle(
        (Vector3) { minion->entity.position.x - 10, minion->entity.position.y, 5 }, 
        &DUST_PARTICLE_SPRITE,
        (Vector3) { -20, 0, 0 }, (Vector3) { 0, 0, 100 },
        1.0, 0.5, WHITE, GetColor(0xFFFFFF00), 1.0, 0.2
    );

    spawnParticle(
        (Vector3) { minion->entity.position.x + 10, minion->entity.position.y, 5 }, 
        &DUST_PARTICLE_SPRITE,
        (Vector3) { 20, 0, 0 }, (Vector3) { 0, 0, 100 },
        1.0, 0.5, WHITE, GetColor(0xFFFFFF00), 1.0, 0.2
    );

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

const float TOWER_ATTACK_RADIUS[] = {
    320,
    160,
    0
};

const float TOWER_ATTACK_PERIOD[] = {
    0.4,
    1.5,
    0
};

const float TOWER_PROJECTILE_SPEED[] = {
    320,
    320,
    0
};




int spawnTower(int type, Vector2 position, float health) {
    assert(health > 0);

    int id = createEntity(TOWER_TYPE);
    if (id == NULLID) return;
    
    Tower* tower = getEntity(TOWER_TYPE, id);
    tower->type = type;
    tower->health = health;
    tower->entity.position = position;
    tower->value = health * 2;
    tower->attackCooldown = TOWER_ATTACK_PERIOD[type];

    isMinionTargetRecalculationPending = true;

    return id;
}

void drawTower(int id) {
    Tower* tower = getEntity(TOWER_TYPE, id);
    
    Texture2D* sprite = &ARCHER_TOWER_SPRITE;
    switch(tower->type) {
        case ARCHER_TOWER_TYPE: sprite = &ARCHER_TOWER_SPRITE; break;
        case BOMB_TOWER_TYPE: sprite = &BOMB_TOWER_SPRITE; break;
        case SUMMONER_TOWER_TYPE: sprite = &SUMMONER_TOWER_SPRITE; break;
    }
    
    drawSpriteAnchored(*sprite, tower->entity.position, 0, (Vector2) { 0.5, 1.0 }, GetColor(ENEMY_COLOR));
    

    
    char str[8];
    sprintf(str, "%d", (int) ceil(tower->health));
    /*DrawText(str, tower->entity.position.x, tower->entity.position.y - ARCHER_TOWER_SPRITE.height / 2, 30, BLACK);*/

    Vector2 textPosition = tower->entity.position;
    textPosition.y -= ARCHER_TOWER_SPRITE.height / 2 - 4;

    drawTextAnchored(textPosition, (Vector2) { 0.5, 0.5 }, MAIN_FONT, str, 32, 0.0, BLACK);
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
    if (tower->attackCooldown > 0) {
        tower->attackCooldown -= delta;
    }

    else {
        

        getMinionIdsInRange(&minionIdsInRange, &currentTileMap, tower->entity.position, TOWER_ATTACK_RADIUS[tower->type]);

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
            float attackTime = distanceToMinion / TOWER_PROJECTILE_SPEED[tower->type];
            minion->isTargted = true;
            int projectileType = tower->type == ARCHER_TOWER_TYPE ? ARROW_PROJECTILE_TYPE : BOMB_PROJECTILE_TYPE;
            spawnProjectile(projectileType, tower->entity.position, id, max(attackTime, 0.1));
            tower->attackCooldown += TOWER_ATTACK_PERIOD[tower->type];
        }
    }

    
}

void onTowerDestroyed(int id) {
    Tower* tower = getEntity(TOWER_TYPE, id);

    /*spawnParticle(
        (Vector3) { tower->entity.position.x - 10, tower->entity.position.y, 20 },
        &BRICK_PARTICLE_SPRITE,
        (Vector3) { -20, 0, 0 }, (Vector3) { 0, 0, 100 },
        1.0, 0.5, WHITE, GetColor(0xFFFFFF00), 1.0, 0.2
    );

    spawnParticle(
        (Vector3) { tower->entity.position.x - 10, tower->entity.position.y, 20 },
        &BRICK_PARTICLE_SPRITE,
        (Vector3) { -20, 0, 0 }, (Vector3) { 0, 0, 100 },
        1.0, 0.5, WHITE, GetColor(0xFFFFFF00), 1.0, 0.2
    );

    spawnParticle(
        (Vector3) { tower->entity.position.x - 10, tower->entity.position.y, 20 },
        &BRICK_PARTICLE_SPRITE,
        (Vector3) { -20, 0, 0 }, (Vector3) { 0, 0, 100 },
        1.0, 0.5, WHITE, GetColor(0xFFFFFF00), 1.0, 0.2
    );

    spawnParticle(
        (Vector3) { tower->entity.position.x - 10, tower->entity.position.y, 20 },
        &BRICK_PARTICLE_SPRITE,
        (Vector3) { -20, 0, 0 }, (Vector3) { 0, 0, 100 },
        1.0, 0.5, WHITE, GetColor(0xFFFFFF00), 1.0, 0.2
    );*/

    if (entityClasses[TOWER_TYPE].spawnCount == 0) {
        loadNextLevel();
    }
}


//------------------------------------------------------------------------------------
// C Projectile
//------------------------------------------------------------------------------------

#define BOMB_EXPLOSION_RADIUS 100

int spawnProjectile(int type, Vector2 startPosition, int targetMinionId, float totalAliveTime) {
    assert(getEntity(MINION_TYPE, targetMinionId)->isSpawned);

    int id = createEntity(PROJECTILE_TYPE);
    Projectile* projectile = getEntity(PROJECTILE_TYPE, id);

    projectile->startPosition = startPosition;
    projectile->targetMinionId = targetMinionId;
    projectile->type = type;

    Minion* targetMinion = getEntity(MINION_TYPE, targetMinionId);

    projectile->targetPosition = Vector2Add(targetMinion->entity.position, Vector2Scale(targetMinion->velocity, totalAliveTime));
    projectile->aliveTime = 0.0;
    projectile->totalAliveTime = totalAliveTime;
    projectile->entity.position = startPosition;
    projectile->entity.height = calculateProjectileHeight(0);
    return id;
}


void drawProjectile(int id) {
    Projectile* projectile = getEntity(PROJECTILE_TYPE, id);
    Vector2 drawPosition = projectile->entity.position;
    drawPosition.y -= projectile->entity.height;
    float angle = projectile->type == ARROW_PROJECTILE_TYPE ? 
        90 + projectile->angle * RAD2DEG : projectile->angle * RAD2DEG * 0.4;
        
    drawSpriteAnchored(projectile->type == ARROW_PROJECTILE_TYPE ? ARROW_SPRITE : BOMB_SPRITE, drawPosition, angle, (Vector2) { 0.5, 0 }, GetColor(ENEMY_COLOR));
}


void updateProjectile(int id, float delta) {
    Projectile* projectile = getEntity(PROJECTILE_TYPE, id);

    // Update target position
    if (projectile->targetMinionId != NULLID) {
        Minion* targetMinion = getEntity(MINION_TYPE, projectile->targetMinionId);
        if (!targetMinion->entity.isSpawned) {
            projectile->targetMinionId = NULLID;
        } else {
            //projectile->targetPosition = targetMinion->entity.position;
        }
    }

    // Update alive time
    projectile->aliveTime += delta;
    float totalTime = projectile->totalAliveTime;
    float alivePercentage = projectile->aliveTime / projectile->totalAliveTime;

    if (alivePercentage >= 1.0) {
        switch(projectile->type) {
            case ARROW_PROJECTILE_TYPE:
                if (projectile->targetMinionId != NULLID) {
                    destroyEntity(MINION_TYPE, projectile->targetMinionId);
                }
                break;
            case BOMB_PROJECTILE_TYPE:
                explodeAt(projectile->targetPosition, BOMB_EXPLOSION_RADIUS);
                break;
                
        }
        return destroyEntity(PROJECTILE_TYPE, id);
        
    }

    float oldY = projectile->entity.position.y - projectile->entity.height;
    float oldX = projectile->entity.position.x;

    // Update position / height
    projectile->entity.position = Vector2Lerp(projectile->startPosition, projectile->targetPosition, alivePercentage);
    projectile->entity.height = calculateProjectileHeight(alivePercentage);

    float y = projectile->entity.position.y - projectile->entity.height;
    float x = projectile->entity.position.x;
    

    projectile->angle = atan2f(y - oldY, x - oldX);
}

float calculateProjectileHeight(float timePercent) {
    static const float startHeight = 40.0;
    static const float peakHeight = 80.0; // this is not actually peak height, but I'm too lazy to make the equation better
    static const float endHeight = 10.0;

    float result = -peakHeight * (timePercent - 1) * (timePercent + startHeight / peakHeight) + endHeight;

    return result;
}


void onProjectileDestroyed(int id) {
    
} 

//------------------------------------------------------------------------------------
// C Trap
//------------------------------------------------------------------------------------

#define TRAP_RANGE 40
#define TRAP_EXPLOSION_RADIUS 120

void updateTrap(int id, float delta) {
    Trap* trap = getEntity(TRAP_TYPE, id);

    // Can optimize to check hasMinionInRange
    getMinionIdsInRange(&minionIdsInRange, &currentTileMap, trap->entity.position, TRAP_RANGE);

    if (minionIdsInRange.used > 0) {
        explodeAt(trap->entity.position, TRAP_EXPLOSION_RADIUS);
        destroyEntity(TRAP_TYPE, id);
    }
}

void drawTrap(int id) {
    Trap* trap = getEntity(TRAP_TYPE, id);
    drawSpriteAnchored(TRAP_SPRITE, trap->entity.position, 0, (Vector2) { 0.5, 1.0 }, GetColor(ENEMY_COLOR));
}

void onTrapDestroyed(int id) {

}


//------------------------------------------------------------------------------------
// C Particle
//------------------------------------------------------------------------------------

int spawnParticle(
    Vector3 position,
    Texture2D* sprite,
    Vector3 velocity,
    Vector3 acceleration,
    float duration,
    float dampening,
    Color startColor,
    Color endColor,
    float startScale,
    float endScale
) {
    int id = createEntity(PARTICLE_TYPE);
    if (id == NULLID) return NULLID;

    Particle* particle = getEntity(PARTICLE_TYPE, id);

    particle->entity.position = * (Vector2*) &position;
    particle->entity.height = position.z;
    particle->sprite = sprite;
    particle->velocity = velocity;
    particle->acceleration = acceleration;
    particle->duration = duration;
    particle->dampening = dampening;
    particle->startColor = startColor;
    particle->endColor = endColor;
    particle->startScale = startScale;
    particle->endScale = endScale;

    return id;
}

void updateParticle(int id, float delta) {
    Particle* particle = getEntity(PARTICLE_TYPE, id);

    if (particle->entity.lifeTime > particle->duration) {
        destroyEntity(PARTICLE_TYPE, id);
        return;
    } 

    particle->velocity = Vector3Add(particle->velocity, Vector3Scale(particle->acceleration, delta));
    particle->velocity = Vector3Add(particle->velocity, Vector3Scale(particle->velocity, -particle->dampening * delta));
    particle->entity.position.x += particle->velocity.x * delta;
    particle->entity.position.y += particle->velocity.y * delta;
    particle->entity.height += particle->velocity.z * delta;
}

void drawParticle(int id) {
    Particle* particle = getEntity(PARTICLE_TYPE, id);
    float alivePercent = particle->entity.lifeTime / particle->duration;

    Color color = ColorLerp(particle->startColor, particle->endColor, alivePercent);
    float scale = Lerp(particle->startScale, particle->endScale, alivePercent);

    Vector2 drawPosition = particle->entity.position;
    drawPosition.y -= particle->entity.height;

    drawSpriteAnchoredScaled(*particle->sprite, drawPosition, 0, scale, (Vector2) { 0.5, 0.5 }, color);
}

void onParticleDestroyed(int id) {

}

//------------------------------------------------------------------------------------
// C Explosion
//------------------------------------------------------------------------------------

int explodeAt(Vector2 position, float radius) {
    getMinionIdsInRange(&minionIdsInRange, &currentTileMap, position, radius);

    printf("%d\n", minionIdsInRange.used);
    for ITERATE(i, minionIdsInRange.used) {
        int id = minionIdsInRange.array[i];
        destroyEntity(MINION_TYPE, id);
    }

}


//------------------------------------------------------------------------------------
// C TileMap
//------------------------------------------------------------------------------------





#define GROUND_TILE 0xffffffff
#define PLACEABLE_TILE 0x3294c4ff
#define TRAP_TILE 0xc49632ff

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
                spawnTower(ARCHER_TOWER_TYPE, position, color.r * 10);
                tileData->type = GROUND_TILE;
            }

            if (tileData->type == TRAP_TILE) {
                // SPAWN TRAP
                int id = createEntity(TRAP_TYPE);
                getEntity(TRAP_TYPE, id)->position = position;
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
                TILE_SIZE, TILE_SIZE,
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
// C LoadLevel
//------------------------------------------------------------------------------------



#define LEVEL_COUNT 8
Level levels[LEVEL_COUNT];
int currentLevelNumber = -1;
int pendingLevelNumber = -1;

void initLevels() {
    levels[0] = (Level){ .imagePath = "Images/Maps/Level0.png", .startingMinionCount = 100 };
    levels[1] = (Level){ .imagePath = "Images/Maps/Level1.png", .startingMinionCount = 40 };
    levels[2] = (Level){ .imagePath = "Images/Maps/TrapTest.png", .startingMinionCount = 10000 };
    levels[3] = (Level){ .imagePath = "Images/Maps/TestLevel.png", .startingMinionCount = 10 };
    levels[4] = (Level){ .imagePath = "Images/Maps/TestLevel.png", .startingMinionCount = 10 };
    levels[5] = (Level){ .imagePath = "Images/Maps/TestLevel.png", .startingMinionCount = 10 };
    levels[6] = (Level){ .imagePath = "Images/Maps/TestLevel.png", .startingMinionCount = 10 };
    levels[7] = (Level){ .imagePath = "Images/Maps/TestLevel.png", .startingMinionCount = 10 };
}

void reloadLevel() {
    pendingLevelNumber = currentLevelNumber;
}

void loadNextLevel() {
    pendingLevelNumber = max(0, currentLevelNumber + 1);
}

void loadPreviousLevel() {
    pendingLevelNumber = min(currentLevelNumber - 1, LEVEL_COUNT - 1);
}

void loadLevel(Level* level) {

    // Reset classes
    for ITERATE(type, TYPE_COUNT) {
        resetClass(type);
    }

    // Load map
    Image tilemapImage = LoadImage(level->imagePath);
    currentTileMap = loadTileMap(&tilemapImage);
    UnloadImage(tilemapImage);

    // Reset Array
    freeIntArray(&minionIdsInRange);
    initIntArray(&minionIdsInRange, 128);

    freeGlobalIdArray(&allEntities);
    initGlobalIdArray(&allEntities, 128);

    // Reset Values
    isMinionTargetRecalculationPending = false;
    minionInventoryCount = level->startingMinionCount;

    

    camera.zoom = 0.5;
    setCameraCenter(&camera, (Vector2) {
        currentTileMap.width * TILE_SIZE / 2,
        currentTileMap.height * TILE_SIZE / 2
    });
    
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
    
    initIntArray(&minionIdsInRange, 128);
    initGlobalIdArray(&allEntities, 128);

    initLevels();
    

    loadSprites();
    loadFonts();

    loadNextLevel();
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // M Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------
        
        if (IsKeyPressed(KEY_R)) {
            reloadLevel();
        }
        if (IsKeyPressed(KEY_M)) {
            loadNextLevel();
        }
        if (IsKeyPressed(KEY_N)) {
            loadPreviousLevel();
        }

        if (pendingLevelNumber != NULLID) {
            currentLevelNumber = pendingLevelNumber;
            pendingLevelNumber = NULLID;
            loadLevel(&levels[currentLevelNumber]);
        }

        
        
        float delta = GetFrameTime();

        Vector2 mouseWorldPosition = GetScreenToWorld2D(GetMousePosition(), camera);

        TileData* tileAtMouse = getTileAt(&currentTileMap, mouseWorldPosition);
        bool canSpawn = minionInventoryCount > 0 && tileAtMouse != NULL && tileAtMouse->type == PLACEABLE_TILE;

        if (
            canSpawn
            && (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
            || (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && Vector2Distance(mouseWorldPosition, lastSpawnPoint) >= spawnDeltaDis))) {
            Vector2 spawnPoint = mouseWorldPosition;
            lastSpawnPoint = spawnPoint;
            minionInventoryCount--;
            spawnMinionAt(spawnPoint);
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && DEBUG_MODE)
        {
            spawnTower(BOMB_TOWER_TYPE, mouseWorldPosition, 100);
        }

        // Update Entities
        for ITERATE(type, TYPE_COUNT) {
            EntityClass* entityClass = &entityClasses[type];
            for ITERATE(id, entityClass->bankSize) {
                Entity* entity = getEntity(type, id);
                if (!entity->isSpawned) continue;
                entity->lifeTime += delta;
                entityClass->update(id, delta);
            }
        }

        // Update Tilemap
        updateTileMap(&currentTileMap);

        //printf("%d\n", entityClasses[MINION_TYPE].spawnCount);

        //----------------------------------------------------------------------------------
        // M Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        // Clear
        ClearBackground(RAYWHITE);

        // World
        BeginMode2D(camera);                         
                                     

        drawTileMap(&currentTileMap);

        // Shadows
        for ITERATE(id, entityClasses[MINION_TYPE].bankSize) {
            Entity* entity = getEntity(MINION_TYPE, id);
            if (!entity->isSpawned) continue;
            drawSpriteAnchored(MINION_SHADOW_SPRITE, entity->position, 0, (Vector2) { 0.5, 0.5 }, WHITE);
        }

        for ITERATE(id, entityClasses[TOWER_TYPE].bankSize) {
            Entity* entity = getEntity(TOWER_TYPE, id);
            if (!entity->isSpawned) continue;
            drawSpriteAnchored(TOWER_SHADOW_SPRITE, entity->position, 0, (Vector2) { 0.5, 0.5 }, WHITE);
        }

        for ITERATE(id, entityClasses[TRAP_TYPE].bankSize) {
            Entity* entity = getEntity(TRAP_TYPE, id);
            if (!entity->isSpawned) continue;
            drawSpriteAnchored(TRAP_SHADOW_SPRITE, entity->position, 0, (Vector2) { 0.5, 0.5 }, WHITE);
        }


        // Main Draw

        
        allEntities.used = 0;

        for ITERATE(type, TYPE_COUNT) {
            EntityClass* entityClass = &entityClasses[type];
            for ITERATE(id, entityClass->bankSize) {
                Entity* entity = getEntity(type, id);
                if (!entity->isSpawned) continue;

                //entityClass->draw(id);
                insertGlobalIdArray(&allEntities, (GlobalId){type, id});
            }
        }
        sortGlobalIdArrayByDepth(&allEntities);

        for ITERATE(i, allEntities.used) {
            GlobalId globalId = allEntities.array[i];
            int type = globalId.type;
            int id = globalId.id;

            Entity* entity = getEntity(type, id);
            if (!entity->isSpawned) continue;
            entityClasses[type].draw(id);
        }

        EndMode2D(camera);
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

    unloadSprites();
    unloadFonts();

    freeIntArray(&minionIdsInRange);
    freeGlobalIdArray(&allEntities);

    destroyTileMap(&currentTileMap);

    for ITERATE(type, TYPE_COUNT) {
        destroyClass(type);
    }

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}





























