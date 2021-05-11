#include "PolyBlit.hpp"

// Globals

#ifdef TARGET_32BLIT_HW
const uint8_t CHUNK_LOAD_DIST = 3;
const uint8_t CHUNK_SIZE = 4;
#else
const uint8_t CHUNK_LOAD_DIST = 4;
const uint8_t CHUNK_SIZE = 8;
#endif

uint2 display_size{ 320, 240 };

CWire3DEntities::Camera camera = CWire3DEntities::Camera(display_size);
CWire3DWorld::World world = CWire3DWorld::World(&camera, CHUNK_SIZE, CHUNK_LOAD_DIST); // the product of the last two parameters must not be exceed 48, otherwise it is likely to be unplayable.

ButtonStates buttonStates = { 0 };

PerlinNoise::PerlinNoise perlin = PerlinNoise::PerlinNoise(31415);

CWire3DWorld::Chunk custom_chunk_generator(int2 chunk_position) {
    std::vector<CWire3DWorld::Triangle> triangles;

    for (int z = 0; z < world.chunk_size; z++) {
        for (int x = 0; x < world.chunk_size; x++) {
            CWire3DWorld::Triangle t1, t2;

            CWire3DWorld::Node* p1 = new CWire3DWorld::Node();
            CWire3DWorld::Node* p2 = new CWire3DWorld::Node();
            CWire3DWorld::Node* p3 = new CWire3DWorld::Node();
            CWire3DWorld::Node* p4 = new CWire3DWorld::Node();

            p1->position = float3{ (float)(chunk_position.x * world.chunk_size + x), 0.0f, (float)(chunk_position.y * world.chunk_size + z) };
            p2->position = float3{ (float)(chunk_position.x * world.chunk_size + x + 1), 0.0f, (float)(chunk_position.y * world.chunk_size + z) };
            p3->position = float3{ (float)(chunk_position.x * world.chunk_size + x), 0.0f, (float)(chunk_position.y * world.chunk_size + z + 1) };
            p4->position = float3{ (float)(chunk_position.x * world.chunk_size + x + 1), 0.0f, (float)(chunk_position.y * world.chunk_size + z + 1) };

            float h1, h2, h3, h4;

            h1 = ((float)perlin.accumulatedOctaveNoise2D_0_1(p1->position.x / 32.0, p1->position.z / 32.0, 1) - 0.4f) * 24.0f;
            h2 = ((float)perlin.accumulatedOctaveNoise2D_0_1(p2->position.x / 32.0, p2->position.z / 32.0, 1) - 0.4f) * 24.0f;
            h3 = ((float)perlin.accumulatedOctaveNoise2D_0_1(p3->position.x / 32.0, p3->position.z / 32.0, 1) - 0.4f) * 24.0f;
            h4 = ((float)perlin.accumulatedOctaveNoise2D_0_1(p4->position.x / 32.0, p4->position.z / 32.0, 1) - 0.4f) * 24.0f;

            h1 = std::max(0.0f, h1);
            h2 = std::max(0.0f, h2);
            h3 = std::max(0.0f, h3);
            h4 = std::max(0.0f, h4);

            p1->position.y = h1;
            p2->position.y = h2;
            p3->position.y = h3;
            p4->position.y = h4;

            t1.p1 = p1;
            t1.p2 = p2;
            t1.p3 = p3;

            t2.p1 = p2;
            t2.p2 = p3;
            t2.p3 = p4;

            float c1 = h1 + h2 + h3;
            float c2 = h2 + h3 + h4;

            t1.colour = byte3{ (uint8_t)(rand() % 20 + 10), (uint8_t)(std::min(255.0f, 220.0f + c1)), (uint8_t)(c1 + 20) };
            t2.colour = byte3{ (uint8_t)(rand() % 20 + 10), (uint8_t)(std::min(255.0f, 230.0f + c2)), (uint8_t)(c2 + 25) };

            if (!h1 && !h2 && !h3) {
                t1.colour = byte3{ (uint8_t)(rand() % 10 + 5), (uint8_t)(rand() % 10 + 30), (uint8_t)(240) };
            }
            if (!h2 && !h3 && !h4) {
                t2.colour = byte3{ (uint8_t)(rand() % 10 + 5), (uint8_t)(rand() % 10 + 35), (uint8_t)(220) };
            }

            triangles.push_back(t1);
            triangles.push_back(t2);
        }
    }

    CWire3DWorld::Chunk chunk;
    chunk.chunk_position = chunk_position;
    chunk.triangles = triangles;

    return chunk;
}

void custom_chunk_destroyer(CWire3DWorld::Chunk &chunk) {
    //printf("%i, %i\n", chunk.chunk_position.x, chunk.chunk_position.y);
    for (uint16_t i = 0; i < chunk.triangles.size(); i += 2) {
        delete chunk.triangles[i].p1;
        delete chunk.triangles[i].p2;
        delete chunk.triangles[i].p3;
        delete chunk.triangles[i + 1].p3;
    }
}

void custom_triangle_renderer(const CWire3DWorld::Triangle& triangle) {
    if (triangle.p1->projected_position.z > 0.0f && triangle.p2->projected_position.z > 0.0f && triangle.p3->projected_position.z > 0.0f) {
        screen.pen = Pen(triangle.colour.x, triangle.colour.y, triangle.colour.z);

        screen.triangle(
            Point(triangle.p1->projected_position.x, triangle.p1->projected_position.y),
            Point(triangle.p2->projected_position.x, triangle.p2->projected_position.y),
            Point(triangle.p3->projected_position.x, triangle.p3->projected_position.y)
        );
    }
}

