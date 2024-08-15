#include "world/level/storage/Region.h"

#include <miniz/miniz.h>
#include <mpack/mpack.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "client/Crash.h"
#include "client/gui/DebugUI.h"
#include "util/Paths.h"
#include "world/World.h"

static const int sectorSize			   = 1024;
static const int headerSize			   = sizeof(ChunkInfo) * REGION_SIZE * REGION_SIZE;
static mpack_node_data_t* nodeDataPool = NULL;
static const int nodeDataPoolSize	   = 1024;
const int decompressBufferSize		   = sizeof(Chunk) * 4 + headerSize;
static char* decompressBuffer;
static const int fileBufferSize = sizeof(Chunk) * 4 + headerSize;
static char* fileBuffer;

void Region_InitPools() {
	nodeDataPool	 = (mpack_node_data_t*)malloc(sizeof(mpack_node_data_t) * nodeDataPoolSize);
	fileBuffer		 = malloc(fileBufferSize);
	decompressBuffer = malloc(decompressBufferSize);
}

void Region_DeinitPools() {
	free(nodeDataPool);
	free(decompressBuffer);
	free(fileBuffer);
}

void Region_Init(Region* region, int x, int z) {
	region->x = x;
	region->z = z;

	vec_init(&region->sectors);

	char buffer[256];
	sprintf(buffer, "%s/regions/r.%d.%d.dat", gWorld->path, x, z);

	// Open the .dat file
	region->dataFile = fopen(buffer, "r+b");
	if (region->dataFile == NULL) {
		// Create a new .dat file if it doesn't exist
		region->dataFile = fopen(buffer, "w+b");
	}

	if (region->dataFile == NULL) {
		Crash("World Path invalid, internal error. Unable to load Region %d.%d.\n\nPath name: %s\nBuffer String: %s\nBuffer Address: 0x%x",
			  x, z, gWorld->path, buffer, buffer);
	}

	// Check if the .dat file already has metadata
	fseek(region->dataFile, 0, SEEK_END);
	long fileSize = ftell(region->dataFile);
	fseek(region->dataFile, 0, SEEK_SET);

	if (fileSize > headerSize) {
		// File exists and has metadata, read it
		fread(fileBuffer, headerSize, 1, region->dataFile);

		// Parse raw metadata
		for (int i = 0; i < REGION_SIZE * REGION_SIZE; i++) {
			ChunkInfo* chunkInfo						   = (ChunkInfo*)(fileBuffer + i * sizeof(ChunkInfo));
			region->grid[i % REGION_SIZE][i / REGION_SIZE] = *chunkInfo;

			if (chunkInfo->actualSize > 0) {
				while (chunkInfo->position + chunkInfo->blockSize > region->sectors.length) {
					vec_push(&region->sectors, false);
				}
				for (int j = 0; j < chunkInfo->blockSize; j++)
					region->sectors.data[chunkInfo->position + j] = true;
			}
		}
	} else {
		// No metadata, initialize
		memset(region->grid, 0x0, sizeof(region->grid));
	}
}

void Region_Deinit(Region* region) {
	Region_SaveIndex(region);
	vec_deinit(&region->sectors);
	fclose(region->dataFile);
}

void Region_SaveIndex(Region* region) {
	if (!region->wasUpdated)
		return;

	char buffer[256];
	sprintf(buffer, "%s/regions/r.%d.%d.dat", gWorld->path, region->x, region->z);

	memcpy(fileBuffer, region->grid, sizeof(region->grid));

	FILE* file = fopen(buffer, "r+b");
	if (file == NULL) {
		Crash("Failed to open Region file %d.%d for writing metadata %s", buffer, region->x, region->z);
	}

	fseek(file, 0, SEEK_SET);
	fwrite(fileBuffer, fileBufferSize, 1, file);

	fclose(file);
}

