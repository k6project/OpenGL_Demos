#include "GLSandbox.h"

enum
{
    VA_POSITION = 0, VA_NORMAL,
    VA_TOWORLD, VA_COLORMOD = VA_TOWORLD + 4,
    VA_TOTAL
};

static struct Scene
{
    QMatrix4x4 Projection, View;
    union
    {
        struct { GLuint VBO, IBO; };
        GLuint Buffers[2];
    };
    GLuint VAO, Shader, NumInst;
    Scene() {}
} scene;

struct InstanceData
{
    QMatrix4x4 Transform;
    GLuint Modifier;
};

static const struct Vertex { QVector3D p, n; }
CUBE_MESH_VERTICES[] =
{
    { { -1.f,  1.f, 1.f }, { 0.f, 0.f, 1.f } },
    { {  1.f,  1.f, 1.f }, { 0.f, 0.f, 1.f } },
    { { -1.f, -1.f, 1.f }, { 0.f, 0.f, 1.f } },
    { {  1.f, -1.f, 1.f }, { 0.f, 0.f, 1.f } },
    { {  1.f,  1.f, -1.f }, { 0.f, 0.f, -1.f } },
    { { -1.f,  1.f, -1.f }, { 0.f, 0.f, -1.f } },
    { {  1.f, -1.f, -1.f }, { 0.f, 0.f, -1.f } },
    { { -1.f, -1.f, -1.f }, { 0.f, 0.f, -1.f } },
    { {  1.f, 1.f,  1.f }, { 0.f, 1.f, 0.f } },
    { { -1.f, 1.f,  1.f }, { 0.f, 1.f, 0.f } },
    { {  1.f, 1.f, -1.f }, { 0.f, 1.f, 0.f } },
    { { -1.f, 1.f, -1.f }, { 0.f, 1.f, 0.f } },
    { { -1.f, -1.f,  1.f },{ 0.f, -1.f, 0.f } },
    { {  1.f, -1.f,  1.f },{ 0.f, -1.f, 0.f } },
    { { -1.f, -1.f, -1.f },{ 0.f, -1.f, 0.f } },
    { {  1.f, -1.f, -1.f },{ 0.f, -1.f, 0.f } },
    { { 1.f,  1.f,  1.f }, { 1.f, 0.f, 0.f } },
    { { 1.f,  1.f, -1.f }, { 1.f, 0.f, 0.f } },
    { { 1.f, -1.f,  1.f }, { 1.f, 0.f, 0.f } },
    { { 1.f, -1.f, -1.f }, { 1.f, 0.f, 0.f } },
    { { -1.f,  1.f, -1.f },{ -1.f, 0.f, 0.f } },
    { { -1.f,  1.f,  1.f },{ -1.f, 0.f, 0.f } },
    { { -1.f, -1.f, -1.f },{ -1.f, 0.f, 0.f } },
    { { -1.f, -1.f,  1.f },{ -1.f, 0.f, 0.f } }
};

const unsigned int CUBE_MESH_NUM_INDICES = 36;
static const unsigned short CUBE_MESH_INDICES[] =
{
    0,  2,  3,  3,  1,  0, 4,  6,  7,  7,  5,  4,
    8, 10, 11, 11,  9,  8, 12, 14, 15, 15, 13, 12,
   16, 18, 19, 19, 17, 16, 20, 22, 23, 23, 21, 20
};

static inline quint32 PyramidNumber(quint32 n)
{
    return (2 * n * n * n + 3 * n * n + n) / 6;
}

