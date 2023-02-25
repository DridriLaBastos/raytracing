//
// Created by Adrien COURNAND on 14/02/2023.
//

#ifndef RAYTRACING_LOG_HPP
#define RAYTRACING_LOG_HPP

#include <cstdio>

#define INFO(FMT,...) printf(" * [INFO] " FMT "\n", ##__VA_ARGS__)
#define ERROR(FMT,...) fprintf(stderr, " * [WARNING] " FMT "\n", ##__VA_ARGS__)

#endif //RAYTRACING_LOG_HPP
