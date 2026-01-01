// audio_implementation.cpp
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
#include <windows.h>
#endif

#include <stdlib.h>

// Include ONLY the header definitions of stb_vorbis
#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c" 

// Define the miniaudio implementation
// This now sees the stb_vorbis types and enables ma_stbvorbis...
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

// Include the actual code logic for stb_vorbis!
#undef STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

// Correct VTable implementation for stb_vorbis...
static ma_result ma_decoding_backend_init_stb_vorbis(
    void*        pUserData,
    ma_read_proc onRead,
    ma_seek_proc onSeek,
    ma_tell_proc onTell,
    void*        pReadSeekTellUserData,
    const ma_decoding_backend_config* pConfig,
    const ma_allocation_callbacks*    pAllocationCallbacks,
    ma_data_source** ppBackend
)
{
    ma_stbvorbis* pVorbis = (ma_stbvorbis*) ma_malloc(sizeof(*pVorbis), pAllocationCallbacks);
    if (pVorbis == NULL) return MA_OUT_OF_MEMORY;

    ma_result result = ma_stbvorbis_init(onRead, onSeek, onTell, pReadSeekTellUserData, pConfig, pAllocationCallbacks, pVorbis);
    if (result != MA_SUCCESS)
    {
        ma_free(pVorbis, pAllocationCallbacks);
        return result;
    }

    *ppBackend = pVorbis;

    return MA_SUCCESS;
}

ma_decoding_backend_vtable g_ma_decoding_backend_vtable_stb_vorbis = {
    ma_decoding_backend_init_stb_vorbis,
    NULL, NULL, NULL
};
