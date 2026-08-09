#include "W3DViewport.h"

#include "RenderObjUtils.h"

#include "agg_def.h"
#include "assetmgr.h"
#include "bmp2d.h"
#include "camera.h"
#include "dazzle.h"
#include "distlod.h"
#include "hanim.h"
#include "hlod.h"
#include "light.h"
#include "matrix3d.h"
#include "part_ldr.h"
#include "rcfile.h"
#include "refcount.h"
#include "rendobj.h"
#include "mesh.h"
#include "meshmdl.h"
#include "part_emt.h"
#include "ringobj.h"
#include "scene.h"
#include "SoundScene.h"
#include "soundrobj.h"
#include "sphereobj.h"
#include "vector2.h"
#include "vector3.h"
#include "WWAudio.h"
#include "ww3d.h"
#include "wwmath.h"

#include <QPaintEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QShowEvent>
#include <QMouseEvent>
#include <QDir>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>

#ifdef _WIN32
#include <windows.h>
#endif

namespace {
constexpr double kPi = 3.14159265358979323846;
constexpr double kRadToDeg = 180.0 / kPi;
constexpr double kDegToRad = kPi / 180.0;
constexpr const char *kCameraBoneName = "CAMERA";

Matrix3D BuildCameraBoneTransform(const Matrix3D &bone_transform)
{
    Matrix3D cam_transform(Vector3(0.0f, -1.0f, 0.0f),
                           Vector3(0.0f, 0.0f, 1.0f),
                           Vector3(-1.0f, 0.0f, 0.0f),
                           Vector3(0.0f, 0.0f, 0.0f));
    return bone_transform * cam_transform;
}

bool GetCameraTransform(RenderObjClass *render_obj, Matrix3D &tm)
{
    if (!render_obj) {
        return false;
    }

    const int count = render_obj->Get_Num_Sub_Objects();
    for (int index = 0; index < count; ++index) {
        RenderObjClass *sub_obj = render_obj->Get_Sub_Object(index);
        if (!sub_obj) {
            continue;
        }

        const bool found = GetCameraTransform(sub_obj, tm);
        sub_obj->Release_Ref();
        if (found) {
            return true;
        }
    }

    const int bone_index = render_obj->Get_Bone_Index(kCameraBoneName);
    if (bone_index > 0) {
        tm = render_obj->Get_Bone_Transform(bone_index);
        return true;
    }

    return false;
}

bool Is2DPreviewObject(const RenderObjClass *render_obj)
{
    return render_obj && render_obj->Class_ID() == RenderObjClass::CLASSID_BITMAP2D;
}

SphereClass BuildEmitterDisplaySphere(const ParticleEmitterClass &emitter)
{
    const Vector3 velocity = emitter.Get_Start_Velocity();
    const Vector3 acceleration = emitter.Get_Acceleration();
    const float lifetime = emitter.Get_Lifetime();

    Vector3 distance =
        (velocity * lifetime) + ((acceleration * (lifetime * lifetime)) / 2.0f);
    Vector3 distance_maxima(0.0f, 0.0f, 0.0f);

    if (acceleration.X != 0.0f || acceleration.Y != 0.0f || acceleration.Z != 0.0f) {
        const Vector3 time_max(
            acceleration.X != 0.0f ? -velocity.X / acceleration.X : 0.0f,
            acceleration.Y != 0.0f ? -velocity.Y / acceleration.Y : 0.0f,
            acceleration.Z != 0.0f ? -velocity.Z / acceleration.Z : 0.0f);

        if (time_max.X >= 0.0f && time_max.X < lifetime) {
            distance_maxima.X = std::fabs(
                (velocity.X * time_max.X)
                + ((acceleration.X * time_max.X * time_max.X) / 2.0f));
        }
        if (time_max.Y >= 0.0f && time_max.Y < lifetime) {
            distance_maxima.Y = std::fabs(
                (velocity.Y * time_max.Y)
                + ((acceleration.Y * time_max.Y * time_max.Y) / 2.0f));
        }
        if (time_max.Z >= 0.0f && time_max.Z < lifetime) {
            distance_maxima.Z = std::fabs(
                (velocity.Z * time_max.Z)
                + ((acceleration.Z * time_max.Z * time_max.Z) / 2.0f));
        }
    }

    distance.X = std::fabs(distance.X);
    distance.Y = std::fabs(distance.Y);
    distance.Z = std::fabs(distance.Z);

    float max_distance = std::max(distance.X, distance.Y);
    max_distance = std::max(max_distance, distance.Z);
    max_distance = std::max(max_distance, distance_maxima.X);
    max_distance = std::max(max_distance, distance_maxima.Y);
    max_distance = std::max(max_distance, distance_maxima.Z);

    Vector3 center = distance / 2.0f;
    center.X = std::max(center.X, distance_maxima.X / 2.0f);
    center.Y = std::max(center.Y, distance_maxima.Y / 2.0f);
    center.Z = std::max(center.Z, distance_maxima.Z / 2.0f);

    SphereClass sphere;
    sphere.Center = center;
    sphere.Radius =
        std::max(emitter.Get_Particle_Size() * 5.0f, (max_distance * 3.0f) / 5.0f);
    return sphere;
}

void StopAndDetachEmitterBuffers(RenderObjClass *render_obj)
{
    if (!render_obj) {
        return;
    }

    const int sub_object_count = render_obj->Get_Num_Sub_Objects();
    for (int index = 0; index < sub_object_count; ++index) {
        RenderObjClass *sub_object = render_obj->Get_Sub_Object(index);
        if (sub_object) {
            StopAndDetachEmitterBuffers(sub_object);
            sub_object->Release_Ref();
        }
    }

    if (render_obj->Class_ID() == RenderObjClass::CLASSID_PARTICLEEMITTER) {
        auto *emitter = static_cast<ParticleEmitterClass *>(render_obj);
        emitter->Stop();
        emitter->Remove_Buffer_From_Scene();
        emitter->Buffer_Scene_Not_Needed();
    }
}

void ToggleAlternateMaterials(RenderObjClass *render_obj)
{
    if (!render_obj) {
        return;
    }

    if (render_obj->Class_ID() == RenderObjClass::CLASSID_MESH) {
        auto *mesh = static_cast<MeshClass *>(render_obj);
        MeshModelClass *model = mesh->Get_Model();
        if (model) {
            model->Enable_Alternate_Material_Description(
                !model->Is_Alternate_Material_Description_Enabled());
        }
    }

    const int count = render_obj->Get_Num_Sub_Objects();
    for (int index = 0; index < count; ++index) {
        RenderObjClass *sub_obj = render_obj->Get_Sub_Object(index);
        if (sub_obj) {
            ToggleAlternateMaterials(sub_obj);
            sub_obj->Release_Ref();
        }
    }
}

class W3DViewScene final : public SimpleSceneClass
{
public:
    void SetAllowLodSwitching(bool enabled) { _allowLodSwitching = enabled; }
    bool IsAllowLodSwitching() const { return _allowLodSwitching; }

    void Visibility_Check(CameraClass *camera) override
    {
        RefRenderObjListIterator it(&RenderList);
        for (it.First(); !it.Is_Done(); it.Next()) {
            RenderObjClass *robj = it.Peek_Obj();
            if (!robj) {
                continue;
            }

            if (robj->Is_Force_Visible()) {
                robj->Set_Visible(true);
            } else {
                robj->Set_Visible(!camera->Cull_Sphere(robj->Get_Bounding_Sphere()));
            }

            const int lod_level = robj->Get_LOD_Level();
            if (robj->Is_Really_Visible()) {
                robj->Prepare_LOD(*camera);
            }

            if (!_allowLodSwitching) {
                robj->Set_LOD_Level(lod_level);
            }
        }

        Visibility_Checked = true;
    }

    void Add_Render_Object(RenderObjClass *obj) override
    {
        SimpleSceneClass::Add_Render_Object(obj);
        Recalculate_Fog_Planes();
    }

    void Add_To_Lineup(RenderObjClass *obj)
    {
        if (!obj || !Can_Line_Up(obj)) {
            return;
        }

        AABoxClass obj_box;
        obj->Get_Obj_Space_Bounding_Box(obj_box);
        const float obj_width = obj_box.Extent.Y * 2.0f;

        const AABoxClass scene_box = Get_Line_Up_Bounding_Box();
        const float scene_width = scene_box.Extent.Y * 2.0f;

        const float new_scene_width = scene_width + obj_width + obj_width / 3.0f;
        const float delta = (new_scene_width - scene_width) / 2.0f;

        int existing_objects = 0;
        SceneIterator *it = Create_Iterator();
        if (it) {
            for (it->First(); !it->Is_Done(); it->Next()) {
                RenderObjClass *current = it->Current_Item();
                if (!current || !Can_Line_Up(current)) {
                    continue;
                }
                Vector3 pos = current->Get_Position();
                pos.Y -= delta;
                current->Set_Position(pos);
                ++existing_objects;
            }
            Destroy_Iterator(it);
        }

        if (existing_objects > 0) {
            obj->Set_Position(Vector3(0.0f, new_scene_width / 2.0f - obj_box.Extent.Y, 0.0f));
        } else {
            obj->Set_Position(Vector3(0.0f, 0.0f, 0.0f));
        }

        Add_Render_Object(obj);
        _lineupList.Add(obj);
    }

    void Clear_Lineup()
    {
        RenderObjClass *obj = nullptr;
        while ((obj = _lineupList.Remove_Head()) != nullptr) {
            StopAndDetachEmitterBuffers(obj);
            Remove_Render_Object(obj);
            obj->Release_Ref();
        }
        Recalculate_Fog_Planes();
    }

    bool Can_Line_Up(RenderObjClass *obj) const
    {
        return obj && Can_Line_Up(obj->Class_ID());
    }

    bool Can_Line_Up(int class_id) const
    {
        return class_id == RenderObjClass::CLASSID_HMODEL ||
               class_id == RenderObjClass::CLASSID_HLOD;
    }

