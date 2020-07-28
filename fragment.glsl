#version 330 core
in vec2 coord;
out vec3 color;

uniform vec2 scale;
uniform vec3 cam_pos;
uniform vec4 cam_look;

const float MIN_MARCH = 1e-5;
const float FAR_CLIP = 100;
const float NORMAL_OFFSET = MIN_MARCH * 10;
const float SHARPNESS = 5;

const vec3 bgcolor = vec3(0, .01, .05);
const vec3 fgcolor = vec3(.7, 0, .9);
const vec3 shcolor = mix(bgcolor, fgcolor, .2);

// Quaternions {{{
// GLSL has w as the last component, rather than the first as in quats, so names are a bit confusing
vec4 qinv(vec4 q) {
	float fac = 1.0f / dot(q, q);
	return q * vec4(fac, -fac, -fac, -fac);
}

vec4 qmul(vec4 a, vec4 b) {
	vec2 pm = vec2(1, -1);
	return vec4(
		dot(a, b.xyzw * pm.xyyy),
		dot(a, b.yxwz * pm.xxxy),
		dot(a, b.zwxy * pm.xyxx),
		dot(a, b.wzyx * pm.xxyx)
	);
}

vec3 v3rot(vec3 v, vec4 q) {
	return qmul(q, qmul(vec4(0, v), qinv(q))).yzw;
}
// }}}

float dist2sphere(vec3 point, vec3 center, float radius) {
	return distance(point, center) - radius;
}

float distance(vec3 point) {
	float d = dist2sphere(point, vec3(0, 0, 0), 1);
	d = min(d, dist2sphere(point, vec3(1, 1, 1), 0.2));
	d = min(d, dist2sphere(point, vec3(-1, 0, 14), 0.2));
	return d;
}

void raymarch(vec3 origin, vec3 ray, inout float len) {
	const float inf = uintBitsToFloat(0x7f800000U);

	float march = 0;

	// TODO: unroll this loop a bit, should improve perf due to reduced branching
	for (;;) {
		len += march;

		march = distance(origin + ray*len);

		if (abs(march) < MIN_MARCH) {
			return;
		} else if (len > FAR_CLIP) {
			len = inf;
			return;
		}
	}
}

vec3 normal(vec3 origin, vec3 ray, float len) {
	vec3 d1 = cross(ray, ray + vec3(10, 0, 0));
	vec3 d2 = cross(ray, ray + vec3(0, 10, 0));

	d1 = normalize(d1) * NORMAL_OFFSET;
	d2 = normalize(d2) * NORMAL_OFFSET;

	float len1 = len * 0.0;
	raymarch(origin, ray + d1, len1);

	float len2 = len * 0.0;
	raymarch(origin, ray + d2, len2);

	vec3 p0 = origin + ray*len;
	vec3 p1 = origin + (ray + d1)*len1;
	vec3 p2 = origin + (ray + d2)*len2;

	vec3 normal = cross(p1 - p0, p2 - p0);
	return normalize(normal);
}

float light(vec3 light, vec3 normal, vec3 point, vec3 ray) {
	vec3 incident = point - light;
	vec3 reflection = reflect(incident, normal);
	float bright = dot(reflection, ray);
	bright /= 2*(1 + length(reflection));
	return 0.5 - bright;
}

float brightness(vec3 origin, vec3 ray, float len) {
	vec3 normal = normal(origin, ray, len);
	float b = light(vec3(0, 10, -20), normal, origin + ray*len, ray);
	return b;
}

void main() {
	float clip = 3;
	vec3 ray = vec3(scale * coord, -clip);
	ray = v3rot(ray, cam_look);
	float len = length(ray);
	ray /= len;

	raymarch(cam_pos, ray, len);
	if (isinf(len)) {
		color = bgcolor;
		return;
	}

	float bright = brightness(cam_pos, ray, len);
	color = mix(shcolor, fgcolor, bright);
}
