#version 330

in vec3 vertex;

void main()
{
    gl_Position = vec4(vertex, 1.0f); // vertex is in NDC, just pass it
}