    AABoxClass Get_Line_Up_Bounding_Box()
    {
        AABoxClass sum_of_boxes(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f));
        SceneIterator *it = Create_Iterator();
        if (it) {
            for (it->First(); !it->Is_Done(); it->Next()) {
                RenderObjClass *current = it->Current_Item();
                if (current && Can_Line_Up(current)) {
                    sum_of_boxes.Add_Box(current->Get_Bounding_Box());
                }
            }
            Destroy_Iterator(it);
        }
        return sum_of_boxes;
    }

    SphereClass Get_Bounding_Sphere()
    {
        SphereClass bounding_sphere(Vector3(0.0f, 0.0f, 0.0f), 0.0f);
        SceneIterator *it = Create_Iterator();
        if (it) {
            for (it->First(); !it->Is_Done(); it->Next()) {
                RenderObjClass *current = it->Current_Item();
                if (!current) {
                    continue;
                }
                if (current->Class_ID() != RenderObjClass::CLASSID_LIGHT) {
                    bounding_sphere.Add_Sphere(current->Get_Bounding_Sphere());
                }
            }
            Destroy_Iterator(it);
        }
        return bounding_sphere;
    }

    void Recalculate_Fog_Planes()
    {
        const float kFogOpaqueMultiple = 8.0f;
        const float kFogMinimumDepth = 200.0f;
        float fog_near = 0.0f;
        float fog_far = 0.0f;
        Get_Fog_Range(&fog_near, &fog_far);

        const SphereClass sphere = Get_Bounding_Sphere();
        fog_far = sphere.Radius * kFogOpaqueMultiple;
        if (fog_far < fog_near + kFogMinimumDepth) {
            fog_far = fog_near + kFogMinimumDepth;
        }
        Set_Fog_Range(fog_near, fog_far);
    }

private:
    bool _allowLodSwitching = false;
    RefRenderObjListClass _lineupList;
};

void SwitchLod(RenderObjClass *render_obj, int increment, bool &switched)
{
    if (!render_obj) {
        return;
    }

    const int count = render_obj->Get_Num_Sub_Objects();
    for (int index = 0; index < count; ++index) {
        RenderObjClass *sub_obj = render_obj->Get_Sub_Object(index);
        if (sub_obj) {
            SwitchLod(sub_obj, increment, switched);
            sub_obj->Release_Ref();
        }
    }

    if (render_obj->Class_ID() == RenderObjClass::CLASSID_HLOD) {
        auto *hlod = static_cast<HLodClass *>(render_obj);
        hlod->Set_LOD_Level(hlod->Get_LOD_Level() + increment);
        switched = true;
    }
}
} // namespace

static void SetLowestLod(RenderObjClass *render_obj);
static void ResetSceneLod(SceneClass *scene);

W3DViewport::W3DViewport(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAutoFillBackground(false);
    setFocusPolicy(Qt::StrongFocus);

    _timer.setInterval(16);
    connect(&_timer, &QTimer::timeout, this, &W3DViewport::renderFrame);
}

W3DViewport::~W3DViewport()
{
    clearAnimation();
    shutdownWW3D();
}

QPaintEngine *W3DViewport::paintEngine() const
{
    return nullptr;
}

void W3DViewport::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    initWW3D();

    if (_initialized && !_timer.isActive()) {
        _elapsed.restart();
        _timer.start();
    }
}

void W3DViewport::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (_initialized && _windowed) {
        const int render_width = _fullscreen && _initialDisplayWidth > 0
            ? _initialDisplayWidth
            : width();
        const int render_height = _fullscreen && _initialDisplayHeight > 0
            ? _initialDisplayHeight
            : height();
        int actual_width = 0;
        int actual_height = 0;
        int actual_bits_per_pixel = 0;
        if (trySetDeviceResolution(render_width,
                                   render_height,
                                   _bitsPerPixel,
                                   actual_width,
                                   actual_height,
                                   actual_bits_per_pixel)) {
            _bitsPerPixel = actual_bits_per_pixel;
            if (!_fullscreen) {
                _initialDisplayWidth = actual_width;
                _initialDisplayHeight = actual_height;
            }
            updateCameraFov(actual_width, actual_height);
        }
    }
}

void W3DViewport::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    if (_timer.isActive()) {
        _timer.stop();
    }
}

void W3DViewport::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
}

void W3DViewport::focusOutEvent(QFocusEvent *event)
{
    setLightMeshVisible(false);
    if (!_leftDown && !_rightDown) {
        unsetCursor();
    }
    QWidget::focusOutEvent(event);
}

void W3DViewport::keyPressEvent(QKeyEvent *event)
{
    if (event && event->key() == Qt::Key_Control) {
        setLightMeshVisible(true);
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

void W3DViewport::keyReleaseEvent(QKeyEvent *event)
{
    if (event && event->key() == Qt::Key_Control) {
        setLightMeshVisible(false);
        event->accept();
        return;
    }
    QWidget::keyReleaseEvent(event);
}

void W3DViewport::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        _leftDown = true;
    } else if (event->button() == Qt::RightButton) {
        _rightDown = true;
    }

    _lastPos = event->pos();
    updateInteractionCursor();
    grabMouse();
    event->accept();
}

void W3DViewport::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        _leftDown = false;
    } else if (event->button() == Qt::RightButton) {
        _rightDown = false;
    }

    if (!_leftDown && !_rightDown) {
        releaseMouse();
    }
    updateInteractionCursor();

    event->accept();
}

void W3DViewport::mouseMoveEvent(QMouseEvent *event)
{
    if (!_initialized || !_camera) {
        _lastPos = event->pos();
        return;
    }

    const QPoint current = event->pos();
    const int delta_x = _lastPos.x() - current.x();
    const int delta_y = _lastPos.y() - current.y();

    const bool control_down = event->modifiers().testFlag(Qt::ControlModifier);

    setLightMeshVisible(control_down);

    if (_leftDown && _rightDown) {
        const float mid_x = width() * 0.5f;
        const float mid_y = height() * 0.5f;
        if (mid_x > 0.0f && mid_y > 0.0f) {
            const float last_x = (static_cast<float>(_lastPos.x()) - mid_x) / mid_x;
            const float last_y = (mid_y - static_cast<float>(_lastPos.y())) / mid_y;
            const float point_x = (static_cast<float>(current.x()) - mid_x) / mid_x;
            const float point_y = (mid_y - static_cast<float>(current.y())) / mid_y;

            const Vector3 camera_pan(-1.0f * _cameraDistance * (point_x - last_x),
                                     -1.0f * _cameraDistance * (point_y - last_y),
                                     0.0f);

            Matrix3D transform = _camera->Get_Transform();
            transform.Translate(camera_pan);

            const Matrix3 view = Build_Matrix3(_rotation);
            const Vector3 move = view * camera_pan;
            _orbitCenter += move;

            _camera->Set_Transform(transform);
        }
    } else if (control_down && _leftDown && _sceneLight) {
        const float mid_x = width() * 0.5f;
        const float mid_y = height() * 0.5f;
        if (mid_x > 0.0f && mid_y > 0.0f) {
            const float last_x = (static_cast<float>(_lastPos.x()) - mid_x) / mid_x;
            const float last_y = (mid_y - static_cast<float>(_lastPos.y())) / mid_y;
            const float point_x = (static_cast<float>(current.x()) - mid_x) / mid_x;
            const float point_y = (mid_y - static_cast<float>(current.y())) / mid_y;

            const Quaternion mouse_motion = Inverse(Trackball(last_x, last_y, point_x, point_y, 0.8f));
            const Quaternion camera_orientation = Build_Quaternion(_camera->Get_Transform());
            const Quaternion current_light = Build_Quaternion(_sceneLight->Get_Transform());

            Quaternion light_orientation = camera_orientation * mouse_motion;
            light_orientation = light_orientation * Inverse(camera_orientation);
            light_orientation = light_orientation * current_light;
            light_orientation.Normalize();

            Vector3 center_in_light_space;
            const Matrix3D current_transform = _sceneLight->Get_Transform();
            Matrix3D::Inverse_Transform_Vector(
                current_transform, _orbitCenter, &center_in_light_space);

            Matrix3D light_transform(light_orientation, _orbitCenter);
            light_transform.Translate(-center_in_light_space);
            _sceneLight->Set_Transform(light_transform);
            _sceneLightOrientation = light_orientation;
            _sceneLightOrientationSet = true;
            _sceneLightDistance = (_sceneLight->Get_Position() - _orbitCenter).Length();
            _sceneLightDistanceSet = true;
            syncLightMesh();
        }
    } else if (control_down && _rightDown && _sceneLight && _renderObject) {
        if (height() > 0 && delta_y != 0) {
            const float radius = std::max(0.0f, _renderObject->Get_Bounding_Sphere().Radius);
            const float adjustment = (static_cast<float>(delta_y) / static_cast<float>(height()))
                * radius * 3.0f;

            Matrix3D light_transform = _sceneLight->Get_Transform();
            light_transform.Translate(Vector3(0.0f, 0.0f, adjustment));
            const float new_distance =
                (light_transform.Get_Translation() - _renderObject->Get_Position()).Length();
            if (new_distance > radius) {
                _sceneLight->Set_Transform(light_transform);
                _sceneLightDistance = (_sceneLight->Get_Position() - _orbitCenter).Length();
                _sceneLightDistanceSet = true;
                syncLightMesh();
            }
        }
    } else if (_leftDown) {
        const float mid_x = width() * 0.5f;
        const float mid_y = height() * 0.5f;
        if (mid_x > 0.0f && mid_y > 0.0f) {
            const float last_x = (static_cast<float>(_lastPos.x()) - mid_x) / mid_x;
            const float last_y = (mid_y - static_cast<float>(_lastPos.y())) / mid_y;
            const float point_x = (static_cast<float>(current.x()) - mid_x) / mid_x;
            const float point_y = (mid_y - static_cast<float>(current.y())) / mid_y;

            Quaternion rotation = Trackball(last_x, last_y, point_x, point_y, 0.8f);

            if (_allowedRotation == CameraRotation::OnlyX) {
                Matrix3D temp_matrix = Build_Matrix3D(rotation);
                Matrix3D temp_matrix2(1);
                temp_matrix2.Rotate_X(temp_matrix.Get_X_Rotation());
                temp_matrix2.Set_Translation(temp_matrix.Get_Translation());
                rotation = Build_Quaternion(temp_matrix2);
            } else if (_allowedRotation == CameraRotation::OnlyY) {
                Matrix3D temp_matrix = Build_Matrix3D(rotation);
                Matrix3D temp_matrix2(1);
                temp_matrix2.Rotate_Y(temp_matrix.Get_Y_Rotation());
                temp_matrix2.Set_Translation(temp_matrix.Get_Translation());
                rotation = Build_Quaternion(temp_matrix2);
            } else if (_allowedRotation == CameraRotation::OnlyZ) {
                Matrix3D temp_matrix = Build_Matrix3D(rotation);
                Matrix3D temp_matrix2(1);
                temp_matrix2.Rotate_Z(temp_matrix.Get_Z_Rotation());
                temp_matrix2.Set_Translation(temp_matrix.Get_Translation());
                rotation = Build_Quaternion(temp_matrix2);
            }

            _rotation = rotation;

            Matrix3D transform = _camera->Get_Transform();
            Matrix3D inverse;
            transform.Get_Orthogonal_Inverse(inverse);

            const Vector3 to_object = inverse * _orbitCenter;
            transform.Translate(to_object);
            Matrix3D::Multiply(transform, Build_Matrix3D(rotation), &transform);
            transform.Translate(-to_object);

            _camera->Set_Transform(transform);
        }
    } else if (_rightDown) {
        if (height() > 0 && delta_y != 0) {
            Matrix3D transform = _camera->Get_Transform();
            const float delta = static_cast<float>(delta_y) / static_cast<float>(height());
            float adjustment = delta * _cameraDistance * 3.0f;

            if ((adjustment < _minZoomAdjust) && (adjustment >= 0.0f)) {
                adjustment = _minZoomAdjust;
            }
            if ((adjustment > -_minZoomAdjust) && (adjustment <= 0.0f)) {
                adjustment = -_minZoomAdjust;
            }

            if ((_cameraDistance + adjustment) > 0.0f) {
                _cameraDistance += adjustment;
                transform.Translate(Vector3(0.0f, 0.0f, adjustment));
                _camera->Set_Transform(transform);
            }
        }
    }

    _lastPos = current;
    event->accept();
}

