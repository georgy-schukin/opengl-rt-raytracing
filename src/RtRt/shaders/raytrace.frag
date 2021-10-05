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

uniform vec3 ambientLight = vec3(0.0);

uniform int limitOfRecursion = 1;

uniform vec3 backgroundColor = vec3(0.0);

bool intersectSphere(Sphere sphere, vec3 ray, vec3 startPoint, out vec3 intersectionPoint) {
    vec3 v = startPoint - sphere.position;
    float d = dot(v, ray);
    float discriminant = d * d - (dot(v, v) - sphere.radius * sphere.radius);
    if (discriminant < 0) {
        return false;
    }

    float t1 = -d + sqrt(discriminant);
    float t2 = -d - sqrt(discriminant);

    float t = 0;
    float epsilon = 1e-6;
    if (t1 < epsilon) {
        if(t2 < epsilon) {
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

int intersect(vec3 startPoint, vec3 ray, out vec3 closestIntersectionPoint) {
    int closestObject = -1;
    float minDistance = 1e+8;
    for (int i = 0; i < numOfSpheres; i++) {
        vec3 intersectionPoint;
        if (intersectSphere(spheres[i], startPoint, ray, intersectionPoint)) {
            vec3 dist = intersectionPoint - startPoint;
            float distance = dot(dist, dist);
            if (distance < minDistance) {
                closestObject = i;
                closestIntersectionPoint = intersectionPoint;
                minDistance = distance;
            }
        }
    }
    return closestObject;
}

vec3 illuminationStep(vec3 point, vec3 ray, int recursionStep) {
    vec3 closestIntersectionPoint;
    // Find an object we a looking at
    int closestObject = intersect(point, ray, closestIntersectionPoint);
    if (closestObject == -1) {
        return recursionStep == 0 ? backgroundColor : vec3(0.0);
    }

    Sphere closestSphere = spheres[closestObject];

    vec3 color = vec3(0.0);
    for(int i = 0; i < numOfLightSources; i++) {
        // Check if the point on the object is illuminated
        vec3 rayToLightSource = normalize(lightSources[i].position - closestIntersectionPoint);
        vec3 intersectionPoint;
        // Optimization potential: do not need closest object here, just check for an obstacle
        int obstacle = intersect(closestIntersectionPoint, rayToLightSource, intersectionPoint);
        if (obstacle != -1) {
            // Check if light is closer then the intersected object
            vec3 dist = intersectionPoint - closestIntersectionPoint;
            float obstacleDistance = sqrt(dot(dist, dist));
            if (obstacleDistance > 1) {
                obstacle = -1;
            }
        }
        if (obstacle == -1) {
            // Reflection
            vec3 normal = normalize(closestIntersectionPoint - closestSphere.position);
            float cosine = -dot(normal, ray);
            vec3 reflectedRay = ray + 2 * cosine * normal;
            cosine = max(0.0, dot(reflectedRay, rayToLightSource));
            // Apply coefficients of the body color to the intensity of the light source
            color += lightSources[i].color * closestSphere.color * cosine;
        }
    }
    // Apply ambient light
    color += ambientLight * closestSphere.color;

    if (recursionStep < limitOfRecursion) {
        // Reflection
        vec3 normal = normalize(closestIntersectionPoint - closestSphere.position);
        float cosine = -dot(normal, ray);
        vec3 reflectedRay = normalize(ray + 2 * cosine * normal);
        vec3 reflectionColor = illuminationStep(closestIntersectionPoint, reflectedRay, recursionStep + 1);
        color += closestSphere.color * reflectionColor;
    }
    return color;
}

uniform vec2 windowSize;
uniform vec3 planeSize;
uniform vec3 viewPoint;

out vec4 fragColor;

void main()
{
    vec2 pos = gl_FragCoord.xy;
    vec3 ray = vec3(
        pos.x * planeSize.x / windowSize.x - planeSize.x / 2,
        pos.y * planeSize.y / windowSize.y - planeSize.y / 2,
        planeSize.z);
    vec3 color = illuminationStep(viewPoint, normalize(ray), 0);
    fragColor = vec4(color, 1.0f);
}
