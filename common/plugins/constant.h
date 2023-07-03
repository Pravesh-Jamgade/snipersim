#ifndef CONSTANT_H
#define CONSTANT_H

#pragma once
enum class ARRAY_TYPE
{
    NONE=0,
    OA,
    EA,
    PA
};

static String arr_name[] = {"NA", "OA", "EA", "PA"};
static inline String get_arr_name(int i)
{
    return arr_name[i];
}

#define ARRAY_TYPE_SIZE 4
#endif