void W3DViewport::wheelEvent(QWheelEvent *event)
{
    if (!_initialized || !_camera) {
        return;
    }

    const int delta = event->angleDelta().y();
    if (delta == 0) {
        return;
    }

    Matrix3D transform = _camera->Get_Transform();
    const float steps = static_cast<float>(delta) / 120.0f;
    float adjustment = -steps * _cameraDistance * 0.15f;

    if ((adjustment < _minZoomAdjust) && (adjustment >= 0.0f)) {
        adjustment = _minZoomAdjust;
    }
    if ((adjustment > -_minZoomAdjust) && (adjustment <= 0.0f)) {
        adjustment = -_minZoomAdjust;
    }

    if ((_cameraDistance + adjustment) > 0.0f) {
        _cameraDistance += adjustment;
        transform.Translate(Vector3(0.0f, 0.0f, adjustment));
        _camera->Set_Transform(transform);
    }

    event->accept();
}

void W3DViewport::renderFrame()
{
    if (!_initialized) {
        return;
    }

    const qint64 elapsed_ms = _elapsed.restart();
    updateFrameTiming(static_cast<float>(elapsed_ms));
    if (elapsed_ms > 0) {
        const auto next_time = WW3D::Get_Sync_Time() + static_cast<unsigned int>(elapsed_ms);
        WW3D::Sync(next_time);
        updateAnimation(static_cast<float>(elapsed_ms) / 1000.0f);
    }

    updateCameraAnimation();
    updateObjectRotation();
    updateLightRotation();
    renderScene();
}

void W3DViewport::renderScene(bool present)
{
    if (_allowLodSwitching && _scene) {
        ResetSceneLod(_scene);
    }

    WW3D::Begin_Render(true, true, _clearColor);
    if (_backgroundScene && _backgroundCamera) {
        WW3D::Render(_backgroundScene, _backgroundCamera, false, false);
    }
    if (_preview2DScene && _preview2DCamera) {
        WW3D::Render(_preview2DScene, _preview2DCamera, false, false);
    }
    if (_backgroundObjectScene && _backgroundObjectCamera) {
        updateBackgroundObjectCamera();
        WW3D::Render(_backgroundObjectScene, _backgroundObjectCamera, false, false);
    }
    if (_scene && _camera) {
        WW3D::Render(_scene, _camera, false, false);
        if (_dazzleLayer) {
            _dazzleLayer->Render(_camera);
        }
    }
    WW3D::End_Render(present);

    if (auto *audio = WWAudioClass::Get_Instance()) {
        audio->On_Frame_Update();
    }
}

int W3DViewport::captureScreenshot(const QString &basePath)
{
    if (!_initialized || basePath.isEmpty()) {
        return 0;
    }

    // A discard swap chain does not preserve the back buffer after Present.
    // Render the current scene without presenting so the screenshot contains
    // the exact viewport frame. The active frame timer will render and present
    // the next display frame; only render synchronously when that timer is off.
    renderScene(false);
    const QByteArray native = QDir::toNativeSeparators(basePath).toLocal8Bit();
    const int screenshot_number = WW3D::Make_Back_Buffer_Screen_Shot(native.constData());
    if (!_timer.isActive()) {
        renderScene(true);
    }
    return screenshot_number;
}

void W3DViewport::renderFrameWithTicks(int ticks, bool present)
{
    if (!_initialized) {
        return;
    }

    if (ticks > 0) {
        updateFrameTiming(static_cast<float>(ticks));
        const auto next_time = WW3D::Get_Sync_Time() + static_cast<unsigned int>(ticks);
        WW3D::Sync(next_time);
        updateAnimation(static_cast<float>(ticks) / 1000.0f);
    }

    updateCameraAnimation();
    updateObjectRotation();
    updateLightRotation();
    renderScene(present);
}

void W3DViewport::updateFrameTiming(float elapsedMs)
{
    if (elapsedMs <= 0.0f) {
        return;
    }

    _frameTimeAccumMs += elapsedMs;
    ++_frameTimeSamples;

    if (_frameTimeAccumMs >= 1000.0f) {
        _averageFrameMs = _frameTimeAccumMs / static_cast<float>(_frameTimeSamples);
        _frameTimeAccumMs = 0.0f;
        _frameTimeSamples = 0;
    }
}

static void SetLowestLod(RenderObjClass *render_obj)
{
    if (!render_obj) {
        return;
    }

    const int count = render_obj->Get_Num_Sub_Objects();
    for (int index = 0; index < count; ++index) {
        RenderObjClass *sub_obj = render_obj->Get_Sub_Object(index);
        if (sub_obj) {
            SetLowestLod(sub_obj);
            sub_obj->Release_Ref();
        }
    }

    if (render_obj->Class_ID() == RenderObjClass::CLASSID_HLOD) {
        static_cast<HLodClass *>(render_obj)->Set_LOD_Level(0);
    }
}

static void ResetSceneLod(SceneClass *scene)
{
    if (!scene) {
        return;
    }

    SceneIterator *it = scene->Create_Iterator();
    if (!it) {
        return;
    }

    for (it->First(); !it->Is_Done(); it->Next()) {
        RenderObjClass *obj = it->Current_Item();
        SetLowestLod(obj);
    }

    scene->Destroy_Iterator(it);
}

void W3DViewport::initWW3D()
{
    if (_initialized) {
        return;
    }

    createWinId();
    void *hwnd = reinterpret_cast<void *>(winId());

    if (WW3D::Init(hwnd, nullptr, false) != WW3D_ERROR_OK) {
        return;
    }

    WW3D::Enable_Static_Sort_Lists(true);

    const int render_width = _fullscreen && _initialDisplayWidth > 0
        ? _initialDisplayWidth
        : width();
    const int render_height = _fullscreen && _initialDisplayHeight > 0
        ? _initialDisplayHeight
        : height();
    if (WW3D::Set_Render_Device(-1,
                                render_width,
                                render_height,
                                _bitsPerPixel,
                                1,
                                true) != WW3D_ERROR_OK) {
        WW3D::Shutdown();
        return;
    }

    int actual_width = render_width;
    int actual_height = render_height;
    int actual_bits_per_pixel = _bitsPerPixel;
    bool actual_windowed = _windowed;
    WW3D::Get_Device_Resolution(
        actual_width, actual_height, actual_bits_per_pixel, actual_windowed);
    _windowed = actual_windowed;
    _bitsPerPixel = actual_bits_per_pixel;
    _initialDisplayWidth = actual_width;
    _initialDisplayHeight = actual_height;

    if (auto *asset_manager = WW3DAssetManager::Get_Instance()) {
        asset_manager->Load_Procedural_Textures();

        ResourceFileClass light_mesh_file("Light.w3d");
        if (light_mesh_file.Is_Available(0)
            && asset_manager->Load_3D_Assets(light_mesh_file)) {
            _lightMesh = asset_manager->Create_Render_Obj("LIGHT");
        }
    }

    if (DazzleRenderObjClass::Get_Type_Class(0)) {
        _dazzleLayer = new DazzleLayerClass();
        DazzleRenderObjClass::Set_Current_Dazzle_Layer(_dazzleLayer);
        DazzleRenderObjClass::Enable_Dazzle_Rendering(true);
    } else {
        DazzleRenderObjClass::Set_Current_Dazzle_Layer(nullptr);
        DazzleRenderObjClass::Enable_Dazzle_Rendering(false);
    }

    initScene();
    updateCameraFov(actual_width, actual_height);
    _initialized = true;
}

