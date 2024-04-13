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
    Vector2 targetPosition;
    bool isSpawned;
} Minion;


typedef struct Timer {
    float maxTime;
    float time;
} Timer;


typedef struct Base {
    Vector2 position;
    float health;
} Base;


//------------------------------------------------------------------------------------
// C Vars
//------------------------------------------------------------------------------------


#define MAX_MINION_COUNT 5000
Minion minions[MAX_MINION_COUNT];
int spawnedMinionCount = 0;

#define MAX_BASE_COUNT 10
int baseCount = 0;
Base bases[MAX_BASE_COUNT];

//------------------------------------------------------------------------------------
// C Minions
//------------------------------------------------------------------------------------


void initMinions() {
    foreach(Minion * minion, minions) {
        minion->isSpawned = false;
    }
}

void updateMinions(float delta) {
    foreach(Minion * minion, minions) {
        if (!minion->isSpawned) continue;

        // UPDATE VELOCITY
        Vector2 moveDirection = Vector2Normalize(
            Vector2Subtract(minion->targetPosition, minion->position)
        );

        minion->velocity = Vector2Scale(moveDirection, MINION_SPEED);

        // UPDATE POSITION
        minion->position = Vector2Add(minion->position, Vector2Scale(minion->velocity, delta));

        
    }
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

Vector2 calculateMinionTarget(int id) {
    Minion* minion = &minions[id];

    if (!baseCount) 
        return minion->position;

    Base *closestBase = &bases[0];
    float sqrDistance = Vector2DistanceSqr(minion->position, closestBase->position);
    for (int i = 1; i < baseCount; i++) {
        Base * base = &bases[i];
        float sqrDistance2 = Vector2DistanceSqr(minion->position, base->position);
        if (sqrDistance2 < sqrDistance)
        {
            closestBase = base;
            sqrDistance = sqrDistance2;
        }
    }

    return closestBase->position;
}


int spawnMinionAt(Vector2 position) {
    if (spawnedMinionCount >= MAX_MINION_COUNT) return NULLID;
    int id = spawnedMinionCount++;

    Minion* minion = &minions[id];

    minion->isSpawned = true;
    minion->position = position;
    minion->velocity = (Vector2){ 0, 0 };
    minion->targetPosition = calculateMinionTarget(id);

    return id;
}




//------------------------------------------------------------------------------------
// C Bases
//------------------------------------------------------------------------------------



int spawnBase(Vector2 position, float health)
{
    assert(health > 0);

    if (baseCount >= MAX_BASE_COUNT) return NULLID;
    
    int id = baseCount++;
    Base* base = &bases[id];
    base->health = health;
    base->position = position;

    return id;
}

int damageBase(int id, float damageAmount)
{
    bases[id].health -= damageAmount;
    if (bases[id].health <= 0)
        destroyBase(id);
}

int destroyBase(int id)
{
    bases[id] = bases[--baseCount];
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

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && spawnTimer.time <= 0) {
            spawnTimer.time = spawnTimer.maxTime;
            spawnMinionAt(GetMousePosition());
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        {
            spawnBase(GetMousePosition(), 10);
        }

        // Update minions
        cleanMinions();
        updateMinions(GetFrameTime());


        //----------------------------------------------------------------------------------
        // M Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        // Clear
        ClearBackground(RAYWHITE);

        drawMinions();

        for ITERATE(i, baseCount) {
            DrawCircleV(bases[i].position, 10, YELLOW);
        }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}





























