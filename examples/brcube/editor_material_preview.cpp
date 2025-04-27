#include "editor_material_preview.hpp"

Editor::MaterialPreview::MaterialPreview(br_pixelmap *screen, br_registry *reg) noexcept
{
    br_actor  *root, *sphere, *light;
    br_camera *camdata;
    br_light  *lightdata;

    root         = BrActorAllocate(BR_ACTOR_CAMERA, nullptr);
    root->t.type = BR_TRANSFORM_IDENTITY;

    camdata                = reinterpret_cast<br_camera *>(root->type_data);
    camdata->type          = BR_CAMERA_PERSPECTIVE;
    camdata->field_of_view = BR_ANGLE_DEG(60);
    camdata->hither_z      = BR_SCALAR(0.1);
    camdata->yon_z         = BR_SCALAR(1024.0f);
    camdata->aspect        = BR_SCALAR(1.0f);

    sphere         = BrActorAdd(root, BrActorAllocate(BR_ACTOR_MODEL, nullptr));
    sphere->model  = static_cast<br_model*>(BrRegistryFind(reg, "breditor_sph32"));
    sphere->t.type = BR_TRANSFORM_MATRIX34;

    sphere->material         = BrMaterialAllocate("XX");
    sphere->material->colour = BR_COLOUR_RGBA(255, 0, 0, 0);
    BrMaterialUpdate(sphere->material, BR_MATU_ALL);

    /*
     * Move the sphere in front of the camera.
     */
    BrMatrix34Translate(&sphere->t.t.mat, BR_SCALAR(0), BR_SCALAR(0), BR_SCALAR(-2.5));

    light                    = BrActorAdd(root, BrActorAllocate(BR_ACTOR_LIGHT, nullptr));
    lightdata                = reinterpret_cast<br_light *>(light->type_data);
    lightdata->type          = BR_LIGHT_DIRECT;
    lightdata->attenuation_c = 1;

    BrMatrix34PostRotateY(&light->t.t.mat, BR_ANGLE_DEG(30));
    BrMatrix34PostRotateX(&light->t.t.mat, BR_ANGLE_DEG(30));

    this->m_root.reset(root);
    this->m_sphere = sphere;
    this->m_light  = light;
    this->m_colour_buffer.reset(BrPixelmapMatchTypedSized(screen, BR_PMMATCH_OFFSCREEN, BR_PMT_RGBA_8888, 128, 128));
    this->m_depth_buffer.reset(BrPixelmapMatch(this->m_colour_buffer.get(), BR_PMMATCH_DEPTH));

    this->m_colour_buffer->origin_x = static_cast<br_int_16>(this->m_colour_buffer->width >> 1);
    this->m_colour_buffer->origin_y = static_cast<br_int_16>(this->m_colour_buffer->height >> 1);
}

void Editor::MaterialPreview::Draw(br_pixelmap *rt, br_material *mat) noexcept
{
    if(rt == nullptr)
        return;

    this->m_sphere->material = mat;

    BrLightEnable(this->m_light);

    BrRendererFrameBegin();
    BrPixelmapFill(this->m_colour_buffer.get(), BR_COLOUR_RGBA(0, 0, 0, 0));
    BrPixelmapFill(this->m_depth_buffer.get(), 0xFFFFFFFF);
    BrZbSceneRender(this->m_root.get(), this->m_root.get(), this->m_colour_buffer.get(), this->m_depth_buffer.get());
    BrRendererFrameEnd();

    BrLightDisable(this->m_light);

    BrPixelmapCopy(rt, this->m_colour_buffer.get());
}
