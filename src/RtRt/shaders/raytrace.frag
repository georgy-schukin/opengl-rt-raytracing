#version 330

struct Sphere
{
    vec3 position;
    float radius;
    vec3 color;
};

struct LightSource
{
    vec3 position;
    vec3 color;
};

uniform Sphere spheres[32];
uniform LightSource lightSources[10];

uniform int numOfSpheres;
uniform int numOfLightSources;

uniform vec3 ambientLight = vec3(0.05);

uniform int numOfSteps = 1;

uniform vec3 backgroundColor = vec3(0.0);

bool intersectSphere(Sphere sphere, vec3 startPoint, vec3 ray, out float intersectionDistance) {
    vec3 v = startPoint - sphere.position;
    float d = dot(v, ray);
    float discriminant = d * d - (dot(v, v) - sphere.radius * sphere.radius);
    if (discriminant < 0) {
        return false;
    }

    float t1 = -d + sqrt(discriminant);
    float t2 = -d - sqrt(discriminant);

    float t = 0;
    float epsilon = 1e-3;
    if (t1 < epsilon) {
        if (t2 < epsilon) {
            return false;
        }
        else {
            t = t2;
        }
    } else if (t2 < epsilon) {
        t = t1;
    } else {
        t = (t1 > t2) ? t2 : t1;
    }

    intersectionDistance = t;
    return true;
}

int getIntersection(vec3 startPoint, vec3 ray, out vec3 closestIntersectionPoint) {
    int closestObject = -1;
    float minDistance = 1e+8;
    for (int i = 0; i < numOfSpheres; i++) {
        float intersectionDistance;
        if (intersectSphere(spheres[i], startPoint, ray, intersectionDistance)) {
            if (intersectionDistance < minDistance) {
                closestObject = i;               
                minDistance = intersectionDistance;
            }
        }
    }
    if (closestObject != -1) {
        closestIntersectionPoint = startPoint + minDistance * ray;
    }
    return closestObject;
}

bool nextIntersection(vec3 point, vec3 ray, int step, out vec3 color, out vec3 intersectionPoint, out vec3 reflectedRay, out Sphere sphere) {
    color = vec3(0.0);
    // Find an object we a looking at
    int closestObject = getIntersection(point, ray, intersectionPoint);
    if (closestObject == -1) {
        color = (step == 0 ? backgroundColor : vec3(0));
        return false;
    }

    sphere = spheres[closestObject];
    //return closestSphere.color;

    vec3 normal = normalize(intersectionPoint - sphere.position);
    reflectedRay = ray - 2 * dot(normal, ray) * normal;

    for(int i = 0; i < numOfLightSources; i++) {
        // Check if the point on the object is illuminated
        vec3 shadowRay = lightSources[i].position - intersectionPoint;
        float lightDistance = length(shadowRay);
        shadowRay = normalize(shadowRay);
        vec3 intersectionLightPoint;
        // Optimization potential: do not need closest object here, just check for an obstacle
        int obstacle = getIntersection(intersectionPoint, shadowRay, intersectionLightPoint);
        if (obstacle != -1) {
            // Check if light is closer then the intersected object
            if (length(intersectionLightPoint - intersectionPoint) > lightDistance) {
                obstacle = -1;
            }
        }
        if (obstacle == -1) {
            // Reflection
            // Apply coefficients of the body color to the intensity of the light source
            color += lightSources[i].color * sphere.color * max(0.0, dot(reflectedRay, shadowRay));
        }
    }
    // Apply ambient light
    color += ambientLight * sphere.color;
    return true;
}

vec3 getIllumination(vec3 point, vec3 ray) {
    vec3 totalColor = vec3(0.0);
    vec3 currPoint = point;
    vec3 currRay = ray;
    vec3 currMult = vec3(1.0);
    bool stop = false;

    for (int n = 0; n < numOfSteps && !stop; n++) {
        vec3 intersectionPoint;
        vec3 reflectedRay;
        vec3 color;
        Sphere sphere;
        stop = !nextIntersection(currPoint, currRay, n, color, intersectionPoint, reflectedRay, sphere);
        if (stop) {
            totalColor += currMult * color;
        } else {
            totalColor += currMult * color;
            currPoint = intersectionPoint;
            currRay = reflectedRay;
            currMult *= sphere.color;
        }

        //if (recursionStep < numOfSteps) {
            // Reflection
            //vec3 reflectionColor = getIllumination(illumPoint, normalize(reflectedRay), recursionStep + 1);
            //color += currSphere.color * reflectionColor;
        //}
    }
    return totalColor;
}

uniform mat4 camToWorld;
uniform vec2 windowSize;
uniform float cameraFOV;
uniform float fovTangent;

uniform sampler2D jitter;
uniform int jitterSize;

uniform sampler1D randoms;
uniform int randomsSize;

uniform int numOfSamples = 1;

out vec4 fragColor;

int currRand = 0;

void seed(int seed) {
    currRand = seed;
}

float rand() {
    float value = texture(randoms, float(currRand) / float(randomsSize)).r;
    currRand++;
    return value;
}

vec3 shoot(vec2 fragCoord, float aspect, vec3 viewPoint) {
    float px = (2 * (fragCoord.x + 0.5) / windowSize.x - 1) * fovTangent * aspect;
    float py = (2 * (fragCoord.y + 0.5) / windowSize.y - 1) * fovTangent;
    vec3 posWorld = vec4(camToWorld * vec4(px, py, -1, 1)).xyz;
    vec3 ray = normalize(posWorld - viewPoint);
    vec3 color = getIllumination(viewPoint, ray);
    return color;
}

void main()
{
    float aspect = windowSize.x / windowSize.y; // assuming width > height
    vec3 viewPoint = vec4(camToWorld * vec4(0, 0, 0, 1)).xyz;
    vec3 color = vec3(0);
    if (numOfSamples == 1) {
        color = shoot(gl_FragCoord.xy, aspect, viewPoint);
    } else {
        seed(int(gl_FragCoord.x * windowSize.y + gl_FragCoord.y));
        for (int i = 0; i < numOfSamples; i++)
        for (int j = 0; j < numOfSamples; j++) {
            float dx = (i + rand()) / numOfSamples;
            float dy = (j + rand()) / numOfSamples;
            color += shoot(gl_FragCoord.xy - vec2(0.5) + vec2(dx, dy), aspect, viewPoint);
        }
        color /= (numOfSamples * numOfSamples);
        /*int jx = int(gl_FragCoord.x) % (jitterSize * numOfSamples);
        int jy = int(gl_FragCoord.y) % (jitterSize * numOfSamples);
        vec2 start = gl_FragCoord.xy / (jitterSize * numOfSamples);
        for (int i = 0; i < numOfSamples; i++)
        for (int j = 0; j < numOfSamples; j++) {
            vec2 jit = texture(jitter, start + vec2(i, j) / jitterSize).rg;
            jit = (jit * jitterSize - vec2(jx, jy)) / numOfSamples;
            color += shoot(gl_FragCoord.xy - vec2(0.5) + jit, aspect, viewPoint);
        }*/
    }
    fragColor = vec4(clamp(color, vec3(0), vec3(1)), 1.0f);
    //fragColor = vec4(pixelPos.x - int(pixelPos.x), pixelPos.y - int(pixelPos.y), 0, 1);
}
