#include "MeshPropertiesDialog.h"

#include "ui_MeshPropertiesDialog.h"

#include "assetmgr.h"
#include "mesh.h"
#include "meshmdl.h"
#include "rendobj.h"
#include "w3d_file.h"

#include <QDialogButtonBox>

MeshPropertiesDialog::MeshPropertiesDialog(const QString &meshName, QWidget *parent)
    : QDialog(parent)
    , _ui(new Ui::MeshPropertiesDialog)
{
    _ui->setupUi(this);
    connect(_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    if (meshName.isEmpty()) {
        setErrorState("No mesh selected.");
        return;
    }

    _ui->descriptionLabel->setText(QString("Mesh: %1").arg(meshName));

    auto *asset_manager = WW3DAssetManager::Get_Instance();
    if (!asset_manager) {
        setErrorState("WW3D asset manager is not available.");
        return;
    }

    const QByteArray name_bytes = meshName.toLatin1();
    RenderObjClass *render_obj = asset_manager->Create_Render_Obj(name_bytes.constData());
    if (!render_obj) {
        setErrorState("Failed to load mesh.");
        return;
    }

    _ui->polygonCountValue->setText(QString::number(render_obj->Get_Num_Polys()));

    if (render_obj->Class_ID() != RenderObjClass::CLASSID_MESH) {
        setErrorState("Selected object is not a mesh.");
        render_obj->Release_Ref();
        return;
    }

    auto *mesh = static_cast<MeshClass *>(render_obj);
    MeshModelClass *model = mesh->Get_Model();
    if (model) {
        _ui->vertexCountValue->setText(QString::number(model->Get_Vertex_Count()));
    }

    const char *user_text = mesh->Get_User_Text();
    if (user_text) {
        _ui->userTextValue->setText(QString::fromLatin1(user_text));
    } else {
        _ui->userTextValue->setText("");
    }

    const uint32 flags = mesh->Get_W3D_Flags();

    if ((flags & W3D_MESH_FLAG_COLLISION_BOX) == W3D_MESH_FLAG_COLLISION_BOX) {
        _ui->meshTypeCollision->setChecked(true);
    } else if ((flags & W3D_MESH_FLAG_SKIN) == W3D_MESH_FLAG_SKIN) {
        _ui->meshTypeSkin->setChecked(true);
    } else if ((flags & W3D_MESH_FLAG_SHADOW) == W3D_MESH_FLAG_SHADOW) {
        _ui->meshTypeShadow->setChecked(true);
    } else {
        _ui->meshTypeNormal->setChecked(true);
    }

    const uint32 collision_flags = flags & W3D_MESH_FLAG_COLLISION_TYPE_MASK;
    if ((collision_flags & W3D_MESH_FLAG_COLLISION_TYPE_PHYSICAL) == W3D_MESH_FLAG_COLLISION_TYPE_PHYSICAL) {
        _ui->collisionPhysical->setChecked(true);
    }
    if ((collision_flags & W3D_MESH_FLAG_COLLISION_TYPE_PROJECTILE) ==
        W3D_MESH_FLAG_COLLISION_TYPE_PROJECTILE) {
        _ui->collisionProjectile->setChecked(true);
    }

    if ((flags & W3D_MESH_FLAG_HIDDEN) == W3D_MESH_FLAG_HIDDEN) {
        _ui->hiddenCheck->setChecked(true);
    }

    render_obj->Release_Ref();
}

MeshPropertiesDialog::~MeshPropertiesDialog()
{
    delete _ui;
}

void MeshPropertiesDialog::setErrorState(const QString &message)
{
    _ui->descriptionLabel->setText(message);
    _ui->polygonCountValue->setText("n/a");
    _ui->vertexCountValue->setText("n/a");
    _ui->userTextValue->setText("");
    _ui->meshTypeNormal->setChecked(false);
    _ui->meshTypeCollision->setChecked(false);
    _ui->meshTypeSkin->setChecked(false);
    _ui->meshTypeShadow->setChecked(false);
    _ui->collisionPhysical->setChecked(false);
    _ui->collisionProjectile->setChecked(false);
    _ui->hiddenCheck->setChecked(false);
}
