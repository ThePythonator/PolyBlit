#include "World.hpp"

namespace CWire3DWorld {
	World::World(CWire3DEntities::Camera* camera, uint8_t chunk_size, uint8_t chunk_spawn_dist) {
		this->camera = camera;

		this->chunk_size = chunk_size;
		this->chunk_spawn_distance = chunk_spawn_dist;

		chunk_generator = nullptr;
		chunk_destroyer = nullptr;
		triangle_renderer = nullptr;
	}

	void World::update() {
		// Calculate chunks we need to be loaded / unloaded
		int center_x = camera->get_position().x / chunk_size;
		int center_z = camera->get_position().z / chunk_size;

		std::vector<int2> chunks_to_load;

		for (int x = -chunk_spawn_distance; x < chunk_spawn_distance + 1; x++) {
			for (int z = -chunk_spawn_distance; z < chunk_spawn_distance + 1; z++) {
				chunks_to_load.push_back(int2{ center_x + x, center_z + z });
			}
		}

		// Remove chunks which are too far away
		//std::remove_if(loaded_chunks.begin(), loaded_chunks.end(), [chunks_to_load](Chunk &chunk) { return !std::count_if(chunks_to_load.begin(), chunks_to_load.end(), [chunk](int2 chunk_position) { return chunk.chunk_position == chunk_position; }); });

		// Move all chunks which need to be removed to the end
		std::vector<Chunk>::iterator start = std::partition(loaded_chunks.begin(), loaded_chunks.end(), [chunks_to_load](Chunk& chunk) { return std::count_if(chunks_to_load.begin(), chunks_to_load.end(), [chunk](int2 chunk_position) { return chunk.chunk_position == chunk_position; }); });
		// Deallocate chunks which will be removed
		std::for_each(start, loaded_chunks.end(), [&](Chunk& chunk) { destroy_chunk(chunk); });
		// Remove those chunks
		loaded_chunks.erase(start, loaded_chunks.end());


		// Create chunks
		for (int2 chunk_position : chunks_to_load) {
			if (!std::count_if(loaded_chunks.begin(), loaded_chunks.end(), [chunk_position](Chunk& chunk) { return chunk.chunk_position == chunk_position; })) {
				// Chunk isn't in loaded_chunks, need to load chunk
				generate_chunk(chunk_position);
			}
		}

		for (Chunk chunk : loaded_chunks) {
			for (uint16_t i = 0; i < chunk.triangles.size(); i++) {
				chunk.triangles[i].p1->projected_position = camera->project_point(chunk.triangles[i].p1->position);
				chunk.triangles[i].p2->projected_position = camera->project_point(chunk.triangles[i].p2->position);
				chunk.triangles[i].p3->projected_position = camera->project_point(chunk.triangles[i].p3->position);
			}
		}
	}

	void World::render() {
		if (triangle_renderer) {
			std::vector<Triangle> all_triangles;
			for (Chunk chunk : loaded_chunks) {
				all_triangles.insert(all_triangles.end(), chunk.triangles.begin(), chunk.triangles.end());
			}

			std::sort(all_triangles.begin(), all_triangles.end(), [&](const Triangle& a, const Triangle& b) {return std::min(length2(camera->get_position() - a.p1->position), std::min(length2(camera->get_position() - a.p2->position), length2(camera->get_position() - a.p3->position))) > std::min(length2(camera->get_position() - b.p1->position), std::min(length2(camera->get_position() - b.p2->position), length2(camera->get_position() - b.p3->position))); });

			for (const Triangle& triangle : all_triangles) {
				if (!camera->should_clip_sides(triangle.p1->projected_position, triangle.p2->projected_position, triangle.p3->projected_position)) {
					triangle_renderer(triangle);
				}
			}
		}
	}

	void World::generate_chunk(int2 chunk_position) {
		if (chunk_generator) {
			Chunk chunk = chunk_generator(chunk_position);
			loaded_chunks.push_back(chunk);
		}
	}

	void World::destroy_chunk(Chunk& chunk) {
		if (chunk_destroyer) {
			chunk_destroyer(chunk);
		}
	}

	void World::set_chunk_generator(Chunk (*chunk_generator) (int2)) {
		this->chunk_generator = chunk_generator;
	}

	void World::set_chunk_destroyer(void(*chunk_destroyer) (Chunk&)) {
		this->chunk_destroyer = chunk_destroyer;
	}

	void World::set_triangle_renderer(void (*triangle_renderer) (const Triangle&)) {
		this->triangle_renderer = triangle_renderer;
	}
}