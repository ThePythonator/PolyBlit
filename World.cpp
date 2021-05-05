#include "World.hpp"

namespace CWire3DWorld {
	World::World(CWire3DEntities::Camera* camera, uint8_t chunk_size, uint8_t chunk_spawn_dist) {
		this->camera = camera;

		this->chunk_size = chunk_size;
		this->chunk_spawn_distance = chunk_spawn_dist;

		chunk_generator = nullptr;
		triangle_renderer = nullptr;
	}

	void World::update() {
		// Calculate chunks we need to be loaded / unloaded
		int center_x = camera->get_position()->x / chunk_size;
		int center_z = camera->get_position()->z / chunk_size;

		std::vector<int2> chunks_to_load;

		for (int x = -chunk_spawn_distance; x < chunk_spawn_distance + 1; x++) {
			for (int z = -chunk_spawn_distance; z < chunk_spawn_distance + 1; z++) {
				chunks_to_load.push_back(int2{ center_x + x, center_z + z });
			}
		}

		for (int2 chunk_position : chunks_to_load) {
			if (!std::count_if(loaded_chunks.begin(), loaded_chunks.end(), [chunk_position](Chunk chunk) { return chunk.chunk_position == chunk_position; })) {
				// Chunk isn't in loaded_chunks, need to load chunk
				generate_chunk(chunk_position);
			}
		}

		// Remove chunks which are too far away
		std::remove_if(loaded_chunks.begin(), loaded_chunks.end(), [chunks_to_load](Chunk chunk) { return !std::count_if(chunks_to_load.begin(), chunks_to_load.end(), [chunk](int2 chunk_position) { return chunk.chunk_position == chunk_position; }); });

		// Remove all duplicate nodes generated from generate_chunk()
		//loaded_chunks.resize(std::distance(loaded_nodes.begin(), std::unique(loaded_nodes.begin(), loaded_nodes.end(), [](Node* n1, Node* n2) { return n1->position == n2->position; })));

		/*for (Node* node : loaded_nodes) {
			node->projected_position = camera->project_point(node->position);
		}*/
		for (Chunk chunk : loaded_chunks) {
			for (uint16_t i = 0; i < chunk.triangles.size(); i++) {
				chunk.triangles[i].p1->projected_position = camera->project_point(chunk.triangles[i].p1->position);
				chunk.triangles[i].p2->projected_position = camera->project_point(chunk.triangles[i].p2->position);
				chunk.triangles[i].p3->projected_position = camera->project_point(chunk.triangles[i].p3->position);
				//printf("%f\n", chunk.triangles[i].p1->projected_position.z);
			}
		}
	}

	void World::render() {
		if (triangle_renderer) {
			for (Chunk chunk : loaded_chunks) {
				for (Triangle triangle : chunk.triangles) {
					triangle_renderer(triangle);
				}
			}
		}
	}

	void World::generate_chunk(int2 chunk_position) {
		if (chunk_generator) {
			Chunk chunk = chunk_generator(chunk_position);
			loaded_chunks.push_back(chunk);
			/*for (const Triangle& triangle : chunk.triangles) {
				loaded_nodes.push_back(triangle.p1);
				loaded_nodes.push_back(triangle.p2);
				loaded_nodes.push_back(triangle.p3);
			}*/
		}
	}

	void World::set_chunk_generator(Chunk (*chunk_generator) (int2)) {
		this->chunk_generator = chunk_generator;
	}

	void World::set_triangle_renderer(void (*triangle_renderer) (Triangle)) {
		this->triangle_renderer = triangle_renderer;
	}
}