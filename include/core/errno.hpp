/*
*	name: include/core/errorcode.hpp
*	origin: Citrus Engine
*	purpose: Provide engine::errorcode
*	author: https://github.com/ComradeYellowCitrusFruit
*	license: LGPL-3.0-only
*/

#ifndef CITRUS_ENGINE_ERROR_NUMBER_HPP__
#define CITRUS_ENGINE_ERROR_NUMBER_HPP__

#define ENGINE_FATALERR_PREFIX 0x8 << 28
#define ENGINE_ERROR_PREFIX 0x4 << 28
#define ENGINE_SILENT_PREFIX 0x2 << 28

#define ENGINE_NO_MEM ENGINE_FATALERR_PREFIX | 01
#define ENGINE_ILLEGAL_OP ENGINE_FATALERR_PREFIX | 02
#define ENGINE_BADSHADER ENGINE_FATALERR_PREFIX | 03
#define ENGINE_INVALID_PTR ENGINE_FATALERR_PREFIX | 020

#define ENGINE_INVALID_ARG ENGINE_ERROR_PREFIX | 040
#define ENGINE_INVALID_SCENE ENGINE_ERROR_PREFIX | 060
#define ENGINE_HASH_COLLISION ENGINE_INVALID_ARG
#define ENGINE_CLIB_ERRNO_SET ENGINE_ERROR_PREFIX | 017

// memrealloc invalid pointer
#define ENGINE_SOFT_MEM_LIMIT_REACHED ENGINE_SILENT_PREFIX | 01
#define ENGINE_MEMREALLOC_INVALID_PTR ENGINE_SILENT_PREFIX | 020
#define ENGINE_MEMFREE_INVALID_PTR ENGINE_MEMREALLOC_INVALID_PTR

// TODO: More defines

namespace engine
{
    // TODO: make engine::errorcode have a seperate variable for each thread, like errno
    extern unsigned int errorcode;
}

#endif
