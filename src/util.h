#pragma once
#include <math.h>
#include <string>
#include "Vec2.h"


float pointLineDistance(const Vec2& p, const Vec2& v, const Vec2& w);
float pointPointDistance(const Vec2& p, const Vec2& v);
bool saveStringToFile(const std::string& data, const std::string& filepath);
void saveLevelData(std::string data);