size_t MakePyramid(quint32 levels, quint32 &outCount, quint8 *buffer, size_t max)
{
    float hStep = 2.4f, vStep = 2.f;
    quint32 count = PyramidNumber(levels), index = 0;
    size_t reqBytes = count * sizeof(InstanceData) + sizeof(CUBE_MESH_VERTICES);
    if (reqBytes <= max)
    {
        memcpy(buffer, CUBE_MESH_VERTICES, sizeof(CUBE_MESH_VERTICES));
        InstanceData *instances = new (buffer + sizeof(CUBE_MESH_VERTICES)) InstanceData[count];
        for (quint32 levelSize = levels; levelSize > 0; levelSize--)
        {
            GLuint colorMod = levelSize % 2;
            float y = (levels - levelSize) * vStep;
            float start = (levelSize - 1) * hStep * (-0.5f);
            for (quint32 column = 0; column < levelSize; column++)
            {
                float x = start + column * hStep;
                for (quint32 row = 0; row < levelSize; row++)
                {
                    float z = start + row * hStep;
                    InstanceData *instance = &instances[index];
                    instance->Transform.translate(x, y, z);
                    instance->Modifier = colorMod;
                    index++;
                }
            }
        }
        outCount = count;
        return reqBytes;
    }
    return 0;
}

void ResizeFrame()
{
    scene.Projection.setToIdentity();
    float aspect = FrameWidth / static_cast<float>(FrameHeight);
    scene.Projection.perspective(60.f, aspect, 0.001f, 100.f);
    GLint proj = glGetUniformLocation(scene.Shader, "uProjection");
    glUniformMatrix4fv(proj, 1, GL_FALSE, scene.Projection.constData());
}

void CreateScene()
{
    quint8 buff[2048];
    size_t totalBytes = MakePyramid(3, scene.NumInst, buff, sizeof(buff));
    scene.View.lookAt({0.f, 7.f, 8.f}, {0.f, 0.f, 0.f}, {0.f, 1.f, 0.f});

    scene.Shader = loadShader("Shaders/Instancing");
    glLinkProgram(scene.Shader);
    glUseProgram(scene.Shader);

    GLint view = glGetUniformLocation(scene.Shader, "uView");
    glUniformMatrix4fv(view, 1, GL_FALSE, scene.View.constData());

    glGenBuffers(2, scene.Buffers);
    glGenVertexArrays(1, &scene.VAO);
    glBindVertexArray(scene.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, scene.VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, scene.IBO);
    glBufferData(GL_ARRAY_BUFFER, totalBytes, buff, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(CUBE_MESH_INDICES), CUBE_MESH_INDICES, GL_STATIC_DRAW);
    for (GLuint attr = VA_POSITION; attr < VA_TOTAL; attr++)
    {
        glEnableVertexAttribArray(attr);
    }

    glVertexAttribPointer(VA_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, p));
    glVertexAttribPointer(VA_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, n));

    ptrdiff_t offset =  sizeof(CUBE_MESH_VERTICES);
    glVertexAttribPointer(VA_TOWORLD + 0, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offset);
    offset += sizeof(float) << 2;
    glVertexAttribPointer(VA_TOWORLD + 1, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offset);
    offset += sizeof(float) << 2;
    glVertexAttribPointer(VA_TOWORLD + 2, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offset);
    offset += sizeof(float) << 2;
    glVertexAttribPointer(VA_TOWORLD + 3, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offset);
    offset = sizeof(CUBE_MESH_VERTICES) + sizeof(QMatrix4x4);
    glVertexAttribIPointer(VA_COLORMOD, 1, GL_UNSIGNED_INT, sizeof(InstanceData), (void*) offset);

    glVertexAttribDivisor(VA_TOWORLD + 0, 1);
    glVertexAttribDivisor(VA_TOWORLD + 1, 1);
    glVertexAttribDivisor(VA_TOWORLD + 2, 1);
    glVertexAttribDivisor(VA_TOWORLD + 3, 1);
    glVertexAttribDivisor(VA_COLORMOD, 1);

    glClearColor(0.0f, 0.4f, 0.4f, 1.f);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

void DestroyScene()
{
    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDeleteBuffers(3, scene.Buffers);
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &scene.VAO);
    unloadShader(scene.Shader);
}

void RenderFrame()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawElementsInstanced(GL_TRIANGLES, CUBE_MESH_NUM_INDICES, GL_UNSIGNED_SHORT, 0, scene.NumInst);
}