void W3DViewport::shutdownWW3D()
{
    if (!_initialized) {
        // A hidden/offscreen viewport can still receive a render object through
        // tree selection even though showEvent() never initialized WW3D.  Release
        // those scene-independent references before the asset manager shuts down.
        shutdownScene();
        REF_PTR_RELEASE(_lightMesh);
        return;
    }

    _timer.stop();
    shutdownScene();
    REF_PTR_RELEASE(_lightMesh);
    if (_dazzleLayer) {
        DazzleRenderObjClass::Set_Current_Dazzle_Layer(nullptr);
        delete _dazzleLayer;
        _dazzleLayer = nullptr;
    }
    DazzleRenderObjClass::Enable_Dazzle_Rendering(false);
    WW3D::Shutdown();
    _initialized = false;
}

void W3DViewport::initScene()
{
    if (_scene || _camera) {
        return;
    }

    ParticleEmitterClass::Set_Default_Remove_On_Complete(false);

    _scene = NEW_REF(W3DViewScene, ());
    setWireframeEnabled(_wireframeEnabled);
    _scene->Set_Ambient_Light(_ambientLight);
    _scene->Set_Fog_Color(_clearColor);
    _scene->Set_Fog_Enable(_fogEnabled);
    setLodAutoSwitchingEnabled(_allowLodSwitching);

    _backgroundScene = NEW_REF(SimpleSceneClass, ());
    _backgroundCamera = NEW_REF(CameraClass, ());
    _backgroundCamera->Set_View_Plane(Vector2(-1.0f, -1.0f), Vector2(1.0f, 1.0f));
    _backgroundCamera->Set_Position(Vector3(0.0f, 0.0f, 1.0f));
    _backgroundCamera->Set_Clip_Planes(0.1f, 10.0f);
    refreshBackgroundBitmap();

    _preview2DScene = NEW_REF(SimpleSceneClass, ());
    _preview2DCamera = NEW_REF(CameraClass, ());
    _preview2DCamera->Set_View_Plane(Vector2(-1.0f, -1.0f), Vector2(1.0f, 1.0f));
    _preview2DCamera->Set_Position(Vector3(0.0f, 0.0f, 1.0f));
    _preview2DCamera->Set_Clip_Planes(0.1f, 10.0f);

    _backgroundObjectScene = NEW_REF(SimpleSceneClass, ());
    _backgroundObjectScene->Set_Ambient_Light(Vector3(0.5f, 0.5f, 0.5f));
    _backgroundObjectCamera = NEW_REF(CameraClass, ());
    _backgroundObjectCamera->Set_View_Plane(Vector2(-1.0f, -1.0f), Vector2(1.0f, 1.0f));
    _backgroundObjectCamera->Set_Position(Vector3(0.0f, 0.0f, 0.0f));
    _backgroundObjectCamera->Set_Clip_Planes(0.1f, 10.0f);
    if (!_backgroundObjectName.isEmpty()) {
        setBackgroundObjectName(_backgroundObjectName);
    }

    _sceneLight = NEW_REF(LightClass, ());
    _sceneLight->Set_Position(Vector3(0.0f, 5000.0f, 3000.0f));
    _sceneLight->Set_Intensity(1.0f);
    _sceneLight->Set_Force_Visible(true);
    _sceneLight->Set_Flag(LightClass::NEAR_ATTENUATION, false);
    _sceneLight->Set_Far_Attenuation_Range(1000000.0f, 1000000.0f);
    _sceneLight->Set_Ambient(Vector3(0.0f, 0.0f, 0.0f));
    _sceneLight->Set_Diffuse(Vector3(1.0f, 1.0f, 1.0f));
    _sceneLight->Set_Specular(Vector3(1.0f, 1.0f, 1.0f));
    _scene->Add_Render_Object(_sceneLight);

    _camera = NEW_REF(CameraClass, ());
    Matrix3D transform(1);
    transform.Look_At(Vector3(35.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), 0);
    _camera->Set_Transform(transform);
    _camera->Set_Clip_Planes(0.2f, 10000.0f);
    updateCameraFov(width(), height());
    if (_manualClipPlanes) {
        setCameraClipPlanes(_manualNear, _manualFar);
    }

    if (auto *audio = WWAudioClass::Get_Instance()) {
        if (auto *sound_scene = audio->Get_Sound_Scene()) {
            sound_scene->Attach_Listener_To_Obj(_camera);
        }
    }

    if (_renderObject) {
        addRenderObjectToDisplayScene(_renderObject);
        if (!Is2DPreviewObject(_renderObject)) {
            resetCameraToObject(*_renderObject);
        }
    }

    applySceneLightSettings();
    syncLightMesh();
}

void W3DViewport::shutdownScene()
{
    if (auto *audio = WWAudioClass::Get_Instance()) {
        if (auto *sound_scene = audio->Get_Sound_Scene()) {
            sound_scene->Attach_Listener_To_Obj(nullptr);
        }
    }

    if (_scene) {
        auto *viewer_scene = static_cast<W3DViewScene *>(_scene);
        viewer_scene->Clear_Lineup();
    }
    if (_renderObject) {
        removeRenderObjectFromDisplayScene(_renderObject);
    }
    if (_scene && _sceneLight) {
        _scene->Remove_Render_Object(_sceneLight);
    }
    if (_scene && _lightMesh && _lightMeshInScene) {
        _scene->Remove_Render_Object(_lightMesh);
        _lightMeshInScene = false;
    }
    if (_backgroundBitmapObj) {
        _backgroundBitmapObj->Remove();
    }
    REF_PTR_RELEASE(_backgroundBitmapObj);
    if (_backgroundObject) {
        _backgroundObject->Remove();
    }
    REF_PTR_RELEASE(_backgroundObject);
    REF_PTR_RELEASE(_backgroundObjectCamera);
    REF_PTR_RELEASE(_backgroundObjectScene);
    REF_PTR_RELEASE(_preview2DCamera);
    REF_PTR_RELEASE(_preview2DScene);
    REF_PTR_RELEASE(_backgroundCamera);
    REF_PTR_RELEASE(_backgroundScene);
    REF_PTR_RELEASE(_animation);
    REF_PTR_RELEASE(_renderObject);
    REF_PTR_RELEASE(_sceneLight);
    REF_PTR_RELEASE(_camera);
    REF_PTR_RELEASE(_scene);
}

void W3DViewport::addRenderObjectToDisplayScene(RenderObjClass *object)
{
    if (!object) {
        return;
    }

    if (Is2DPreviewObject(object)) {
        if (_preview2DScene) {
            _preview2DScene->Add_Render_Object(object);
        }
        return;
    }

    if (!_scene) {
        return;
    }

    _scene->Add_Render_Object(object);
    if (object->Class_ID() == RenderObjClass::CLASSID_PARTICLEEMITTER) {
        auto *emitter = static_cast<ParticleEmitterClass *>(object);
        emitter->Enable_Remove_On_Complete(false);
        emitter->Start();
    }
}

void W3DViewport::removeRenderObjectFromDisplayScene(RenderObjClass *object)
{
    if (!object) {
        return;
    }

    if (Is2DPreviewObject(object)) {
        if (_preview2DScene) {
            _preview2DScene->Remove_Render_Object(object);
        }
        return;
    }

    StopAndDetachEmitterBuffers(object);

    if (_scene) {
        _scene->Remove_Render_Object(object);
    }
}

void W3DViewport::updateCameraFov(int width, int height, bool force)
{
    if (!_camera || width <= 0 || height <= 0) {
        return;
    }

    if (_manualFov && !force) {
        if (_manualHfov > 0.0 && _manualVfov > 0.0) {
            _camera->Set_View_Plane(_manualHfov, _manualVfov);
        }
        return;
    }

    float hfov = DEG_TO_RADF(45.0f);
    float vfov = DEG_TO_RADF(45.0f);

    if (height > width) {
        vfov = DEG_TO_RADF(45.0f);
        hfov = (static_cast<float>(width) / static_cast<float>(height)) * vfov;
    } else {
        hfov = DEG_TO_RADF(45.0f);
        vfov = (static_cast<float>(height) / static_cast<float>(width)) * hfov;
    }

    _camera->Set_View_Plane(hfov, vfov);
}

void W3DViewport::resetCameraToObject(RenderObjClass &object)
{
    if (!_camera) {
        return;
    }

    const SphereClass sphere = object.Class_ID() == RenderObjClass::CLASSID_PARTICLEEMITTER
        ? BuildEmitterDisplaySphere(static_cast<const ParticleEmitterClass &>(object))
        : object.Get_Bounding_Sphere();
    const Vector3 old_center = _orbitCenter;
    _orbitCenter = sphere.Center;
    _cameraDistance = std::max(1.0f, sphere.Radius * 3.0f);
    _minZoomAdjust = _cameraDistance / 190.0f;
    Matrix3D transform(1);
    transform.Look_At(_orbitCenter + Vector3(_cameraDistance, 0.0f, 0.0f), _orbitCenter, 0);
    _rotation = Build_Quaternion(transform);
    _camera->Set_Transform(transform);
    if (!_manualClipPlanes) {
        const float min_clip = std::max(0.2f, _minZoomAdjust * 0.5f);
        _camera->Set_Clip_Planes(min_clip, _cameraDistance * 60.0f);
        setFogNearAndRecalculate(min_clip);
    }

    const int bone_index = object.Get_Bone_Index(kCameraBoneName);
    if (bone_index > 0) {
        Matrix3D bone_transform = object.Get_Bone_Transform(bone_index);
        if (_cameraBonePosX) {
            bone_transform = BuildCameraBoneTransform(bone_transform);
        }
        _camera->Set_Transform(bone_transform);
    }

    if (_sceneLight) {
        if (!_sceneLightOrientationSet && !_sceneLightDistanceSet) {
            Matrix3D light_tm(1);
            light_tm.Set_Translation(_orbitCenter);
            light_tm.Translate(Vector3(0.0f, 0.0f, 0.7f * _cameraDistance));
            _sceneLight->Set_Transform(light_tm);
            _sceneLightDistance = 0.7f * _cameraDistance;
        } else {
            updateSceneLightPosition(old_center);
        }
        syncLightMesh();
    }
    emit objectCameraReset();
}