static u32 reserveSectors(Region* region, int amount) {
	int amountFulfilled = 0;
	int startValue		= -1;
	for (int i = 0; i < region->sectors.length; i++) {
		if (!region->sectors.data[i]) {
			if (startValue == -1)
				startValue = i;
			amountFulfilled++;
		} else {
			amountFulfilled = 0;
			startValue		= -1;
		}
		if (amountFulfilled == amount) {
			for (int i = 0; i < amount; i++)
				region->sectors.data[startValue + i] = true;
			return startValue;
		}
	}
	for (int i = 0; i < amount; i++)
		vec_push(&region->sectors, true);
	return region->sectors.length - amount;
}

static void freeSectors(Region* region, u32 address, u8 size) {
	for (size_t i = 0; i < size; i++) {
		region->sectors.data[address + i] = false;
	}
}

void Region_SaveChunk(Region* region, Chunk* chunk) {
	int x = ChunkToLocalRegionCoord(chunk->x);
	int z = ChunkToLocalRegionCoord(chunk->z);

	if (region->grid[x][z].revision == chunk->revision)
		return;

	region->wasUpdated = true;

	// Initialize MPack writer for chunk data
	mpack_writer_t writer;
	mpack_writer_init(&writer, decompressBuffer, decompressBufferSize);

	// Write chunk data to MPack
	mpack_start_map(&writer, 4);

	mpack_write_cstr(&writer, "clusters");
	mpack_start_array(&writer, CLUSTER_PER_CHUNK);
	for (int i = 0; i < CLUSTER_PER_CHUNK; i++) {
		bool empty = Cluster_IsEmpty(&chunk->clusters[i]);

		mpack_start_map(&writer, empty ? 2 : 4);

		if (!empty) {
			mpack_write_cstr(&writer, "blocks");
			mpack_write_bin(&writer, (char*)chunk->clusters[i].blocks, sizeof(chunk->clusters[i].blocks));
			mpack_write_cstr(&writer, "metadataLight");
			mpack_write_bin(&writer, (char*)chunk->clusters[i].metadataLight, sizeof(chunk->clusters[i].metadataLight));
		}

		mpack_write_cstr(&writer, "revision");
		mpack_write_u32(&writer, chunk->clusters[i].revision);

		mpack_write_cstr(&writer, "empty");
		mpack_write_bool(&writer, empty);

		mpack_finish_map(&writer);
	}
	mpack_finish_array(&writer);

	mpack_write_cstr(&writer, "genProgress");
	mpack_write_u8(&writer, chunk->genProgress);

	mpack_write_cstr(&writer, "heightmap");
	mpack_write_bin(&writer, (char*)chunk->heightmap, sizeof(chunk->heightmap));

	mpack_write_cstr(&writer, "biome");
	mpack_write_u8(&writer, chunk->biome);

	mpack_finish_map(&writer);

	// Finish MPack writer
	mpack_error_t err = mpack_writer_destroy(&writer);
	if (err != mpack_ok) {
		Crash("MPack error %d while saving chunk %d.%d.%d\nPath: %s", err, x, z, chunk->x, region->dataFile);
	}

	// Compress the MPack buffer
	mz_ulong compressedSize = fileBufferSize;
	if (compress((u8*)fileBuffer, &compressedSize, (u8*)decompressBuffer, mpack_writer_buffer_used(&writer)) != Z_OK) {
		DebugUI_Log("Error while compressing chunk %d.%d.%d", chunk->x, chunk->z);
		return;
	}

	if (region->grid[x][z].actualSize > 0) {
		freeSectors(region, region->grid[x][z].position, region->grid[x][z].blockSize);
	}

	// Reserve sectors and write compressed chunk data
	u32 startPosition = reserveSectors(region, compressedSize);
	fseek(region->dataFile, headerSize + startPosition * sectorSize, SEEK_SET);
	fwrite(fileBuffer, compressedSize, 1, region->dataFile);
	fflush(region->dataFile);

	// Update region info
	region->grid[x][z].position		  = startPosition;
	region->grid[x][z].compressedSize = compressedSize;
	region->grid[x][z].actualSize	  = mpack_writer_buffer_used(&writer);
	region->grid[x][z].revision		  = chunk->revision;
}

