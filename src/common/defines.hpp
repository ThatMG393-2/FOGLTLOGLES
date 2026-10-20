#pragma once

#ifndef EXTERN
#define EXTERN extern "C"
#endif

#ifndef PUBLIC
#define PUBLIC __attribute__((visibility("default")))
#endif

#ifndef PUBLIC_API
#define PUBLIC_API PUBLIC EXTERN
#endif

#ifndef DEFINE_API
#define DEFINE_API(type, func, ...) PUBLIC_API type func __VA_ARGS__
#endif