void W3DViewport::setRenderObject(RenderObjClass *object)
{
    if (_renderObject) {
        removeRenderObjectFromDisplayScene(_renderObject);
    }
    if (_scene) {
        auto *viewer_scene = static_cast<W3DViewScene *>(_scene);
        viewer_scene->Clear_Lineup();
    }
    REF_PTR_RELEASE(_renderObject);

    if (!object) {
        return;
    }

    REF_PTR_SET(_renderObject, object);
    _renderObject->Set_Transform(Matrix3D(1));
    addRenderObjectToDisplayScene(_renderObject);
    if (_scene) {
        if ((_autoResetCamera || _oneTimeCameraReset) && !Is2DPreviewObject(_renderObject)) {
            resetCameraToObject(*_renderObject);
            _oneTimeCameraReset = false;
        }
    }

    if (_renderObject) {
        if (_animationCombo) {
            _renderObject->Set_Animation(_animationCombo);
        } else if (_animation) {
            if (_animationBlend) {
                _renderObject->Set_Animation(_animation, _animationFrame);
            } else {
                _renderObject->Set_Animation(_animation, static_cast<int>(_animationFrame));
            }
        }
    }
}

void W3DViewport::setManualFovEnabled(bool enabled)
{
    _manualFov = enabled;
    if (!_camera) {
        return;
    }

    if (_manualFov) {
        if (_manualHfov > 0.0 && _manualVfov > 0.0) {
            _camera->Set_View_Plane(_manualHfov, _manualVfov);
        }
    } else {
        updateCameraFov(width(), height(), true);
    }
}

bool W3DViewport::isManualFovEnabled() const
{
    return _manualFov;
}

void W3DViewport::setManualClipPlanesEnabled(bool enabled)
{
    _manualClipPlanes = enabled;
    if (_manualClipPlanes && _camera) {
        setCameraClipPlanes(_manualNear, _manualFar);
    }
}

bool W3DViewport::isManualClipPlanesEnabled() const
{
    return _manualClipPlanes;
}

void W3DViewport::setCameraFovDegrees(double hfov_deg, double vfov_deg)
{
    const double hfov = hfov_deg * kDegToRad;
    const double vfov = vfov_deg * kDegToRad;
    _manualHfov = hfov;
    _manualVfov = vfov;
    if (_camera) {
        _camera->Set_View_Plane(hfov, vfov);
    }
}

void W3DViewport::cameraFovDegrees(double &hfov_deg, double &vfov_deg) const
{
    double hfov = _manualHfov;
    double vfov = _manualVfov;
    if (_camera) {
        hfov = _camera->Get_Horizontal_FOV();
        vfov = _camera->Get_Vertical_FOV();
    } else if (hfov <= 0.0 || vfov <= 0.0) {
        hfov = 45.0 * kDegToRad;
        vfov = 45.0 * kDegToRad;
    }

    hfov_deg = hfov * kRadToDeg;
    vfov_deg = vfov * kRadToDeg;
}

void W3DViewport::setCameraClipPlanes(float znear, float zfar)
{
    _manualNear = znear;
    _manualFar = zfar;
    if (_camera) {
        _camera->Set_Clip_Planes(znear, zfar);
    }
    setFogNearAndRecalculate(znear);
}

void W3DViewport::cameraClipPlanes(float &znear, float &zfar) const
{
    if (_camera) {
        _camera->Get_Clip_Planes(znear, zfar);
        return;
    }

    znear = _manualNear;
    zfar = _manualFar;
}

void W3DViewport::setFogNearAndRecalculate(float nearClip)
{
    if (!_scene) {
        return;
    }

    float fogNear = 0.0f;
    float fogFar = 0.0f;
    _scene->Get_Fog_Range(&fogNear, &fogFar);
    _scene->Set_Fog_Range(nearClip, fogFar);
    static_cast<W3DViewScene *>(_scene)->Recalculate_Fog_Planes();
}

void W3DViewport::resetFov()
{
    updateCameraFov(width(), height(), true);
}

void W3DViewport::setCameraDistance(float distance)
{
    if (!_camera) {
        _cameraDistance = distance;
        return;
    }

    _cameraDistance = distance;
    if (_cameraDistance < 0.0f) {
        _cameraDistance = 0.0f;
    }

    Matrix3D transform(1);
    transform.Look_At(_orbitCenter + Vector3(_cameraDistance, 0.0f, 0.0f), _orbitCenter, 0.0f);
    _camera->Set_Transform(transform);
    _rotation = Build_Quaternion(transform);
    _minZoomAdjust = _cameraDistance / 190.0f;
}

float W3DViewport::cameraDistance() const
{
    return _cameraDistance;
}

void W3DViewport::setInitialDisplayMode(int width,
                                        int height,
                                        int bitsPerPixel,
                                        bool fullscreen)
{
    if (_initialized) {
        return;
    }

    _initialDisplayWidth = width > 0 ? width : 0;
    _initialDisplayHeight = height > 0 ? height : 0;
    _bitsPerPixel = bitsPerPixel > 0 ? bitsPerPixel : 32;
    _fullscreen = fullscreen;
    // WW3D's exclusive-fullscreen path assumes its render HWND is a
    // top-level legacy window and rewrites that window's style. The Qt
    // viewport is a native child widget, so Direct3D remains windowed while
    // the QMainWindow owns the borderless fullscreen state.
    _windowed = true;
}

bool W3DViewport::applyResolution(int width, int height, int bitsPerPixel, bool fullscreen)
{
    if (!_initialized || width <= 0 || height <= 0) {
        return false;
    }

    const int bpp = bitsPerPixel > 0 ? bitsPerPixel : _bitsPerPixel;
    const int render_width = fullscreen
        ? width
        : this->width();
    const int render_height = fullscreen
        ? height
        : this->height();
    int actual_width = 0;
    int actual_height = 0;
    int actual_bits_per_pixel = 0;
    if (!trySetDeviceResolution(render_width,
                                render_height,
                                bpp,
                                actual_width,
                                actual_height,
                                actual_bits_per_pixel)) {
        return false;
    }

    _initialDisplayWidth = actual_width;
    _initialDisplayHeight = actual_height;
    _fullscreen = fullscreen;
    _bitsPerPixel = actual_bits_per_pixel;
    _windowed = true;
    updateCameraFov(actual_width, actual_height, true);
    if (_manualClipPlanes) {
        setCameraClipPlanes(_manualNear, _manualFar);
    }

    return true;
}

bool W3DViewport::trySetDeviceResolution(int width,
                                         int height,
                                         int bitsPerPixel,
                                         int &actualWidth,
                                         int &actualHeight,
                                         int &actualBitsPerPixel)
{
    if (!_initialized || width <= 0 || height <= 0 || bitsPerPixel <= 0) {
        return false;
    }

    int previous_width = 0;
    int previous_height = 0;
    int previous_bits_per_pixel = 0;
    bool previous_windowed = true;
    WW3D::Get_Device_Resolution(
        previous_width, previous_height, previous_bits_per_pixel, previous_windowed);

    // The Qt renderer deliberately uses a windowed Direct3D child surface,
    // whose bit depth is fixed to the desktop mode.
    if (!previous_windowed || bitsPerPixel != previous_bits_per_pixel) {
        return false;
    }

    actualWidth = previous_width;
    actualHeight = previous_height;
    actualBitsPerPixel = previous_bits_per_pixel;
    if (width == previous_width && height == previous_height) {
        return true;
    }

    if (WW3D::Set_Device_Resolution(width, height, bitsPerPixel, 1, false) !=
        WW3D_ERROR_OK) {
        return false;
    }

    bool actual_windowed = true;
    WW3D::Get_Device_Resolution(
        actualWidth, actualHeight, actualBitsPerPixel, actual_windowed);
    if (actual_windowed && actualWidth == width && actualHeight == height &&
        actualBitsPerPixel == bitsPerPixel) {
        return true;
    }

    WW3D::Set_Device_Resolution(previous_width,
                                previous_height,
                                previous_bits_per_pixel,
                                previous_windowed ? 1 : 0,
                                false);
    return false;
}

bool W3DViewport::addToLineup(RenderObjClass *object)
{
    if (!_scene || !object) {
        return false;
    }

    auto *viewer_scene = static_cast<W3DViewScene *>(_scene);
    if (!viewer_scene->Can_Line_Up(object)) {
        return false;
    }

    viewer_scene->Add_To_Lineup(object);
    return true;
}

bool W3DViewport::canLineUpClass(int class_id) const
{
    if (!_scene) {
        return false;
    }

    auto *viewer_scene = static_cast<W3DViewScene *>(_scene);
    return viewer_scene->Can_Line_Up(class_id);
}

void W3DViewport::clearLineup()
{
    if (!_scene) {
        return;
    }

    auto *viewer_scene = static_cast<W3DViewScene *>(_scene);
    viewer_scene->Clear_Lineup();
}

void W3DViewport::setObjectRotationFlags(int flags)
{
    _objectRotation = flags;
}

int W3DViewport::objectRotationFlags() const
{
    return _objectRotation;
}

void W3DViewport::resetObjectTransform()
{
    if (!_renderObject) {
        return;
    }

    _renderObject->Set_Transform(Matrix3D(1));
}

void W3DViewport::toggleAlternateMaterials()
{
    ToggleAlternateMaterials(_renderObject);
}

void W3DViewport::setLightRotationFlags(int flags)
{
    _lightRotation = flags;
}

int W3DViewport::lightRotationFlags() const
{
    return _lightRotation;
}

float W3DViewport::currentScreenSize() const
{
    if (!_renderObject || !_camera) {
        return 0.0f;
    }

    return _renderObject->Get_Screen_Size(*_camera);
}