void Region_LoadChunk(Region* region, Chunk* chunk) {
	int x				= ChunkToLocalRegionCoord(chunk->x);
	int z				= ChunkToLocalRegionCoord(chunk->z);
	ChunkInfo chunkInfo = region->grid[x][z];

	if (chunkInfo.actualSize <= 0) {
		// If the chunk is not present or has no data, return early.
		return;
	}
	// Set the file position to read chunk data, accounting for header and sector offset.
	fseek(region->dataFile, headerSize + chunkInfo.position * sectorSize, SEEK_SET);

	// Read the compressed chunk data into fileBuffer.
	if (fread(fileBuffer, 1, chunkInfo.compressedSize, region->dataFile) != chunkInfo.compressedSize) {
		Crash("Read chunk data size isn't equal to the expected size for chunk %d.%d.%d", chunk->x, chunk->z, x);
	}

	// Decompress the chunk data.
	mz_ulong uncompressedSize = decompressBufferSize;
	if (uncompress((u8*)decompressBuffer, &uncompressedSize, (u8*)fileBuffer, chunkInfo.compressedSize) != Z_OK) {
		DebugUI_Log("Error while decompressing chunk data for World at Region %d.%d Chunk %d.%d", region->x, region->z, chunk->x, chunk->z);
		return;
	}

	// Initialize MPack tree for reading decompressed data.
	mpack_tree_t tree;
	mpack_tree_init_pool(&tree, decompressBuffer, uncompressedSize, nodeDataPool, nodeDataPoolSize);
	mpack_node_t root = mpack_tree_root(&tree);

	// Load chunk data from MPack.
	mpack_node_t clusters = mpack_node_map_cstr(root, "clusters");
	for (int i = 0; i < CLUSTER_PER_CHUNK; i++) {
		mpack_node_t cluster = mpack_node_array_at(clusters, i);

		chunk->clusters[i].revision = mpack_node_u32(mpack_node_map_cstr(cluster, "revision"));

		mpack_node_t emptyNode = mpack_node_map_cstr_optional(cluster, "empty");
		if (mpack_node_type(emptyNode) != mpack_type_nil) {
			chunk->clusters[i].emptyRevision = chunk->clusters[i].revision;
			chunk->clusters[i].empty		 = mpack_node_bool(emptyNode);
		} else {
			chunk->clusters[i].emptyRevision = 0;
			chunk->clusters[i].empty		 = false;
		}

		mpack_node_t blocksNode = mpack_node_map_cstr_optional(cluster, "blocks");
		if (mpack_node_type(blocksNode) == mpack_type_bin) {
			memcpy(chunk->clusters[i].blocks, mpack_node_data(blocksNode), sizeof(chunk->clusters[i].blocks));
		}
		mpack_node_t metadataNode = mpack_node_map_cstr_optional(cluster, "metadataLight");
		if (mpack_node_type(metadataNode) == mpack_type_bin) {
			memcpy(chunk->clusters[i].metadataLight, mpack_node_data(metadataNode), sizeof(chunk->clusters[i].metadataLight));
		}
	}

	// Read additional chunk data.
	chunk->genProgress = mpack_node_u8(mpack_node_map_cstr(root, "genProgress"));
	chunk->biome	   = mpack_node_u8(mpack_node_map_cstr(root, "biome"));

	mpack_node_t heightmapNode = mpack_node_map_cstr(root, "heightmap");
	if (mpack_node_type(heightmapNode) != mpack_type_nil) {
		memcpy(chunk->heightmap, mpack_node_data(heightmapNode), sizeof(chunk->heightmap));
		chunk->heightmapRevision = chunkInfo.revision;
	} else {
		chunk->heightmapRevision = 0;
	}

	// Clean up MPack tree.
	mpack_error_t err = mpack_tree_destroy(&tree);
	if (err != mpack_ok) {
		Crash("MPack error %d while loading chunk %d.%d from region", err, chunk->x, chunk->z);
	}

	// Update chunk revision.
	chunk->revision = chunkInfo.revision;
}
