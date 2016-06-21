#pragma once

#include <QtCore/QtGlobal>
#include <QtGui/QVector3D>
#include <QtGui/QMatrix4x4>
#include <QtWidgets/QApplication>
#include <QtWidgets/QOpenGLWidget>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLFunctions_4_1_Core>

#define CreateScene() GLView::createScene()
#define DestroyScene() GLView::destroyScene()
#define RenderFrame() GLView::paintGL()
#define ResizeFrame() GLView::resizeScene()

class GLView : public QOpenGLWidget, private QOpenGLFunctions_4_1_Core
{
public:
    ~GLView()
    {
        makeCurrent();
        destroyScene();
        doneCurrent();
    }

protected:

    QMap<GLuint, QOpenGLShaderProgram *> mShaders;

    GLuint loadShader(const QString &name)
    {
        QOpenGLShaderProgram *shader = new QOpenGLShaderProgram();
        if (shader->addShaderFromSourceFile(QOpenGLShader::Vertex, name + ".vert") &&
            shader->addShaderFromSourceFile(QOpenGLShader::Fragment, name + ".frag"))
        {
            mShaders[shader->programId()] = shader;
            return shader->programId();
        }
        delete shader;
        return 0;
    }

    void unloadShader(GLuint shaderId)
    {
        auto pos = mShaders.find(shaderId);
        if (pos != mShaders.end())
        {
            mShaders.erase(pos);
        }
    }

    virtual void initializeGL() override
    {
        if (initializeOpenGLFunctions())
        {
            createScene();
        }
    }

    void createScene();

    void resizeScene();

    void destroyScene();

    virtual void paintGL() override;

    virtual void resizeGL(int width, int height) override
    {
        FrameWidth = width, FrameHeight = height;
        resizeScene();
    }

    int FrameWidth, FrameHeight;
};

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL, true);
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts, false);
    QApplication a(argc, argv);
    QSurfaceFormat format;
    format.setMajorVersion(4);
    format.setMinorVersion(1);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);
    GLView mainView;
    mainView.setMinimumSize(1024, 768);
    mainView.showMaximized();
    return a.exec();
}
