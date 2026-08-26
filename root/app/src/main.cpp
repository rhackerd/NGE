#include "high-level/graphics.h"
#include "high-level/object.h"
#include "high-level/renderer.h"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_keyboard.h>
#include <vector>
#include <cmath>

struct FlyCamera {
    float x = 0.0f, y = 5.0f, z = 10.0f;
    float yaw = -90.0f;   // facing -Z initially
    float pitch = 0.0f;
    float speed = 5.0f;
    float sensitivity = 0.1f;

    void getForward(float& fx, float& fy, float& fz) const {
        float yawRad = yaw * (3.14159265f / 180.0f);
        float pitchRad = pitch * (3.14159265f / 180.0f);
        fx = cosf(yawRad) * cosf(pitchRad);
        fy = sinf(pitchRad);
        fz = sinf(yawRad) * cosf(pitchRad);
    }
};

// ========================================
/* Status: #legacy
Description: This code shows an example of experimental features which may not be used in the future.
Additional Info: None
*/
// ========================================

int main() {
    #ifdef __linux__
    const char* desktop = getenv("XDG_CURRENT_DESKTOP");
    if (desktop) {
        std::string d(desktop);
        std::transform(d.begin(), d.end(), d.begin(), ::tolower);
        if (d.find("gnome") != std::string::npos) {
            SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11");
            printf("Detected gnome, using x11 video driver\n");
        }
    }
    #endif

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMEPAD);

    SDL_Window* win = SDL_CreateWindow("Nova Graphics testing window", 800, 600, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);

    Nova::Graphics::Graphics gfx;
    if (!gfx.init(win)) return 1;

    Nova::Graphics::Renderer renderer(gfx);

    auto camera = renderer.createCamera();
    auto _object = renderer.createObject();
    auto object = _object.lock();
    object->loadMesh("assets/models/aircraft.obj");

    object->loadTexture("assets/textures/aircraft.png");

    camera.lock()->setPerspective(60.0f, 800.0f / 600.0f, 0.1f, 1000.0f);
    camera.lock()->setAspect(800.0f / 600.0f);

    auto scene = Nova::Graphics::Scene{};
    scene.camera = camera;
    scene.objects.push_back(_object);

    FlyCamera flyCam;
    flyCam.x = 0.0f; flyCam.y = 5.0f; flyCam.z = 10.0f;

    bool mouseCaptured = true;
    SDL_SetWindowRelativeMouseMode(win, true);

    bool running = true;
    Uint64 lastTicks = SDL_GetTicks();

    while (running) {
        Uint64 nowTicks = SDL_GetTicks();
        float dt = (nowTicks - lastTicks) / 1000.0f;
        lastTicks = nowTicks;

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                running = false;
            } else if (e.type == SDL_EVENT_WINDOW_RESIZED) {
                auto swap = renderer.getSwapchain();
                swap->handleRecreation();
                camera.lock()->setAspect(swap->getExtent().width / (float)swap->getExtent().height);
            } else if (e.type == SDL_EVENT_MOUSE_MOTION && mouseCaptured) {
                flyCam.yaw   += e.motion.xrel * flyCam.sensitivity;
                flyCam.pitch -= e.motion.yrel * flyCam.sensitivity;
                if (flyCam.pitch > 89.0f) flyCam.pitch = 89.0f;
                if (flyCam.pitch < -89.0f) flyCam.pitch = -89.0f;
            } else if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE) {
                mouseCaptured = !mouseCaptured;
                SDL_SetWindowRelativeMouseMode(win, mouseCaptured);
            }
        }

        // WASD movement
        const bool* keys = SDL_GetKeyboardState(nullptr);
        float fx, fy, fz;
        flyCam.getForward(fx, fy, fz);

        // right vector = forward x worldUp
        float rx = -fz, rz = fx;
        float len = sqrtf(rx * rx + rz * rz);
        if (len > 0.0001f) { rx /= len; rz /= len; }

        float moveSpeed = flyCam.speed * dt;
        if (keys[SDL_SCANCODE_W]) { flyCam.x += fx * moveSpeed; flyCam.y += fy * moveSpeed; flyCam.z += fz * moveSpeed; }
        if (keys[SDL_SCANCODE_S]) { flyCam.x -= fx * moveSpeed; flyCam.y -= fy * moveSpeed; flyCam.z -= fz * moveSpeed; }
        if (keys[SDL_SCANCODE_A]) { flyCam.x -= rx * moveSpeed; flyCam.z -= rz * moveSpeed; }
        if (keys[SDL_SCANCODE_D]) { flyCam.x += rx * moveSpeed; flyCam.z += rz * moveSpeed; }
        if (keys[SDL_SCANCODE_SPACE])  flyCam.y += moveSpeed;
        if (keys[SDL_SCANCODE_LCTRL])  flyCam.y -= moveSpeed;

        camera.lock()->setPosition(flyCam.x, flyCam.y, flyCam.z);
        camera.lock()->setTarget(flyCam.x + fx, flyCam.y + fy, flyCam.z + fz);

        renderer.step(scene);
    };

    renderer.shutdown();
    gfx.shutdown();

    SDL_Quit();

    return 0;
};