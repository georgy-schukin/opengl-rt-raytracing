#version 330

in vec3 vertex;
out vec3 ndcPos;

void main()
{
    ndcPos = vertex;
    gl_Position = vec4(vertex, 1.0f); // vertex is in NDC, just pass it
}
