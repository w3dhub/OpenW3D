#pragma once

#include "quat.h"

#include <QElapsedTimer>
#include <QPoint>
#include <QString>
#include <QTimer>
#include <QWidget>

class CameraClass;
class DazzleLayerClass;
class HAnimClass;
class HAnimComboClass;
class LightClass;
class Bitmap2DObjClass;
class QFocusEvent;
class QKeyEvent;
class RenderObjClass;
class SceneClass;

class W3DViewport final : public QWidget
{
    Q_OBJECT

public:
    enum ObjectRotation {
        RotateNone = 0,
        RotateX = 1 << 0,
        RotateY = 1 << 1,
        RotateZ = 1 << 2,
        RotateXBack = 1 << 3,
        RotateYBack = 1 << 4,
        RotateZBack = 1 << 5,
    };

    enum class CameraPosition {
        Front,
        Back,
        Left,
        Right,
        Top,
        Bottom,
    };

    enum class CameraRotation {
        Free,
        OnlyX,
        OnlyY,
        OnlyZ,
    };

    enum class AnimationState {
        Playing,
        Stopped,
        Paused,
    };

    struct SceneLightState {
        Vector3 diffuse = Vector3(1.0f, 1.0f, 1.0f);
        Vector3 specular = Vector3(1.0f, 1.0f, 1.0f);
        Quaternion orientation = Quaternion(true);
        float distance = 0.0f;
        float intensity = 1.0f;
        float attenuationStart = 1000000.0f;
        float attenuationEnd = 1000000.0f;
        bool attenuationEnabled = false;
        bool orientationExplicit = false;
        bool distanceExplicit = false;
    };

    explicit W3DViewport(QWidget *parent = nullptr);
    ~W3DViewport() override;
    void setRenderObject(RenderObjClass *object);
    RenderObjClass *currentRenderObject() const { return _renderObject; }
    bool animationStatus(int &currentFrame, int &totalFrames, float &fps) const;
    float averageFrameMilliseconds() const;
    void setCameraPosition(CameraPosition position);
    void resetCamera();
    void setAllowedCameraRotation(CameraRotation rotation);
    CameraRotation allowedCameraRotation() const;
    void setAutoResetEnabled(bool enabled);
    bool isAutoResetEnabled() const;
    void requestOneTimeCameraReset();
    void setCameraAnimationEnabled(bool enabled);
    bool isCameraAnimationEnabled() const;
    void setCameraBonePosX(bool enabled);
    bool isCameraBonePosX() const;
    void setManualFovEnabled(bool enabled);
    bool isManualFovEnabled() const;
    void setManualClipPlanesEnabled(bool enabled);
    bool isManualClipPlanesEnabled() const;
    void setCameraFovDegrees(double hfov_deg, double vfov_deg);
    void cameraFovDegrees(double &hfov_deg, double &vfov_deg) const;
    void setCameraClipPlanes(float znear, float zfar);
    void cameraClipPlanes(float &znear, float &zfar) const;
    void resetFov();
    void setCameraDistance(float distance);
    float cameraDistance() const;
    float currentScreenSize() const;
    void setInitialDisplayMode(int width, int height, int bitsPerPixel, bool fullscreen);
    bool applyResolution(int width, int height, int bitsPerPixel, bool fullscreen);
    bool addToLineup(RenderObjClass *object);
    bool canLineUpClass(int class_id) const;
    void clearLineup();
    void setObjectRotationFlags(int flags);
    int objectRotationFlags() const;
    void resetObjectTransform();
    void toggleAlternateMaterials();
    void setLightRotationFlags(int flags);
    int lightRotationFlags() const;
    void setBackgroundColor(const Vector3 &color);
    Vector3 backgroundColor() const;
    void setBackgroundBitmap(const QString &path);
    QString backgroundBitmap() const;
    void setBackgroundObjectName(const QString &name);
    QString backgroundObjectName() const;
    void setAmbientLight(const Vector3 &color);
    Vector3 ambientLight() const;
    void setFogEnabled(bool enabled);
    bool isFogEnabled() const;
    bool sceneFogRange(float &start, float &end) const;
    void setAnimationState(AnimationState state);
    AnimationState animationState() const;
    void setAnimationSpeed(float speed);
    float animationSpeed() const;
    void setAnimationBlend(bool enabled);
    bool animationBlend() const;
    bool stepAnimation(int delta);
    bool hasAnimation() const;
    QString currentAnimationName() const;
    bool toggleSubobjectLod();
    bool isSubobjectLodBound() const;
    void setSceneLightDiffuse(const Vector3 &color);
    Vector3 sceneLightDiffuse() const;
    void setSceneLightSpecular(const Vector3 &color);
    Vector3 sceneLightSpecular() const;
    void setSceneLightColor(const Vector3 &color);
    Vector3 sceneLightColor() const;
    void setSceneLightOrientation(const Quaternion &orientation);
    Quaternion sceneLightOrientation() const;
    void setSceneLightDistance(float distance);
    float sceneLightDistance() const;
    void setSceneLightIntensity(float intensity);
    float sceneLightIntensity() const;
    void setSceneLightAttenuation(float start, float end, bool enabled);
    void sceneLightAttenuation(float &start, float &end, bool &enabled) const;
    SceneLightState sceneLightState() const;
    void setSceneLightState(const SceneLightState &state);
    void setAnimation(HAnimClass *animation);
    void setAnimationCombo(HAnimComboClass *combo);
    void clearAnimation();
    void setWireframeEnabled(bool enabled);
    bool isWireframeEnabled() const;
    void setLodAutoSwitchingEnabled(bool enabled);
    bool isLodAutoSwitchingEnabled() const;
    bool currentLodInfo(int &level, int &count) const;
    bool setNullLodIncluded(bool enabled);
    bool isNullLodIncluded() const;
    bool recordLodScreenArea();
    bool adjustLodLevel(int delta);
    int captureScreenshot(const QString &basePath);
    bool captureMovie(const QString &baseName, float frameRate, QString *error = nullptr);

signals:
    void animationStateChanged();
    void objectCameraReset();

protected:
    QPaintEngine *paintEngine() const override;
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private slots:
    void renderFrame();

private:
    void initWW3D();
    void shutdownWW3D();
    void initScene();
    void shutdownScene();
    void addRenderObjectToDisplayScene(RenderObjClass *object);
    void removeRenderObjectFromDisplayScene(RenderObjClass *object);
    void renderScene(bool present = true);
    void renderFrameWithTicks(int ticks, bool present = true);
    bool trySetDeviceResolution(int width,
                                int height,
                                int bitsPerPixel,
                                int &actualWidth,
                                int &actualHeight,
                                int &actualBitsPerPixel);
    void updateCameraFov(int width, int height, bool force = false);
    void resetCameraToObject(RenderObjClass &object);
    void setFogNearAndRecalculate(float nearClip);
    void applySceneLightSettings();
    void updateSceneLightPosition(const Vector3 &oldCenter);
    void updateBackgroundObjectCamera();
    void syncLightMesh();
    void setLightMeshVisible(bool visible);
    void updateInteractionCursor();
    void updateAnimation(float deltaSeconds);
    void updateCameraAnimation();
    void updateObjectRotation();
    void updateLightRotation();
    void refreshBackgroundBitmap();
    void applyAnimationFrame(float frame);
    void updateFrameTiming(float elapsedMs);

