# include "mathTool.h"

vec3 quadBezier(vec3 a, vec3 b, vec3 c, float t)
{
	if (t > 1 || t < 0)
		return vec3(0, 0, 0);
	return pow((1 - t), 2) * a + 2 * (1 - t) * t * b
		+ pow(t, 2) * c;
}

vec3 cubicBezier(vec3 a, vec3 b, vec3 c, vec3 d, float t)
{
	if (t > 1 || t < 0)
		return vec3(0, 0, 0);
	return pow((1 - t), 3) * a + 3 * pow((1 - t), 2) * t * b
		+ 3 * (1 - t) * pow(t, 2) * c + pow(t, 3) * d;
}

vec3 quadBezierTan(vec3 a, vec3 b, vec3 c, float t)
{
	if (t > 1 || t < 0)
		return vec3(0, 0, 0);
	return 2 * (1 - t) * (b - a) + 2 * t * (c - b);
}

vec3 cubicBezierTan(vec3 a, vec3 b, vec3 c, vec3 d, float t)
{
	if (t > 1 || t < 0)
		return vec3(0, 0, 0);
	return 3 * pow((1 - t), 2) * (b - a) + 6 * (1 - t) * t * (c - b)
		+ 3 * pow(t, 2) * (d - c);
}

vec3 cubicBeziers(vector<vec3> points, float t, bool closed)
{
	if (points.size() == 0)
		return vec3(0, 0, 0);
	int start = floor(t * (float)(points.size() + closed-1) / 3) * 3;
	float portion = (t * (float)(points.size() + closed-1) - (float)start) / 3.0f;
	if (start + 3 >= points.size() + closed)
	{
		if (start + 2 >= points.size() + closed)
		{
			if (start + 1 >= points.size() + closed)
			{
				return points[start% points.size()];
			}
			else
			{
				// 2
				portion *= 3.0f;
				return (1 - portion) * points[start] + portion * points[(start + 1)% points.size()];
			}
		}
		else
		{
			// 3
			portion *= 3.0f / 2;
			return quadBezier(points[start], points[start + 1], points[(start + 2) % points.size()], portion);
		}
	}
	else
	{
		// 4
		return cubicBezier(points[start], points[start + 1], points[start + 2], points[(start + 3) % points.size()], portion);
	}
}


vec3 getTan(vector<vec3> points, float t, bool closed)
{
	if (points.size() == 0 || points.size() + closed == 1)
		return vec3(0, 0, 0);
	int start = floor(t * (float)(points.size() + closed-1) / 3) * 3;
	float portion = (t * (float)(points.size() + closed-1) - (float)start) / 3.0f;
	if (start + 3 >= points.size() + closed)
	{
		if (start + 2 >= points.size() + closed)
		{
			if (start + 1 >= points.size() + closed)
			{
				// TODO may loop, debug here
				// last point
				return getTan(points, t - 0.0001, closed);
			}
			else
			{
				return -points[start] + points[(start + 1) % points.size()];
			}
		}
		else
		{
			// 3
			portion *= 2.0f / 3;
			return quadBezierTan(points[start], points[start + 1], points[(start + 2) % points.size()], portion);
		}
	}
	else
	{
		// 4
		return cubicBezierTan(points[start], points[start + 1], points[start + 2], points[(start + 3) % points.size()], portion);
	}
}


bool isCounterClockWise(vector<vec3> points, vec3 view)
{
	if (points.size() > 2)
	{
		vec3 c = vec3(1, 0, 0);
		for (int i = 0; i < points.size(); i++)
			c += points[i];
		c /= (float)points.size();
		vec3 y = glm::cross((points[0] - c), (points[1] - c));
		return glm::dot(y, view) > 0 ? true : false;
	}
	else
		return true;
}