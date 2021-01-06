#pragma once
# include <vector>
# include <cmath>
# include <glm/glm.hpp>
using glm::vec3;
using std::vector;

vec3 quadBezier(vec3 a, vec3 b, vec3 c, float t);
vec3 cubicBezier(vec3 a, vec3 b, vec3 c, vec3 d, float t);
vec3 cubicBeziers(vector<vec3> points, float t, bool closed = false);
vec3 quadBezierTan(vec3 a, vec3 b, vec3 c, float t);
vec3 cubicBezierTan(vec3 a, vec3 b, vec3 c, vec3 d, float t);
vec3 getTan(vector<vec3> points, float t, bool closed = false);
bool isCounterClockWise(vector<vec3> points, vec3 view);