void W3DViewport::setCameraPosition(CameraPosition position)
{
    if (!_camera || !_renderObject) {
        return;
    }

    const SphereClass sphere = _renderObject->Get_Bounding_Sphere();
    const Vector3 old_center = _orbitCenter;
    _orbitCenter = sphere.Center;
    _cameraDistance = sphere.Radius * 3.0f;
    if (_cameraDistance < 1.0f) {
        _cameraDistance = 1.0f;
    }
    if (_cameraDistance > 400.0f) {
        _cameraDistance = 400.0f;
    }

    _minZoomAdjust = _cameraDistance / 190.0f;

    Matrix3D transform(1);
    switch (position) {
    case CameraPosition::Front:
        transform.Look_At(_orbitCenter + Vector3(_cameraDistance, 0.0f, 0.0f), _orbitCenter, 0.0f);
        break;
    case CameraPosition::Back:
        transform.Look_At(_orbitCenter + Vector3(-_cameraDistance, 0.0f, 0.0f), _orbitCenter, 0.0f);
        break;
    case CameraPosition::Left:
        transform.Look_At(_orbitCenter + Vector3(0.0f, -_cameraDistance, 0.0f), _orbitCenter, 0.0f);
        break;
    case CameraPosition::Right:
        transform.Look_At(_orbitCenter + Vector3(0.0f, _cameraDistance, 0.0f), _orbitCenter, 0.0f);
        break;
    case CameraPosition::Top:
        transform.Look_At(_orbitCenter + Vector3(0.0f, 0.0f, _cameraDistance), _orbitCenter, 3.1415926535f);
        break;
    case CameraPosition::Bottom:
        transform.Look_At(_orbitCenter + Vector3(0.0f, 0.0f, -_cameraDistance), _orbitCenter, 3.1415926535f);
        break;
    }

    _camera->Set_Transform(transform);
    _rotation = Build_Quaternion(transform);
    updateSceneLightPosition(old_center);
}

void W3DViewport::resetCamera()
{
    if (_renderObject && !Is2DPreviewObject(_renderObject)) {
        resetCameraToObject(*_renderObject);
    }
}

void W3DViewport::setAllowedCameraRotation(CameraRotation rotation)
{
    _allowedRotation = rotation;
}

W3DViewport::CameraRotation W3DViewport::allowedCameraRotation() const
{
    return _allowedRotation;
}

void W3DViewport::setAutoResetEnabled(bool enabled)
{
    _autoResetCamera = enabled;
}

bool W3DViewport::isAutoResetEnabled() const
{
    return _autoResetCamera;
}

void W3DViewport::requestOneTimeCameraReset()
{
    _oneTimeCameraReset = true;
}

void W3DViewport::setCameraAnimationEnabled(bool enabled)
{
    _animateCamera = enabled;
}

bool W3DViewport::isCameraAnimationEnabled() const
{
    return _animateCamera;
}

void W3DViewport::setCameraBonePosX(bool enabled)
{
    _cameraBonePosX = enabled;
}

bool W3DViewport::isCameraBonePosX() const
{
    return _cameraBonePosX;
}

void W3DViewport::setAnimation(HAnimClass *animation)
{
    if (_animationCombo) {
        delete _animationCombo;
        _animationCombo = nullptr;
    }
    REF_PTR_RELEASE(_animation);
    REF_PTR_SET(_animation, animation);
    _animationTime = 0.0f;
    _animationFrame = 0.0f;
    _animationState = AnimationState::Playing;

    if (_renderObject && _animation) {
        _renderObject->Set_Animation(_animation, 0);
    }
    emit animationStateChanged();
}

void W3DViewport::setAnimationCombo(HAnimComboClass *combo)
{
    if (_animationCombo) {
        delete _animationCombo;
    }

    _animationCombo = combo;
    REF_PTR_RELEASE(_animation);
    _animationTime = 0.0f;
    _animationFrame = 0.0f;
    _animationState = AnimationState::Playing;

    if (_animationCombo) {
        _animation = _animationCombo->Get_Motion(0);
    }

    if (_renderObject && _animationCombo) {
        _renderObject->Set_Animation(_animationCombo);
    }
    emit animationStateChanged();
}

void W3DViewport::clearAnimation()
{
    _animationTime = 0.0f;
    _animationFrame = 0.0f;
    _animationState = AnimationState::Stopped;
    REF_PTR_RELEASE(_animation);
    if (_animationCombo) {
        delete _animationCombo;
        _animationCombo = nullptr;
    }
    if (_renderObject) {
        _renderObject->Set_Animation();
    }
    emit animationStateChanged();
}

bool W3DViewport::animationStatus(int &currentFrame, int &totalFrames, float &fps) const
{
    if (!_animation) {
        currentFrame = 0;
        totalFrames = 0;
        fps = 0.0f;
        return false;
    }

    totalFrames = _animation->Get_Num_Frames();
    currentFrame = static_cast<int>(_animationFrame);
    const float frame_rate = _animation->Get_Frame_Rate();
    fps = frame_rate * _animationSpeed;
    return totalFrames > 0;
}

float W3DViewport::averageFrameMilliseconds() const
{
    return _averageFrameMs;
}

void W3DViewport::setBackgroundColor(const Vector3 &color)
{
    _clearColor = color;
    if (_scene) {
        _scene->Set_Fog_Color(color);
    }
}

Vector3 W3DViewport::backgroundColor() const
{
    return _clearColor;
}

void W3DViewport::setBackgroundBitmap(const QString &path)
{
    _backgroundBitmap = path;
    refreshBackgroundBitmap();
}

QString W3DViewport::backgroundBitmap() const
{
    return _backgroundBitmap;
}

void W3DViewport::setBackgroundObjectName(const QString &name)
{
    _backgroundObjectName = name;

    if (_backgroundObject && _backgroundObjectScene) {
        _backgroundObject->Remove();
    }
    REF_PTR_RELEASE(_backgroundObject);

    if (name.trimmed().isEmpty() || !_backgroundObjectScene) {
        return;
    }

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        return;
    }

    const QByteArray native = name.toLatin1();
    _backgroundObject = asset_manager->Create_Render_Obj(native.constData());
    if (!_backgroundObject) {
        return;
    }

    _backgroundObject->Set_Position(Vector3(0.0f, 0.0f, 0.0f));
    updateBackgroundObjectCamera();

    _backgroundObjectScene->Add_Render_Object(_backgroundObject);
}

QString W3DViewport::backgroundObjectName() const
{
    return _backgroundObjectName;
}

void W3DViewport::setAmbientLight(const Vector3 &color)
{
    _ambientLight = color;
    if (_scene) {
        _scene->Set_Ambient_Light(color);
    }
}

Vector3 W3DViewport::ambientLight() const
{
    return _ambientLight;
}

void W3DViewport::setFogEnabled(bool enabled)
{
    _fogEnabled = enabled;
    if (_scene) {
        _scene->Set_Fog_Enable(enabled);
    }
}

bool W3DViewport::isFogEnabled() const
{
    return _fogEnabled;
}

void W3DViewport::updateBackgroundObjectCamera()
{
    if (!_backgroundObject || !_backgroundObjectCamera || !_camera) {
        return;
    }

    const SphereClass sphere = _backgroundObject->Get_Bounding_Sphere();
    const float radius = std::max(0.01f, sphere.Radius);
    Matrix3D transform = _camera->Get_Transform();
    transform.Set_Translation(sphere.Center + transform.Get_Z_Vector() * (radius * 3.0f));
    _backgroundObjectCamera->Set_Transform(transform);
    _backgroundObjectCamera->Set_Clip_Planes(
        std::max(0.001f, radius * 0.01f), std::max(1.0f, radius * 8.0f));
}

bool W3DViewport::sceneFogRange(float &start, float &end) const
{
    if (!_scene) {
        return false;
    }

    _scene->Get_Fog_Range(&start, &end);
    return true;
}

void W3DViewport::setSceneLightColor(const Vector3 &color)
{
    setSceneLightDiffuse(color);
    setSceneLightSpecular(color);
}

Vector3 W3DViewport::sceneLightColor() const
{
    return sceneLightDiffuse();
}

void W3DViewport::setSceneLightDiffuse(const Vector3 &color)
{
    _sceneLightDiffuse = color;
    if (_sceneLight) {
        _sceneLight->Set_Diffuse(color);
    }
}

Vector3 W3DViewport::sceneLightDiffuse() const
{
    if (_sceneLight) {
        Vector3 color;
        _sceneLight->Get_Diffuse(&color);
        return color;
    }

    return _sceneLightDiffuse;
}

void W3DViewport::setSceneLightSpecular(const Vector3 &color)
{
    _sceneLightSpecular = color;
    if (_sceneLight) {
        _sceneLight->Set_Specular(color);
    }
}

Vector3 W3DViewport::sceneLightSpecular() const
{
    if (_sceneLight) {
        Vector3 color;
        _sceneLight->Get_Specular(&color);
        return color;
    }

    return _sceneLightSpecular;
}

void W3DViewport::setSceneLightOrientation(const Quaternion &orientation)
{
    _sceneLightOrientation = orientation;
    _sceneLightOrientationSet = true;
    if (!_sceneLight) {
        return;
    }

    float distance = sceneLightDistance();
    if (distance <= 0.0f) {
        distance = std::max(1.0f, _cameraDistance);
    }

    Matrix3D light_tm(1);
    light_tm.Set_Translation(_orbitCenter);
    Matrix3D::Multiply(light_tm, Build_Matrix3D(orientation), &light_tm);
    light_tm.Translate(Vector3(0.0f, 0.0f, distance));
    _sceneLight->Set_Transform(light_tm);
    syncLightMesh();
}

Quaternion W3DViewport::sceneLightOrientation() const
{
    if (_sceneLight) {
        return Build_Quaternion(_sceneLight->Get_Transform());
    }

    return _sceneLightOrientation;
}

void W3DViewport::setSceneLightDistance(float distance)
{
    _sceneLightDistance = distance;
    _sceneLightDistanceSet = true;
    if (!_sceneLight) {
        return;
    }

    Vector3 direction = _sceneLight->Get_Position() - _orbitCenter;
    float length = direction.Length();
    if (length <= 0.0f) {
        direction = Vector3(0.0f, 0.0f, 1.0f);
    } else {
        direction.Normalize();
    }

    _sceneLight->Set_Position(_orbitCenter + direction * distance);
    syncLightMesh();
}

