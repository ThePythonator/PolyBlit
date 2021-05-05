#pragma once

#include "linalg.h"

#include <algorithm>
#include <vector>

#include "Camera.hpp"

using namespace linalg::aliases;

namespace CWire3DWorld {
	struct Node {
		float3 position;
		float3 projected_position{ 0.0f, 0.0f, 0.0f };
	};

	struct Triangle {
		Node* p1, * p2, * p3;

		byte3 colour;
	};

	struct Chunk {
		int2 chunk_position;

		std::vector<Triangle> triangles;
	};

	class World {
	public:
		uint8_t chunk_size, chunk_spawn_distance;
		
		World() : World(nullptr) { };
		World(CWire3DEntities::Camera* camera, uint8_t chunk_size = 8, uint8_t chunk_spawn_dist = 4);

		void update();
		void render();

		void generate_chunk(int2 chunk_position);
		void set_chunk_generator(Chunk (*chunk_generator) (int2));

		void set_triangle_renderer(void (*triangle_renderer) (Triangle));

	protected:
		CWire3DEntities::Camera* camera;

		Chunk (*chunk_generator) (int2);

		void (*triangle_renderer) (Triangle);

		std::vector<Chunk> loaded_chunks;
		//std::vector<Node*> loaded_nodes;
	};
}