    QTimer _timer;
    QElapsedTimer _elapsed;
    float _frameTimeAccumMs = 0.0f;
    int _frameTimeSamples = 0;
    float _averageFrameMs = 0.0f;
    bool _initialized = false;
    SceneClass *_scene = nullptr;
    CameraClass *_camera = nullptr;
    LightClass *_sceneLight = nullptr;
    RenderObjClass *_lightMesh = nullptr;
    DazzleLayerClass *_dazzleLayer = nullptr;
    RenderObjClass *_renderObject = nullptr;
    HAnimClass *_animation = nullptr;
    HAnimComboClass *_animationCombo = nullptr;
    float _animationFrame = 0.0f;
    float _animationTime = 0.0f;
    bool _animationBlend = true;
    SceneClass *_backgroundScene = nullptr;
    CameraClass *_backgroundCamera = nullptr;
    Bitmap2DObjClass *_backgroundBitmapObj = nullptr;
    SceneClass *_preview2DScene = nullptr;
    CameraClass *_preview2DCamera = nullptr;
    SceneClass *_backgroundObjectScene = nullptr;
    CameraClass *_backgroundObjectCamera = nullptr;
    RenderObjClass *_backgroundObject = nullptr;
    Vector3 _clearColor = Vector3(0.5f, 0.5f, 0.5f);
    QString _backgroundBitmap;
    QString _backgroundObjectName;
    Vector3 _ambientLight = Vector3(0.5f, 0.5f, 0.5f);
    Vector3 _sceneLightDiffuse = Vector3(1.0f, 1.0f, 1.0f);
    Vector3 _sceneLightSpecular = Vector3(1.0f, 1.0f, 1.0f);
    Quaternion _sceneLightOrientation = Quaternion(true);
    float _sceneLightDistance = 0.0f;
    float _sceneLightIntensity = 1.0f;
    float _sceneLightAttenStart = 1000000.0f;
    float _sceneLightAttenEnd = 1000000.0f;
    bool _sceneLightAttenEnabled = false;
    bool _sceneLightOrientationSet = false;
    bool _sceneLightDistanceSet = false;
    bool _lightMeshInScene = false;
    float _lightMeshScale = 1.0f;
    bool _fogEnabled = false;
    bool _wireframeEnabled = false;
    bool _autoResetCamera = true;
    bool _oneTimeCameraReset = true;
    bool _animateCamera = false;
    bool _cameraBonePosX = false;
    bool _manualFov = false;
    bool _manualClipPlanes = false;
    CameraRotation _allowedRotation = CameraRotation::Free;
    bool _windowed = true;
    bool _fullscreen = false;
    bool _leftDown = false;
    bool _rightDown = false;
    QPoint _lastPos;
    float _cameraDistance = 0.0f;
    float _minZoomAdjust = 0.0f;
    Vector3 _orbitCenter = Vector3(0.0f, 0.0f, 0.0f);
    Quaternion _rotation = Quaternion(true);
    int _objectRotation = RotateNone;
    int _lightRotation = RotateNone;
    double _manualHfov = 0.0;
    double _manualVfov = 0.0;
    float _manualNear = 0.2f;
    float _manualFar = 10000.0f;
    int _initialDisplayWidth = 0;
    int _initialDisplayHeight = 0;
    int _bitsPerPixel = 32;
    bool _allowLodSwitching = false;
    AnimationState _animationState = AnimationState::Playing;
    float _animationSpeed = 1.0f;
};
