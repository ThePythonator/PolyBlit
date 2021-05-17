#include "PolyBlit.hpp"

// Globals

#ifdef TARGET_32BLIT_HW
const uint8_t CHUNK_LOAD_DIST = 2;
const uint8_t CHUNK_SIZE = 8;
#else
const uint8_t CHUNK_LOAD_DIST = 5;
const uint8_t CHUNK_SIZE = 8;
#endif

const float MOUNTAIN_LEVEL = 7.0f;
const float MIN_TREE_GAP = 4.0f;

uint2 display_size{ 320, 240 };

CWire3DEntities::Camera camera = CWire3DEntities::Camera(display_size);
CWire3DWorld::World world = CWire3DWorld::World(&camera, CHUNK_SIZE, CHUNK_LOAD_DIST); // the product of the last two parameters must not be exceed 48, otherwise it is likely to be unplayable.

ButtonStates buttonStates = { 0 };

PerlinNoise::PerlinNoise perlin = PerlinNoise::PerlinNoise(31415);

float get_terrain_height(float x, float z) {
    return ((float)perlin.accumulatedOctaveNoise2D_0_1(x / 32.0, z / 32.0, 1) - 0.4f) * 26.0f;
}

CWire3DWorld::Chunk custom_chunk_generator(int2 chunk_position) {
    std::vector<CWire3DWorld::Triangle> triangles;

    // Generate terrain
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

            h1 = get_terrain_height(p1->position.x, p1->position.z);
            h2 = get_terrain_height(p2->position.x, p2->position.z);
            h3 = get_terrain_height(p3->position.x, p3->position.z);
            h4 = get_terrain_height(p4->position.x, p4->position.z);

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

            uint8_t c1 = (uint8_t)(h1 + h2 + h3);
            uint8_t c2 = (uint8_t)(h2 + h3 + h4);

            uint8_t s1 = std::min(255, 210 + c1);
            uint8_t s2 = std::min(255, 220 + c2);

            uint8_t s3 = std::min(255, c1 * 9) - 3;
            uint8_t s4 = std::min(255, c2 * 9);

            t1.colour = byte3{ (uint8_t)(c1 * 2), s1, (uint8_t)std::min(255, c1 * 4) };
            t2.colour = byte3{ (uint8_t)(c2 * 2), s2, (uint8_t)std::min(255, c2 * 4) };

            if (!h1 && !h2 && !h3) {
                t1.colour = byte3{ (uint8_t)(rand() % 10 + 5), (uint8_t)(rand() % 5 + 30), (uint8_t)(245) };
            }
            if (!h2 && !h3 && !h4) {
                t2.colour = byte3{ (uint8_t)(rand() % 10 + 5), (uint8_t)(rand() % 5 + 35), (uint8_t)(230) };
            }

            if (h1 > MOUNTAIN_LEVEL && h2 > MOUNTAIN_LEVEL && h3 > MOUNTAIN_LEVEL) {
                t1.colour = byte3{ s3, s3, s3 };
            }
            if (h2 > MOUNTAIN_LEVEL && h3 > MOUNTAIN_LEVEL && h4 > MOUNTAIN_LEVEL) {
                t2.colour = byte3{ s4, s4, s4 };
            }

            triangles.push_back(t1);
            triangles.push_back(t2);
        }
    }

    // Generate trees
    std::vector<float3> tree_bases;
    uint8_t count = rand() % (CHUNK_SIZE / 4);//(rand() % (1 + CHUNK_SIZE / 2)
    for (int i = 0; i < count; i++) {
        float3 base{ (float)(chunk_position.x * world.chunk_size + rand() % CHUNK_SIZE), 0.0f, (float)(chunk_position.y * world.chunk_size + rand() % CHUNK_SIZE) };

        base.y = get_terrain_height(base.x, base.z);

        // Don't allow trees to spawn in certain places.

        for (float3 tree_base : tree_bases) {
            // Trees can't spawn too close to another tree.
            // Currently this doesn't take into account if trees are less than MIN_TREE_GAP to a tree in a different chunk.
            if (std::abs(tree_base.x - base.x) < MIN_TREE_GAP && std::abs(tree_base.z - base.z) < MIN_TREE_GAP) {
                continue;
            }
        }

        // Trees can't spawn on water or on mountains
        if (base.y <= 0.0f || base.y > MOUNTAIN_LEVEL) {
            continue;
        }

        tree_bases.push_back(base);

        // Tree trunk

        CWire3DWorld::Node* p1 = new CWire3DWorld::Node();
        CWire3DWorld::Node* p2 = new CWire3DWorld::Node();
        CWire3DWorld::Node* p3 = new CWire3DWorld::Node();
        CWire3DWorld::Node* p4 = new CWire3DWorld::Node();
        CWire3DWorld::Node* p5 = new CWire3DWorld::Node();
        CWire3DWorld::Node* p6 = new CWire3DWorld::Node();
        CWire3DWorld::Node* p7 = new CWire3DWorld::Node();
        CWire3DWorld::Node* p8 = new CWire3DWorld::Node();

        float height1 = 4.0f + (rand() % 15) / 10.0f;

        p1->position = float3{ base.x - 0.4f, base.y, base.z - 0.4f };
        p2->position = float3{ base.x + 0.4f, base.y, base.z - 0.4f };
        p3->position = float3{ base.x - 0.4f, base.y, base.z + 0.4f };
        p4->position = float3{ base.x + 0.4f, base.y, base.z + 0.4f };

        p5->position = float3{ base.x - 0.15f, base.y + height1, base.z };
        p6->position = float3{ base.x + 0.15f, base.y + height1, base.z };
        p7->position = float3{ base.x, base.y + height1, base.z - 0.15f };
        p8->position = float3{ base.x, base.y + height1, base.z + 0.15f };

        CWire3DWorld::Triangle t1, t2, t3, t4, t5, t6, t7, t8;

        byte3 c1{ (uint8_t)(140 + rand() % 15), (uint8_t)(60 + rand() % 15), (uint8_t)(10 + rand() % 20) };
        byte3 c2{ (uint8_t)(140 + rand() % 15), (uint8_t)(60 + rand() % 15), (uint8_t)(10 + rand() % 20) };
        byte3 c3{ (uint8_t)(140 + rand() % 15), (uint8_t)(60 + rand() % 15), (uint8_t)(10 + rand() % 20) };
        byte3 c4{ (uint8_t)(140 + rand() % 15), (uint8_t)(60 + rand() % 15), (uint8_t)(10 + rand() % 20) };

        t1.colour = t5.colour = c1;
        t2.colour = t6.colour = c2;
        t3.colour = t7.colour = c3;
        t4.colour = t8.colour = c4;

        t1.p1 = p1;
        t1.p2 = p2;
        t1.p3 = p7;

        t2.p1 = p6;
        t2.p2 = p2;
        t2.p3 = p7;

        t3.p1 = p2;
        t3.p2 = p4;
        t3.p3 = p6;

        t4.p1 = p6;
        t4.p2 = p4;
        t4.p3 = p8;

        t5.p1 = p3;
        t5.p2 = p4;
        t5.p3 = p8;

        t6.p1 = p3;
        t6.p2 = p8;
        t6.p3 = p5;

        t7.p1 = p3;
        t7.p2 = p1;
        t7.p3 = p5;

        t8.p1 = p7;
        t8.p2 = p1;
        t8.p3 = p5;

        triangles.push_back(t1);
        triangles.push_back(t2);
        triangles.push_back(t3);
        triangles.push_back(t4);
        triangles.push_back(t5);
        triangles.push_back(t6);
        triangles.push_back(t7);
        triangles.push_back(t8);

        // Tree top

        CWire3DWorld::Node* p9 = new CWire3DWorld::Node();
        CWire3DWorld::Node* p10 = new CWire3DWorld::Node();
        CWire3DWorld::Node* p11 = new CWire3DWorld::Node();
        CWire3DWorld::Node* p12 = new CWire3DWorld::Node();
        CWire3DWorld::Node* p13 = new CWire3DWorld::Node();
        CWire3DWorld::Node* p14 = new CWire3DWorld::Node();
        CWire3DWorld::Node* p15 = new CWire3DWorld::Node();
        CWire3DWorld::Node* p16 = new CWire3DWorld::Node();

        float height2 = 3.0f + (rand() % 8) / 10.0f;
        float height2_w = height2 / 2.0f - 0.3f;

        p9->position = float3{ base.x - height2_w, base.y + height1, base.z - height2_w };
        p10->position = float3{ base.x + height2_w, base.y + height1, base.z - height2_w };
        p11->position = float3{ base.x - height2_w, base.y + height1, base.z + height2_w };
        p12->position = float3{ base.x + height2_w, base.y + height1, base.z + height2_w };

        p13->position = float3{ base.x - height2_w, base.y + height1 + height2, base.z };
        p14->position = float3{ base.x + height2_w, base.y + height1 + height2, base.z };
        p15->position = float3{ base.x, base.y + height1 + height2, base.z - height2_w };
        p16->position = float3{ base.x, base.y + height1 + height2, base.z + height2_w };

        CWire3DWorld::Triangle t9, t10, t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;

        byte3 c5{ (uint8_t)(50 + rand() % 15), (uint8_t)(180 + rand() % 20), (uint8_t)(40 + rand() % 20) };
        byte3 c6{ (uint8_t)(50 + rand() % 15), (uint8_t)(180 + rand() % 20), (uint8_t)(40 + rand() % 20) };
        byte3 c7{ (uint8_t)(50 + rand() % 15), (uint8_t)(180 + rand() % 20), (uint8_t)(40 + rand() % 20) };
        byte3 c8{ (uint8_t)(50 + rand() % 15), (uint8_t)(180 + rand() % 20), (uint8_t)(40 + rand() % 20) };

        t9.colour = t13.colour = t17.colour = c5;
        t10.colour = t14.colour = t18.colour = c6;
        t11.colour = t15.colour = t19.colour = c7;
        t12.colour = t16.colour = t20.colour = c8;

        t9.p1 = p9;
        t9.p2 = p10;
        t9.p3 = p15;

        t10.p1 = p14;
        t10.p2 = p10;
        t10.p3 = p15;

        t11.p1 = p10;
        t11.p2 = p12;
        t11.p3 = p14;

        t12.p1 = p14;
        t12.p2 = p12;
        t12.p3 = p16;

        t13.p1 = p11;
        t13.p2 = p12;
        t13.p3 = p16;

        t14.p1 = p11;
        t14.p2 = p16;
        t14.p3 = p13;

        t15.p1 = p11;
        t15.p2 = p9;
        t15.p3 = p13;

        t16.p1 = p15;
        t16.p2 = p9;
        t16.p3 = p13;

        // Top and bottom of leaves

        t17.p1 = p9;
        t17.p2 = p10;
        t17.p3 = p11;

        t18.p1 = p10;
        t18.p2 = p11;
        t18.p3 = p12;

        t19.p1 = p13;
        t19.p2 = p14;
        t19.p3 = p15;

        t20.p1 = p14;
        t20.p2 = p13;
        t20.p3 = p16;

        triangles.push_back(t9);
        triangles.push_back(t10);
        triangles.push_back(t11);
        triangles.push_back(t12);
        triangles.push_back(t13);
        triangles.push_back(t14);
        triangles.push_back(t15);
        triangles.push_back(t16);

        triangles.push_back(t17);
        triangles.push_back(t18);
        triangles.push_back(t19);
        triangles.push_back(t20);
    }

    CWire3DWorld::Chunk chunk;
    chunk.chunk_position = chunk_position;
    chunk.triangles = triangles;

    return chunk;
}

void custom_chunk_destroyer(CWire3DWorld::Chunk &chunk) {
    // Assumes nodes in chunk are not used anywhere else
    std::vector<CWire3DWorld::Node*> nodes_deleted;

    for (CWire3DWorld::Triangle& triangle : chunk.triangles) {
        if (!std::count(nodes_deleted.begin(), nodes_deleted.end(), triangle.p1)) {
            nodes_deleted.push_back(triangle.p1);
            delete triangle.p1;
        }

        if (!std::count(nodes_deleted.begin(), nodes_deleted.end(), triangle.p2)) {
            nodes_deleted.push_back(triangle.p2);
            delete triangle.p2;
        }

        if (!std::count(nodes_deleted.begin(), nodes_deleted.end(), triangle.p3)) {
            nodes_deleted.push_back(triangle.p3);
            delete triangle.p3;
        }
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

    camera.translate(float3{ 0.0f, 8.0f, 0.0f });
    camera.set_clip(float2{ 0.02f, 32.0f });
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
    screen.text(buf, minimal_font, { 5, 5 }, true, TextAlign::top_left);
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