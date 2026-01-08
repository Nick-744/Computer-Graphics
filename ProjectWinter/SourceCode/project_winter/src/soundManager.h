#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include "miniaudio.h" // Audio
#include <iostream>

using namespace std;

// ===< Audio System >=== //
extern ma_decoding_backend_vtable g_ma_decoding_backend_vtable_stb_vorbis;
struct SoundManager
{
	ma_engine engine;
	ma_resource_manager resourceManager; // Keep the manager alive!
	bool initialized = false;

	void init()
	{
		// Define the backend list
		ma_decoding_backend_vtable* pBackends[] = { &g_ma_decoding_backend_vtable_stb_vorbis };

		// Initialize the Resource Manager first with OGG support...
		ma_resource_manager_config rmConfig     = ma_resource_manager_config_init();
		rmConfig.ppCustomDecodingBackendVTables = pBackends;
		rmConfig.customDecodingBackendCount     = 1;

		if (ma_resource_manager_init(&rmConfig, &resourceManager) != MA_SUCCESS)
		{
			cout << "Failed to init Resource Manager!" << endl;
			return;
		}

		// Initialize the Engine using the Resource Manager
		ma_engine_config engineConfig = ma_engine_config_init();
		engineConfig.pResourceManager = &resourceManager;

		if (ma_engine_init(&engineConfig, &engine) == MA_SUCCESS)
		{
			initialized = true;
			cout << "Audio Engine Initialized with OGG Support!" << endl;
		}
		else
		{
			cout << "Engine Init Failed!" << endl;
			ma_resource_manager_uninit(&resourceManager);
		}
	}

	void play(const char* filepath)
	{
		if (!initialized) return;
		ma_engine_play_sound(&engine, filepath, NULL);
	}

	void cleanup()
	{
		if (initialized)
		{
			ma_engine_uninit(&engine);
			ma_resource_manager_uninit(&resourceManager); // Cleanup both
		}
	}
};

#endif