float W3DViewport::sceneLightDistance() const
{
    if (_sceneLight) {
        return (_sceneLight->Get_Position() - _orbitCenter).Length();
    }

    return _sceneLightDistance;
}

void W3DViewport::setSceneLightIntensity(float intensity)
{
    _sceneLightIntensity = intensity;
    if (_sceneLight) {
        _sceneLight->Set_Intensity(intensity);
    }
}

float W3DViewport::sceneLightIntensity() const
{
    if (_sceneLight) {
        return _sceneLight->Get_Intensity();
    }

    return _sceneLightIntensity;
}

void W3DViewport::setSceneLightAttenuation(float start, float end, bool enabled)
{
    _sceneLightAttenStart = start;
    _sceneLightAttenEnd = end;
    _sceneLightAttenEnabled = enabled;
    if (_sceneLight) {
        _sceneLight->Set_Far_Attenuation_Range(start, end);
        _sceneLight->Set_Flag(LightClass::FAR_ATTENUATION, enabled);
    }
}

void W3DViewport::sceneLightAttenuation(float &start, float &end, bool &enabled) const
{
    if (_sceneLight) {
        _sceneLight->Get_Far_Attenuation_Range(start, end);
        enabled = _sceneLight->Get_Flag(LightClass::FAR_ATTENUATION) != 0;
        return;
    }

    start = _sceneLightAttenStart;
    end = _sceneLightAttenEnd;
    enabled = _sceneLightAttenEnabled;
}

W3DViewport::SceneLightState W3DViewport::sceneLightState() const
{
    SceneLightState state;
    state.diffuse = sceneLightDiffuse();
    state.specular = sceneLightSpecular();
    state.orientation = sceneLightOrientation();
    state.distance = sceneLightDistance();
    state.intensity = sceneLightIntensity();
    sceneLightAttenuation(
        state.attenuationStart, state.attenuationEnd, state.attenuationEnabled);
    state.orientationExplicit = _sceneLightOrientationSet;
    state.distanceExplicit = _sceneLightDistanceSet;
    return state;
}

void W3DViewport::setSceneLightState(const SceneLightState &state)
{
    setSceneLightDiffuse(state.diffuse);
    setSceneLightSpecular(state.specular);
    setSceneLightIntensity(state.intensity);
    setSceneLightAttenuation(
        state.attenuationStart, state.attenuationEnd, state.attenuationEnabled);

    if (state.orientationExplicit) {
        setSceneLightOrientation(state.orientation);
    }
    setSceneLightDistance(state.distance);

    _sceneLightOrientation = state.orientation;
    _sceneLightDistance = state.distance;
    _sceneLightOrientationSet = state.orientationExplicit;
    _sceneLightDistanceSet = state.distanceExplicit;
}

void W3DViewport::setWireframeEnabled(bool enabled)
{
    _wireframeEnabled = enabled;
    if (_scene) {
        _scene->Set_Polygon_Mode(enabled ? SceneClass::LINE : SceneClass::FILL);
    }
}

bool W3DViewport::isWireframeEnabled() const
{
    return _wireframeEnabled;
}

void W3DViewport::setLodAutoSwitchingEnabled(bool enabled)
{
    _allowLodSwitching = enabled;
    auto *scene = static_cast<W3DViewScene *>(_scene);
    if (scene) {
        scene->SetAllowLodSwitching(enabled);
    }
}

bool W3DViewport::isLodAutoSwitchingEnabled() const
{
    if (auto *scene = static_cast<W3DViewScene *>(_scene)) {
        return scene->IsAllowLodSwitching();
    }

    return _allowLodSwitching;
}

bool W3DViewport::currentLodInfo(int &level, int &count) const
{
    if (!_renderObject || _renderObject->Class_ID() != RenderObjClass::CLASSID_HLOD) {
        return false;
    }

    auto *hlod = static_cast<HLodClass *>(_renderObject);
    level = hlod->Get_LOD_Level();
    count = hlod->Get_LOD_Count();
    return true;
}

bool W3DViewport::setNullLodIncluded(bool enabled)
{
    if (!_renderObject || _renderObject->Class_ID() != RenderObjClass::CLASSID_HLOD) {
        return false;
    }

    auto *hlod = static_cast<HLodClass *>(_renderObject);
    hlod->Include_NULL_Lod(enabled);
    UpdateLodPrototype(*hlod);
    return true;
}

bool W3DViewport::isNullLodIncluded() const
{
    if (!_renderObject || _renderObject->Class_ID() != RenderObjClass::CLASSID_HLOD) {
        return false;
    }

    auto *hlod = static_cast<HLodClass *>(_renderObject);
    return hlod->Is_NULL_Lod_Included();
}

bool W3DViewport::recordLodScreenArea()
{
    if (!_renderObject || !_camera ||
        _renderObject->Class_ID() != RenderObjClass::CLASSID_HLOD) {
        return false;
    }

    auto *hlod = static_cast<HLodClass *>(_renderObject);
    const float screen_size = _renderObject->Get_Screen_Size(*_camera);
    hlod->Set_Max_Screen_Size(hlod->Get_LOD_Level(), screen_size);
    UpdateLodPrototype(*hlod);
    return true;
}

bool W3DViewport::adjustLodLevel(int delta)
{
    bool switched = false;
    SwitchLod(_renderObject, delta, switched);
    return switched;
}

void W3DViewport::applySceneLightSettings()
{
    if (!_sceneLight) {
        return;
    }

    setSceneLightDiffuse(_sceneLightDiffuse);
    setSceneLightSpecular(_sceneLightSpecular);
    setSceneLightIntensity(_sceneLightIntensity);
    setSceneLightAttenuation(_sceneLightAttenStart, _sceneLightAttenEnd, _sceneLightAttenEnabled);

    if (_sceneLightOrientationSet) {
        setSceneLightOrientation(_sceneLightOrientation);
    }

    if (_sceneLightDistanceSet) {
        setSceneLightDistance(_sceneLightDistance);
    }
}

void W3DViewport::updateSceneLightPosition(const Vector3 &oldCenter)
{
    if (!_sceneLight) {
        return;
    }

    Vector3 direction = _sceneLight->Get_Position() - oldCenter;
    float distance = direction.Length();
    if (distance <= 0.0f) {
        direction = Vector3(0.0f, 0.0f, 1.0f);
        distance = std::max(1.0f, _cameraDistance);
    } else {
        direction.Normalize();
    }

    _sceneLight->Set_Position(_orbitCenter + direction * distance);
    _sceneLightDistance = distance;
    syncLightMesh();
}

void W3DViewport::syncLightMesh()
{
    if (!_lightMesh || !_sceneLight) {
        return;
    }

    _lightMesh->Set_Transform(_sceneLight->Get_Transform());

    const float view_distance = std::max(1.0f, _cameraDistance);
    const float desired_scale = view_distance / 14.0f;
    if (_lightMeshScale > 0.0f && desired_scale != _lightMeshScale) {
        _lightMesh->Scale(desired_scale / _lightMeshScale);
        _lightMeshScale = desired_scale;
    }
}

void W3DViewport::setLightMeshVisible(bool visible)
{
    if (!_scene || !_lightMesh || visible == _lightMeshInScene) {
        return;
    }

    if (visible) {
        _scene->Add_Render_Object(_lightMesh);
    } else {
        _scene->Remove_Render_Object(_lightMesh);
    }
    _lightMeshInScene = visible;
}

void W3DViewport::updateInteractionCursor()
{
    if (_leftDown && _rightDown) {
        setCursor(Qt::ClosedHandCursor);
    } else if (_leftDown) {
        setCursor(Qt::OpenHandCursor);
    } else if (_rightDown) {
        setCursor(Qt::SizeVerCursor);
    } else {
        unsetCursor();
    }
}

void W3DViewport::refreshBackgroundBitmap()
{
    if (!_backgroundScene) {
        return;
    }

    if (_backgroundBitmapObj) {
        _backgroundBitmapObj->Remove();
        _backgroundBitmapObj->Release_Ref();
        _backgroundBitmapObj = nullptr;
    }

    if (_backgroundBitmap.trimmed().isEmpty()) {
        return;
    }

    const QByteArray native = QDir::toNativeSeparators(_backgroundBitmap).toLocal8Bit();
    _backgroundBitmapObj = NEW_REF(Bitmap2DObjClass, (native.constData(), 0.5f, 0.5f, true, false));
    if (_backgroundBitmapObj) {
        _backgroundScene->Add_Render_Object(_backgroundBitmapObj);
    }
}

void W3DViewport::updateAnimation(float deltaSeconds)
{
    if (!_animation || !_renderObject) {
        return;
    }

    if (_animationState != AnimationState::Playing) {
        return;
    }

    const int total_frames = _animation->Get_Num_Frames();
    const float frame_rate = _animation->Get_Frame_Rate();
    if (total_frames <= 1 || frame_rate <= 0.0f) {
        _renderObject->Set_Animation(_animation, 0);
        return;
    }

    const float loop_time = static_cast<float>(total_frames - 1) / frame_rate;
    _animationTime += deltaSeconds * _animationSpeed;
    if (_animationTime > loop_time) {
        _animationTime = std::fmod(_animationTime, loop_time);
    }

    _animationFrame = frame_rate * _animationTime;
    if (_animationCombo) {
        const int count = _animationCombo->Get_Num_Anims();
        for (int index = 0; index < count; ++index) {
            _animationCombo->Set_Frame(index, _animationFrame);
        }
        _renderObject->Set_Animation(_animationCombo);
    } else {
        if (_animationBlend) {
            _renderObject->Set_Animation(_animation, _animationFrame);
        } else {
            _renderObject->Set_Animation(_animation, static_cast<int>(_animationFrame));
        }
    }
}

