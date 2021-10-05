#pragma once

class QOpenGLFunctions;

class GLShape {
public:
    virtual ~GLShape() = default;

    virtual void draw(QOpenGLFunctions*) = 0;
};
