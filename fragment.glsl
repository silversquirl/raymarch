#version 330 core
in vec2 coord;
out vec3 color;

const float MIN_MARCH = 0.001;
const float FAR_CLIP = 100;

const vec3 bgcolor = vec3(0, .01, .05);
const vec3 fgcolor = vec3(.5, 0, .7);
const vec3 shcolor = vec3(.01, 0, .05);

float dist2sphere(vec3 point, vec3 center, float radius) {
	return distance(point, center) - radius;
}

float distance(vec3 point) {
	// Unit sphere at origin
	// Hardcoded 'cause I lazy
	return dist2sphere(point, vec3(0, 0, 0), 1);
}

float raymarch(vec3 origin, vec3 ray, float len) {
	float march = 0;
	float amarch;
	float minmarch = 1./0.; // +inf

	for (;;) {
		len += march;

		march = distance(origin + ray*len);
		minmarch = min(march, minmarch);
		amarch = abs(march);

		if (amarch < MIN_MARCH) {
			return 0.;
		} else if (amarch > FAR_CLIP) {
			return minmarch;
		}
	}
}

void main() {
	vec3 cam = vec3(0, 0, 20);
	float clip = 6;
	vec3 clipcoord = vec3(coord, cam.z - clip);

	vec3 ray = clipcoord - cam;
	float len = length(ray);
	ray /= len;

	float minmarch = raymarch(cam, ray, len);
	if (minmarch == 0) {
		color = fgcolor;
	} else {
		// Fancy antialiasing yay
		color = mix(fgcolor, bgcolor, min(minmarch * 100, 1));
	}
}
