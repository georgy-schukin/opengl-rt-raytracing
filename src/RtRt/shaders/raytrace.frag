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

uniform int limitOfRecursion = 1;

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

vec3 getIllumination(vec3 point, vec3 ray, int recursionStep) {
    vec3 illumPoint;
    // Find an object we a looking at
    int closestObject = getIntersection(point, ray, illumPoint);
    if (closestObject == -1) {
        return recursionStep == 0 ? backgroundColor : vec3(0.0);
    }

    Sphere currSphere = spheres[closestObject];
    //return closestSphere.color;

    vec3 color = vec3(0.0);
    for(int i = 0; i < numOfLightSources; i++) {
        // Check if the point on the object is illuminated
        vec3 shadowRay = lightSources[i].position - illumPoint;
        float lightDistance = length(shadowRay);
        vec3 intersectionPoint;
        // Optimization potential: do not need closest object here, just check for an obstacle
        int obstacle = getIntersection(illumPoint, normalize(shadowRay), intersectionPoint);
        if (obstacle != -1) {
            // Check if light is closer then the intersected object            
            if (length(intersectionPoint - illumPoint) > lightDistance) {
                obstacle = -1;
            }
        }
        if (obstacle == -1) {
            // Reflection
            vec3 normal = normalize(illumPoint - currSphere.position);
            float cosine = -dot(normal, ray);
            vec3 reflectedRay = ray + 2 * cosine * normal;
            cosine = max(0.0, dot(reflectedRay, shadowRay));
            // Apply coefficients of the body color to the intensity of the light source
            color += lightSources[i].color * currSphere.color * cosine;
        }
    }
    // Apply ambient light
    color += ambientLight * currSphere.color;

    /*if (recursionStep < limitOfRecursion) {
        // Reflection
        vec3 normal = normalize(closestIntersectionPoint - closestSphere.position);
        float cosine = -dot(normal, ray);
        vec3 reflectedRay = normalize(ray + 2 * cosine * normal);
        //vec3 reflectionColor = illuminationStep(closestIntersectionPoint, reflectedRay, recursionStep + 1);
        //color += closestSphere.color * reflectionColor;
    }*/
    return color;
}

//uniform mat4 projectionMatrix;
//uniform mat4 projectionMatrixInv;
//uniform mat4 viewMatrixInv;
uniform mat4 camToWorld;
uniform vec2 windowSize;
uniform float cameraFOV;

in vec3 ndcPos;
out vec4 fragColor;

void main()
{    
    vec2 pixelPos = gl_FragCoord.xy;
    //float x = (2.0f * pixelPos.x) / windowSize.x - 1.0f;
    //float y = (2.0f * pixelPos.y) / windowSize.y - 1.0f;
    //vec3 pos = vec4(viewMatrixInv * projectionMatrixInv * vec4(ndcPos, 1)).xyz; // pixel pos in world
    //vec4 rayClip = vec4(ndcPos.xy, -1.0, 1.0);
    //vec4 rayEye = vec4(vec4(projectionMatrixInv * rayClip).xy, -1.0f, 0.0f);
    //vec3 ray = normalize(vec4(viewMatrixInv * rayEye).xyz);
    float PI = 3.14159;
    float aspect = windowSize.x / windowSize.y; // assuming width > height
    float tn = tan(cameraFOV * PI / 360);
    float px = (2 * (pixelPos.x + 0.5) / windowSize.x - 1) * tn * aspect;
    float py = (2 * (pixelPos.y + 0.5) / windowSize.y - 1) * tn;
    vec3 viewPoint = vec4(camToWorld * vec4(0, 0, 0, 1)).xyz;
    vec3 posWorld = vec4(camToWorld * vec4(px, py, -1, 1)).xyz;

    //vec3 viewPoint = vec4(viewMatrixInv * vec4(0, 0, 0, 1)).xyz; // cam eye in world
    //vec3 ray = normalize(pos - viewPoint);
    vec3 ray = normalize(posWorld - viewPoint);
    vec3 color = getIllumination(viewPoint, ray, 0);
    color = clamp(color, vec3(0), vec3(1));
    fragColor = vec4(color, 1.0f);
    //fragColor = vec4(vec2(ray + vec3(1))/2, 0, 1.0f);
    //fragColor = vec4((ndcPos.xy + vec2(1))/2, 0, 1);
}
