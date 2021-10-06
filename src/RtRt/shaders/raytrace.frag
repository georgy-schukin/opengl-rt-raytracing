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

uniform Sphere spheres[10];
uniform LightSource lightSources[10];

uniform int numOfSpheres;
uniform int numOfLightSources;

uniform vec3 ambientLight = vec3(0.05);

uniform int numOfSteps = 1;

uniform vec3 backgroundColor = vec3(0.0);

bool intersectSphere(Sphere sphere, vec3 startPoint, vec3 ray, out vec3 intersectionPoint) {
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

    intersectionPoint = ray * t + startPoint;
    return true;
}

int getIntersection(vec3 startPoint, vec3 ray, out vec3 closestIntersectionPoint) {
    int closestObject = -1;
    float minDistance = 1e+8;
    for (int i = 0; i < numOfSpheres; i++) {
        vec3 intersectionPoint;
        if (intersectSphere(spheres[i], startPoint, ray, intersectionPoint)) {
            vec3 dir = intersectionPoint - startPoint;
            float distance = dot(dir, dir);
            if (distance < minDistance) {
                closestObject = i;
                closestIntersectionPoint = intersectionPoint;
                minDistance = distance;
            }
        }
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

out vec4 fragColor;

void main()
{
    float aspect = windowSize.x / windowSize.y; // assuming width > height
    vec2 pixelPos = gl_FragCoord.xy;
    float px = (2 * (pixelPos.x + 0.5) / windowSize.x - 1) * fovTangent * aspect;
    float py = (2 * (pixelPos.y + 0.5) / windowSize.y - 1) * fovTangent;
    vec3 viewPoint = vec4(camToWorld * vec4(0, 0, 0, 1)).xyz;
    vec3 posWorld = vec4(camToWorld * vec4(px, py, -1, 1)).xyz;
    vec3 ray = normalize(posWorld - viewPoint);
    vec3 color = getIllumination(viewPoint, ray);
    color = clamp(color, vec3(0), vec3(1));
    fragColor = vec4(color, 1.0f);
}