///////////////////////////////////////////////////////////////////////////
//
// init()
//
// setup your game here
//
void init() {
    set_screen_mode(ScreenMode::hires);

    world.set_chunk_generator(custom_chunk_generator);
    world.set_chunk_destroyer(custom_chunk_destroyer);
    world.set_triangle_renderer(custom_triangle_renderer);

    camera.translate(float3{ 0.0f, 4.0f, 0.0f });
    camera.set_clip(float2{ 0.05f, 32.0f });
}

///////////////////////////////////////////////////////////////////////////
//
// render(time)
//
// This function is called to perform rendering of the game. time is the 
// amount if milliseconds elapsed since the start of your game
//
void render(uint32_t time) {

    // clear the screen -- screen is a reference to the frame buffer and can be used to draw all things with the 32blit
    screen.clear();

    screen.alpha = 255;
    screen.mask = nullptr;

    world.render();


#ifdef TARGET_32BLIT_HW
    // memory stats
    extern char _sbss, _end, __ltdc_start;

    auto static_used = &_end - &_sbss;
    auto heap_total = &__ltdc_start - &_end;
    auto heap_used = mallinfo().uordblks;

    auto total_ram = static_used + heap_total;

    screen.pen = { 255, 255, 255 };
    char buf[100];
    snprintf(buf, sizeof(buf), "Mem: %i + %i / %i", static_used, heap_used, total_ram);
    screen.text(buf, minimal_font, { 10, 10 }, true, TextAlign::center_center);
#endif

    screen.pen = Pen(100, 140, 200);
}

///////////////////////////////////////////////////////////////////////////
//
// update(time)
//
// This is called to update your game state. time is the 
// amount if milliseconds elapsed since the start of your game
//
void update(uint32_t time) {
    // Update buttonStates
    if (buttons & Button::A) {
        if (buttonStates.A == 2) {
            buttonStates.A = 1;
        }
        else if (buttonStates.A == 0) {
            buttonStates.A = 2;
        }
    }
    else {
        buttonStates.A = 0;
    }

    if (buttons & Button::B) {
        if (buttonStates.B == 2) {
            buttonStates.B = 1;
        }
        else if (buttonStates.B == 0) {
            buttonStates.B = 2;
        }
    }
    else {
        buttonStates.B = 0;
    }

    if (buttons & Button::X) {
        if (buttonStates.X == 2) {
            buttonStates.X = 1;
        }
        else if (buttonStates.X == 0) {
            buttonStates.X = 2;
        }
    }
    else {
        buttonStates.X = 0;
    }

    if (buttons & Button::Y) {
        if (buttonStates.Y == 2) {
            buttonStates.Y = 1;
        }
        else if (buttonStates.Y == 0) {
            buttonStates.Y = 2;
        }
    }
    else {
        buttonStates.Y = 0;
    }

    if (buttons & Button::DPAD_UP) {
        if (buttonStates.UP == 2) {
            buttonStates.UP = 1;
        }
        else if (buttonStates.UP == 0) {
            buttonStates.UP = 2;
        }
    }
    else {
        buttonStates.UP = 0;
    }

    if (buttons & Button::DPAD_DOWN) {
        if (buttonStates.DOWN == 2) {
            buttonStates.DOWN = 1;
        }
        else if (buttonStates.DOWN == 0) {
            buttonStates.DOWN = 2;
        }
    }
    else {
        buttonStates.DOWN = 0;
    }

    if (buttons & Button::DPAD_LEFT) {
        if (buttonStates.LEFT == 2) {
            buttonStates.LEFT = 1;
        }
        else if (buttonStates.LEFT == 0) {
            buttonStates.LEFT = 2;
        }
    }
    else {
        buttonStates.LEFT = 0;
    }

    if (buttons & Button::DPAD_RIGHT) {
        if (buttonStates.RIGHT == 2) {
            buttonStates.RIGHT = 1;
        }
        else if (buttonStates.RIGHT == 0) {
            buttonStates.RIGHT = 2;
        }
    }
    else {
        buttonStates.RIGHT = 0;
    }

    if (buttonStates.UP) {
        camera.move(float3{ 0.0f, 0.0f, 0.1f });
    }
    if (buttonStates.DOWN) {
        camera.move(float3{ 0.0f, 0.0f, -0.1f });
    }

    if (buttonStates.RIGHT) {
        camera.move(float3{ 0.1f, 0.0f, 0.0f });
    }
    if (buttonStates.LEFT) {
        camera.move(float3{ -0.1f, 0.0f, 0.0f });
    }

    if (buttonStates.X) {
        camera.move(float3{ 0.0f, -0.1f, 0.0f });
    }
    if (buttonStates.B) {
        camera.move(float3{ 0.0f, 0.1f, 0.0f });
    }

    if (joystick.x < -0.1f || joystick.x > 0.1f) {
        camera.rotate(float3{ 0.0f, -joystick.x / 40.0f, 0.0f});
    }

    if (joystick.y < -0.1f || joystick.y > 0.1f) {
        float3 angle = camera.get_angle();
        camera.set_angle(float3{ std::min(CWire3DUtilities::half_pi, std::max(-CWire3DUtilities::half_pi, angle.x - joystick.y / 40.0f)), angle.y , angle.z });
    }

    world.update();
    camera.update_rotation();
}