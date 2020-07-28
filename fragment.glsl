#version 330 core
in vec2 coord;
out vec3 color;

const float MIN_MARCH = 1e-6;
const float FAR_CLIP = 100;
const float NORMAL_OFFSET = MIN_MARCH * 10;
const float SHARPNESS = 5;

const vec3 bgcolor = vec3(0, .01, .05);
const vec3 fgcolor = vec3(.7, 0, .9);
const vec3 shcolor = mix(bgcolor, fgcolor, .2);

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

float light(vec3 light, vec3 normal, vec3 ray, float len) {
	vec3 incident = ray*len - light;
	vec3 reflection = reflect(incident, normal);
	float bright = dot(reflection, ray);
	bright /= 2*(1 + length(reflection));
	return 0.5 - bright;
}

float brightness(vec3 origin, vec3 ray, float len) {
	vec3 normal = normal(origin, ray, len);
	float b = light(vec3(0, 10, -20), normal, ray, len);
	return b;
}

void main() {
	vec3 cam = vec3(0, 0, 20);
	float clip = 6;
	vec3 clipcoord = vec3(coord, cam.z - clip);

	vec3 ray = clipcoord - cam;
	float len = length(ray);
	ray /= len;

	raymarch(cam, ray, len);
	if (isinf(len)) {
		color = bgcolor;
		return;
	}

	float bright = brightness(cam, ray, len);
	color = mix(shcolor, fgcolor, bright);
}