void W3DViewport::setAnimationState(AnimationState state)
{
    if (_animationState == state) {
        return;
    }

    _animationState = state;
    emit animationStateChanged();

    if (!_renderObject || !_animation) {
        return;
    }

    if (state == AnimationState::Stopped) {
        _animationTime = 0.0f;
        _animationFrame = 0.0f;
        if (_animationCombo) {
            const int count = _animationCombo->Get_Num_Anims();
            for (int index = 0; index < count; ++index) {
                _animationCombo->Set_Frame(index, 0.0f);
            }
            _renderObject->Set_Animation(_animationCombo);
        } else {
            _renderObject->Set_Animation(_animation, 0);
        }
    }
}

W3DViewport::AnimationState W3DViewport::animationState() const
{
    return _animationState;
}

void W3DViewport::setAnimationSpeed(float speed)
{
    if (speed <= 0.0f) {
        speed = 0.01f;
    }
    _animationSpeed = speed;
}

float W3DViewport::animationSpeed() const
{
    return _animationSpeed;
}

void W3DViewport::setAnimationBlend(bool enabled)
{
    if (_animationBlend == enabled) {
        return;
    }

    _animationBlend = enabled;
    if (!_renderObject || !_animation || _animationCombo) {
        return;
    }

    if (_animationBlend) {
        _renderObject->Set_Animation(_animation, _animationFrame);
    } else {
        _renderObject->Set_Animation(_animation, static_cast<int>(_animationFrame));
    }
}

bool W3DViewport::animationBlend() const
{
    return _animationBlend;
}

bool W3DViewport::stepAnimation(int delta)
{
    if (!_animation || !_renderObject) {
        return false;
    }

    const int total_frames = _animation->Get_Num_Frames();
    if (total_frames <= 1) {
        if (_animationCombo) {
            const int count = _animationCombo->Get_Num_Anims();
            for (int index = 0; index < count; ++index) {
                _animationCombo->Set_Frame(index, 0.0f);
            }
            _renderObject->Set_Animation(_animationCombo);
        } else {
            _renderObject->Set_Animation(_animation, 0);
        }
        _animationFrame = 0.0f;
        _animationTime = 0.0f;
        return true;
    }

    int frame = static_cast<int>(_animationFrame) + delta;
    if (frame >= total_frames) {
        frame = 0;
    } else if (frame < 0) {
        frame = total_frames - 1;
    }

    _animationFrame = static_cast<float>(frame);
    const float frame_rate = _animation->Get_Frame_Rate();
    if (frame_rate > 0.0f) {
        _animationTime = _animationFrame / frame_rate;
    } else {
        _animationTime = 0.0f;
    }
    if (_animationCombo) {
        const int count = _animationCombo->Get_Num_Anims();
        for (int index = 0; index < count; ++index) {
            _animationCombo->Set_Frame(index, _animationFrame);
        }
        _renderObject->Set_Animation(_animationCombo);
    } else {
        _renderObject->Set_Animation(_animation, frame);
    }
    return true;
}

void W3DViewport::applyAnimationFrame(float frame)
{
    _animationFrame = frame;
    if (!_animation || !_renderObject) {
        return;
    }

    const float frame_rate = _animation->Get_Frame_Rate();
    if (frame_rate > 0.0f) {
        _animationTime = _animationFrame / frame_rate;
    } else {
        _animationTime = 0.0f;
    }

    if (_animationCombo) {
        const int count = _animationCombo->Get_Num_Anims();
        for (int index = 0; index < count; ++index) {
            _animationCombo->Set_Frame(index, _animationFrame);
        }
        _renderObject->Set_Animation(_animationCombo);
    } else {
        _renderObject->Set_Animation(_animation, _animationFrame);
    }
}

bool W3DViewport::hasAnimation() const
{
    return _animation != nullptr;
}

QString W3DViewport::currentAnimationName() const
{
    return _animation && _animation->Get_Name()
        ? QString::fromLatin1(_animation->Get_Name())
        : QString();
}

bool W3DViewport::captureMovie(const QString &baseName, float frameRate, QString *error)
{
    if (error) {
        error->clear();
    }

    if (!_renderObject || !_animation) {
        if (error) {
            *error = "No animation is available for capture.";
        }
        return false;
    }

    if (frameRate <= 0.0f) {
        frameRate = 30.0f;
    }

    const int total_frames = _animation->Get_Num_Frames();
    const float anim_rate = _animation->Get_Frame_Rate();
    if (total_frames <= 1 || anim_rate <= 0.0f) {
        if (error) {
            *error = "Animation has no frames to capture.";
        }
        return false;
    }

    const bool timer_active = _timer.isActive();
    if (timer_active) {
        _timer.stop();
    }

    const AnimationState prev_state = _animationState;
    const float prev_frame = _animationFrame;

    setAnimationState(AnimationState::Paused);
    applyAnimationFrame(0.0f);

    const QByteArray base_bytes = baseName.trimmed().isEmpty()
        ? QByteArray("Grab")
        : baseName.toLatin1();

    WW3D::Pause_Movie(true);
    bool capture_ok = WW3D::Try_Start_Movie_Capture(base_bytes.constData(), frameRate);
    if (capture_ok) {
        WW3D::Pause_Movie(true);
    } else if (error) {
        *error = "Unable to create the AVI movie capture file.";
    }

    const float frame_inc = anim_rate / frameRate;
    const int ticks = static_cast<int>(1000.0f / frameRate);

    for (float frame = 0.0f;
         capture_ok && frame <= (static_cast<float>(total_frames) - 1.0f);
         frame += frame_inc) {
        applyAnimationFrame(frame);
        // Capture the unpresented render-device back buffer so movie output is
        // independent of desktop occlusion and the viewport's screen position.
        renderFrameWithTicks(ticks, false);
        if (!WW3D::Try_Update_Movie_Capture_From_Back_Buffer()) {
            capture_ok = false;
            if (error) {
                *error = "Unable to capture or write an AVI movie frame.";
            }
            break;
        }
        renderScene(true);
#ifdef _WIN32
        if (::GetAsyncKeyState(VK_ESCAPE) < 0) {
            break;
        }
#endif
    }

    WW3D::Stop_Movie_Capture();

    if (prev_state == AnimationState::Stopped) {
        setAnimationState(AnimationState::Stopped);
    } else {
        applyAnimationFrame(prev_frame);
        _animationState = prev_state;
        emit animationStateChanged();
    }

    if (timer_active) {
        _elapsed.restart();
        _timer.start();
    }

    return capture_ok;
}

bool W3DViewport::toggleSubobjectLod()
{
    if (!_renderObject) {
        return false;
    }

    const bool enabled = _renderObject->Is_Sub_Objects_Match_LOD_Enabled() != 0;
    _renderObject->Set_Sub_Objects_Match_LOD(!enabled);
    UpdateAggregatePrototype(*_renderObject);
    return !enabled;
}

bool W3DViewport::isSubobjectLodBound() const
{
    if (!_renderObject) {
        return false;
    }

    return _renderObject->Is_Sub_Objects_Match_LOD_Enabled() != 0;
}

void W3DViewport::updateCameraAnimation()
{
    if (!_animateCamera || !_renderObject || !_camera) {
        return;
    }

    Matrix3D bone_transform(1);
    if (!GetCameraTransform(_renderObject, bone_transform)) {
        return;
    }

    const Matrix3D camera_transform = BuildCameraBoneTransform(bone_transform);
    _camera->Set_Transform(camera_transform);
}

void W3DViewport::updateObjectRotation()
{
    if (!_renderObject || _objectRotation == RotateNone) {
        return;
    }

    Matrix3D transform = _renderObject->Get_Transform();

    if (_objectRotation & RotateX) {
        transform.Rotate_X(0.05f);
    } else if (_objectRotation & RotateXBack) {
        transform.Rotate_X(-0.05f);
    }
    if (_objectRotation & RotateY) {
        transform.Rotate_Y(-0.05f);
    } else if (_objectRotation & RotateYBack) {
        transform.Rotate_Y(0.05f);
    }
    if (_objectRotation & RotateZ) {
        transform.Rotate_Z(0.05f);
    } else if (_objectRotation & RotateZBack) {
        transform.Rotate_Z(-0.05f);
    }

    if (!transform.Is_Orthogonal()) {
        transform.Re_Orthogonalize();
    }

    _renderObject->Set_Transform(transform);
}

void W3DViewport::updateLightRotation()
{
    if (!_sceneLight || !_renderObject || _lightRotation == RotateNone) {
        return;
    }

    Matrix3D rotation_matrix(1);
    if (_lightRotation & RotateX) {
        rotation_matrix.Rotate_X(0.05f);
    } else if (_lightRotation & RotateXBack) {
        rotation_matrix.Rotate_X(-0.05f);
    }
    if (_lightRotation & RotateY) {
        rotation_matrix.Rotate_Y(-0.05f);
    } else if (_lightRotation & RotateYBack) {
        rotation_matrix.Rotate_Y(0.05f);
    }
    if (_lightRotation & RotateZ) {
        rotation_matrix.Rotate_Z(0.05f);
    } else if (_lightRotation & RotateZBack) {
        rotation_matrix.Rotate_Z(-0.05f);
    }

    Matrix3D coord_inv;
    Matrix3D coord_to_obj;
    Matrix3D coord_system = _renderObject->Get_Transform();
    coord_system.Get_Orthogonal_Inverse(coord_inv);

    Matrix3D transform = _sceneLight->Get_Transform();
    Matrix3D::Multiply(coord_inv, transform, &coord_to_obj);
    Matrix3D::Multiply(coord_system, rotation_matrix, &transform);
    Matrix3D::Multiply(transform, coord_to_obj, &transform);

    if (!transform.Is_Orthogonal()) {
        transform.Re_Orthogonalize();
    }

    _sceneLight->Set_Transform(transform);
    _sceneLightOrientation = Build_Quaternion(transform);
    _sceneLightOrientationSet = true;
    _sceneLightDistance = sceneLightDistance();
    _sceneLightDistanceSet = true;
    syncLightMesh();
}
