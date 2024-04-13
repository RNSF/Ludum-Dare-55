#include "raylib.h"
#include "raymath.h"
#include <assert.h>

//------------------------------------------------------------------------------------
// C Macros
//------------------------------------------------------------------------------------

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
// C Consts
//------------------------------------------------------------------------------------


const float MINION_SPEED = 50.0;
const float MINION_ACCELERATION = 5.0;
const Vector2 SCREEN_SIZE = { 1920 / 2, 1080 / 2 };
const float SPAWN_PERIOD = 0.01;

//------------------------------------------------------------------------------------
// C Structs
//------------------------------------------------------------------------------------

typedef struct Minion {
    Vector2 position;
    Vector2 velocity;
    int targetBaseId;
    float attackCooldown;
    bool isSpawned;
} Minion;


typedef struct Timer {
    float maxTime;
    float time;
} Timer;


typedef struct Base {
    Vector2 position;
    float health;
    int value;
} Base;


//------------------------------------------------------------------------------------
// C Vars
//------------------------------------------------------------------------------------


#define MAX_MINION_COUNT 5000
Minion minions[MAX_MINION_COUNT];
#define MINION_ATTACK_RANGE 20
#define MINION_ATTACK_PERIOD 1
int spawnedMinionCount = 0;
bool isMinionTargetRecalculationPending = false;
int minionInventoryCount = 100;

#define MAX_BASE_COUNT 10
int baseCount = 0;
Base bases[MAX_BASE_COUNT];


//------------------------------------------------------------------------------------
// C Func
//------------------------------------------------------------------------------------

int damageBase(int id, float damageAmount);




//------------------------------------------------------------------------------------
// C Utils
//------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------
// C Minions
//------------------------------------------------------------------------------------


void initMinions() {
    foreach(Minion * minion, minions) {
        minion->isSpawned = false;
    }
}

void updateMinions(float delta) {
    for ITERATE(i, spawnedMinionCount) {
        Minion* minion = &minions[i];
        if (!minion->isSpawned) continue;

        if (isMinionTargetRecalculationPending) {
            minion->targetBaseId = calculateMinionTarget(i);
        }

        bool inRange = minion->targetBaseId != NULLID
            && Vector2Distance(minion->position, bases[minion->targetBaseId].position) < MINION_ATTACK_RANGE;
        // UPDATE VELOCITY
        if (minion->targetBaseId != NULLID && !inRange) {
            Vector2 moveDirection = Vector2Normalize(
                Vector2Subtract(bases[minion->targetBaseId].position, minion->position)
            );

            minion->velocity = Vector2Scale(moveDirection, MINION_SPEED);
        } else {
            minion->velocity = Vector2Zero();
        }

        // UPDATE POSITION
        minion->position = Vector2Add(minion->position, Vector2Scale(minion->velocity, delta));

        // ATTACK
        if (minion->attackCooldown > 0) minion->attackCooldown -= delta;

        if (minion->targetBaseId != NULLID && inRange && minion->attackCooldown <= 0) {
            minion->attackCooldown += MINION_ATTACK_PERIOD;

            damageBase(minion->targetBaseId, 1);
        }

        
        
    }
    isMinionTargetRecalculationPending = false;
}

void drawMinions() {
    foreach(Minion * minion, minions) {
        if (!minion->isSpawned) continue;

        Vector2 p = minion->position;
        DrawRectangle(p.x - 5, p.y - 5, 10, 10, RED);
    }
}





int destroyMinion(int id) {
    assert(id >= 0 && id < spawnedMinionCount);
    minions[id].isSpawned = false;
}

void cleanMinions() {
    int newSpawnedMinionCount = 0;
    for (int i = 0; i < spawnedMinionCount; i++) {
        Minion *minion = &minions[i];
        if (minion->isSpawned) {
            newSpawnedMinionCount++;
            minions[spawnedMinionCount] = minions[i];
        }
    }

    spawnedMinionCount = newSpawnedMinionCount;
}

int calculateMinionTarget(int id) {
    Minion* minion = &minions[id];

    if (baseCount == 0) 
        return NULLID;

    int closestBaseId = 0;
    float sqrDistance = Vector2DistanceSqr(minion->position, bases[closestBaseId].position);
    for (int i = 1; i < baseCount; i++) {
        Base * base = &bases[i];
        float sqrDistance2 = Vector2DistanceSqr(minion->position, base->position);
        if (sqrDistance2 < sqrDistance)
        {
            closestBaseId = i;
            sqrDistance = sqrDistance2;
        }
    }

    return closestBaseId;
}


int spawnMinionAt(Vector2 position) {
    if (spawnedMinionCount >= MAX_MINION_COUNT) return NULLID;
    int id = spawnedMinionCount++;

    Minion* minion = &minions[id];

    minion->isSpawned = true;
    minion->position = position;
    minion->velocity = (Vector2){ 0, 0 };
    minion->targetBaseId = calculateMinionTarget(id);
    minion->attackCooldown = 0;

    return id;
}




//------------------------------------------------------------------------------------
// C Bases
//------------------------------------------------------------------------------------



int spawnBase(Vector2 position, float health, int value)
{
    assert(health > 0);

    if (baseCount >= MAX_BASE_COUNT) return NULLID;
    
    int id = baseCount++;
    Base* base = &bases[id];
    base->health = health;
    base->position = position;
    base->value = 100;

    isMinionTargetRecalculationPending = true;

    return id;
}

int damageBase(int id, float damageAmount)
{
    bases[id].health -= damageAmount;
}

int destroyBase(int id)
{
    minionInventoryCount += bases[id].value;
    bases[id] = bases[--baseCount];
    isMinionTargetRecalculationPending = true;
    
}

void updateBases()
{
    for ITERATE(i, baseCount)
    {
        if (bases[i].health <= 0) 
        {
            destroyBase(i);
        }
    }
}






//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------

    initMinions();

    InitWindow(SCREEN_SIZE.x, SCREEN_SIZE.y, "raylib [core] example - basic window");

    SetTargetFPS(120);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------


    Timer spawnTimer = { .maxTime = SPAWN_PERIOD };
    spawnTimer.time = spawnTimer.maxTime;

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------
        float delta = GetFrameTime();

        spawnTimer.time -= delta;

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && spawnTimer.time <= 0 && minionInventoryCount > 0) {
            spawnTimer.time = spawnTimer.maxTime;
            minionInventoryCount--;
            spawnMinionAt(GetMousePosition());
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        {
            spawnBase(GetMousePosition(), 100, 100);
        }

        // Update minions
        cleanMinions();
        updateMinions(GetFrameTime());
        updateBases(GetFrameTime());


        //----------------------------------------------------------------------------------
        // M Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        // Clear
        ClearBackground(RAYWHITE);

        drawMinions();

        for ITERATE(i, baseCount) {
            Base * base = &bases[i];
            DrawCircleV(base->position, 10, YELLOW);

            char str[8];
            sprintf(str, "%d", (int) ceil(base->health));
            DrawText(str, base->position.x, base->position.y, 12, BLACK);
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
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}





























