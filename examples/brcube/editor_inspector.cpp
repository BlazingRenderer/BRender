#include "editor_actor.hpp"
#include "editor_inspector.hpp"

#include "ImGuizmo/ImGuizmo.h"

template <typename T>
static void DrawIdentifier(T *e, const char *label)
{
    char idbuf[256];

    const char *identifier = "";
    if(e->identifier != nullptr)
        identifier = e->identifier;

    strncpy(idbuf, identifier, sizeof(idbuf));
    idbuf[BR_ASIZE(idbuf) - 1] = '\0';

    if(ImGui::InputText(label, idbuf, sizeof(idbuf))) {
        if(e->identifier != nullptr)
            BrResFree(e->identifier);

        e->identifier = BrResStrDup(e, idbuf);
    }
}

static void DrawTransformInspector(br_transform *tf)
{
    ImGui::DragFloat3("Position", tf->t.translate.t.v, 0.01f);

    {
        static const char *xx[] = {
            /* [BR_TRANSFORM_MATRIX34]    = */ "Matrix 3x4",
            /* [BR_TRANSFORM_MATRIX34_LP] = */ "Matrix 3x4 LP",
            /* [BR_TRANSFORM_QUAT]        = */ "Quaternion",
            /* [BR_TRANSFORM_EULER]       = */ "Euler",
            /* [BR_TRANSFORM_LOOK_UP]     = */ "Look Up",
            /* [BR_TRANSFORM_TRANSLATION] = */ "Translation",
            /* [BR_TRANSFORM_IDENTITY]    = */ "Identity",
        };
        int type = tf->type;
        if(ImGui::Combo("Type", &type, xx, BR_TRANSFORM_MAX)) {
            if(type < 0)
                type = BR_TRANSFORM_MATRIX34;

            if(type >= BR_TRANSFORM_MAX)
                type = BR_TRANSFORM_IDENTITY;

            tf->type = (br_uint_16)type;
        }
    }

    switch(tf->type) {
        case BR_TRANSFORM_MATRIX34:
            ImGui::DragFloat3("##MATRIX34_R0", tf->t.mat.m[0], 0.01f);
            ImGui::DragFloat3("##MATRIX34_R1", tf->t.mat.m[1], 0.01f);
            ImGui::DragFloat3("##MATRIX34_R2", tf->t.mat.m[2], 0.01f);
            ImGui::DragFloat3("##MATRIX34_R3", tf->t.mat.m[3], 0.01f);
            break;

        case BR_TRANSFORM_MATRIX34_LP:
            ImGui::DragFloat3("##MATRIX34_LP_R0", tf->t.mat.m[0], 0.01f);
            ImGui::DragFloat3("##MATRIX34_LP_R1", tf->t.mat.m[1], 0.01f);
            ImGui::DragFloat3("##MATRIX34_LP_R2", tf->t.mat.m[2], 0.01f);
            ImGui::DragFloat3("##MATRIX34_LP_R3", tf->t.mat.m[3], 0.01f);
            break;

        case BR_TRANSFORM_QUAT: {
            br_quat& q    = tf->t.quat.q;
            float    v[4] = {q.x, q.y, q.z, q.w};
            if(ImGui::DragFloat4("Rotation##QUAT", v)) {
                q.x = v[0];
                q.y = v[1];
                q.z = v[2];
                q.w = v[3];
            }
            break;
        }

        case BR_TRANSFORM_EULER: {
            br_euler&          e        = tf->t.euler.e;
            static const char *orders[] = {
                /* [BR_EULER_XYZ_S] = */ "XYZ Static",
                /* [BR_EULER_XYX_S] = */ "XYX Static",
                /* [BR_EULER_XZY_S] = */ "XZY Static",
                /* [BR_EULER_XZX_S] = */ "XZX Static",
                /* [BR_EULER_YZX_S] = */ "YZX Static",
                /* [BR_EULER_YZY_S] = */ "YZY Static",
                /* [BR_EULER_YXZ_S] = */ "YXZ Static",
                /* [BR_EULER_YXY_S] = */ "YXY Static",
                /* [BR_EULER_ZXY_S] = */ "ZXY Static",
                /* [BR_EULER_ZXZ_S] = */ "ZXZ Static",
                /* [BR_EULER_ZYX_S] = */ "ZYX Static",
                /* [BR_EULER_ZYZ_S] = */ "ZYZ Static",
                /* [BR_EULER_ZYX_R] = */ "ZYX Rotating",
                /* [BR_EULER_XYX_R] = */ "XYX Rotating",
                /* [BR_EULER_YZX_R] = */ "YZX Rotating",
                /* [BR_EULER_XZX_R] = */ "XZX Rotating",
                /* [BR_EULER_XZY_R] = */ "XZY Rotating",
                /* [BR_EULER_YZY_R] = */ "YZY Rotating",
                /* [BR_EULER_ZXY_R] = */ "ZXY Rotating",
                /* [BR_EULER_YXY_R] = */ "YXY Rotating",
                /* [BR_EULER_YXZ_R] = */ "YXZ Rotating",
                /* [BR_EULER_ZXZ_R] = */ "ZXZ Rotating",
                /* [BR_EULER_XYZ_R] = */ "XYZ Rotating",
                /* [BR_EULER_ZYZ_R] = */ "ZYZ Rotating",
            };

            int order = e.order;
            if(ImGui::Combo("Order##EULER", &order, orders, BR_ASIZE(orders))) {
                e.order = (br_uint_8)order;
            }

            float v[3] = {
                BrScalarToFloat(BrAngleToScalar(e.a)) * 360.0f,
                BrScalarToFloat(BrAngleToScalar(e.b)) * 360.0f,
                BrScalarToFloat(BrAngleToScalar(e.c)) * 360.0f,
            };

            if(ImGui::DragFloat3("Rotation##EULER", v, 0.01f)) {
                e.a = BrScalarToAngle(BrFloatToScalar(fmodf(v[0] / 360.0f, 1.0f)));
                e.b = BrScalarToAngle(BrFloatToScalar(fmodf(v[1] / 360.0f, 1.0f)));
                e.c = BrScalarToAngle(BrFloatToScalar(fmodf(v[2] / 360.0f, 1.0f)));
            }

            break;
        }
        case BR_TRANSFORM_LOOK_UP:
            ImGui::DragFloat3("Look##LOOK_UP", tf->t.look_up.look.v);
            ImGui::DragFloat3("Up##LOOK_UP", tf->t.look_up.up.v);
            break;
        case BR_TRANSFORM_IDENTITY:
            break;
    }
}

static void DrawLightInspector(br_light *light)
{
    DrawIdentifier(light, "Identifier");

    static const char *types[] = {
        [BR_LIGHT_POINT]   = "Point",
        [BR_LIGHT_DIRECT]  = "Directional",
        [BR_LIGHT_SPOT]    = "Spot",
        [BR_LIGHT_AMBIENT] = "Ambient",
    };

    bool view_space     = !!(light->type & BR_LIGHT_VIEW);
    bool linear_falloff = !!(light->type & BR_LIGHT_LINEAR_FALLOFF);
    int  type           = light->type & BR_LIGHT_TYPE;

    ImGui::Combo("Type", &type, types, BR_ASIZE(types));
    ImGui::Checkbox("View Space", &view_space);
    ImGui::SameLine();
    ImGui::Checkbox("Linear Falloff", &linear_falloff);

    light->type = type | (view_space ? BR_LIGHT_VIEW : 0) | (linear_falloff ? BR_LIGHT_LINEAR_FALLOFF : 0);

    {
        float col[3] = {BR_RED(light->colour) / 255.0f, BR_GRN(light->colour) / 255.0f, BR_BLU(light->colour) / 255.0f};
        ImGui::ColorEdit3("Colour", col);

        light->colour = BR_COLOUR_RGB(static_cast<uint8_t>(col[0] * 255.0f), static_cast<uint8_t>(col[1] * 255.0f),
                                      static_cast<uint8_t>(col[2] * 255.0f));
    }

    {
        float atten[3] = {light->attenuation_c, light->attenuation_l, light->attenuation_q};
        ImGui::DragFloat3("Attenuation", atten, 0.001f);

        light->attenuation_c = atten[0];
        light->attenuation_l = atten[1];
        light->attenuation_q = atten[2];
    }

    if(type == BR_LIGHT_SPOT) {
        float c[2] = {BrAngleToDegree(light->cone_inner), BrAngleToDegree(light->cone_outer)};
        ImGui::SliderFloat2("Cone", c, 0.0f, 180.0f);
        light->cone_inner = BrDegreeToAngle(c[0]);
        light->cone_outer = BrDegreeToAngle(c[1]);
    }

    if(linear_falloff) {
        float c[2] = {light->radius_inner, light->radius_outer};
        ImGui::SliderFloat2("Radius", c, 0.0f, 360.0f);
        light->radius_inner = c[0];
        light->radius_outer = c[1];
    }
}

void Editor::InspectorPane::Draw(br_actor *root, size_t slot)
{
    Editor::ActorState *state;

    ImGui::PushID(root);

    if(!ImGui::Begin("Editor##BREditor")) {
        ImGui::End();
        ImGui::PopID();
        return;
    }

    if(root == nullptr) {
        ImGui::Text("No item selected");
        ImGui::End();
        ImGui::PopID();
        return;
    }

    state = Editor::ActorUser::Get(root)->slots + slot;

    DrawIdentifier(root, "Identifier##ACTOR");

    ImGui::InputUInt16("Depth", &root->depth, 0, 0, ImGuiInputTextFlags_ReadOnly);

    {
        static const char *rstyles[] = {
            /* [BR_RSTYLE_DEFAULT]           = */ "Default",
            /* [BR_RSTYLE_NONE]              = */ "None",
            /* [BR_RSTYLE_POINTS]            = */ "Points",
            /* [BR_RSTYLE_EDGES]             = */ "Edges",
            /* [BR_RSTYLE_FACES]             = */ "Faces",
            /* [BR_RSTYLE_BOUNDING_POINTS]   = */ "Bounding Points",
            /* [BR_RSTYLE_BOUNDING_EDGES]    = */ "Bounding Edges",
            /* [BR_RSTYLE_BOUNDING_FACES]    = */ "Bounding Faces",
            /* [BR_RSTYLE_ANTIALIASED_LINES] = */ "Antialiased Lines",
            /* [BR_RSTYLE_ANTIALIASED_FACES] = */ "Antialiased Faces",
        };

        int style = root->render_style;
        if(ImGui::Combo("Render Style", &style, rstyles, BR_ASIZE(rstyles))) {
            if(style < 0)
                style = BR_RSTYLE_DEFAULT;

            if(style > BR_RSTYLE_MAX)
                style = BR_RSTYLE_MAX - 1;

            root->render_style = (br_uint_8)style;
        }
    }

    if(ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        DrawTransformInspector(&root->t);
    }

    switch(root->type) {
        case BR_ACTOR_CAMERA:
            ImGui::PushID("CAMERA");

            ImGui::PopID();
            break;
        case BR_ACTOR_LIGHT:
            ImGui::PushID("LIGHT");
            if(ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen)) {
                DrawLightInspector(static_cast<br_light *>(root->type_data));
            }
            ImGui::PopID();
            break;
    }

    if(ImGui::CollapsingHeader("Global Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat3("##MATRIX34_R0_GLOBAL", state->mat.m[0], 0.01f);
        ImGui::DragFloat3("##MATRIX34_R1_GLOBAL", state->mat.m[1], 0.01f);
        ImGui::DragFloat3("##MATRIX34_R2_GLOBAL", state->mat.m[2], 0.01f);
        ImGui::DragFloat3("##MATRIX34_R3_GLOBAL", state->mat.m[3], 0.01f);

        ImGui::Separator();

        ImGui::DragFloat3("Forward##GLOBAL", state->forward.v, 0.01f, 0.0f, 0.0f, "%.3f", ImGuiSliderFlags_NoInput);
        ImGui::DragFloat3("Up##GLOBAL", state->up.v, 0.01f, 0.0f, 0.0f, "%.3f", ImGuiSliderFlags_NoInput);
        ImGui::DragFloat3("Right##GLOBAL", state->right.v, 0.01f, 0.0f, 0.0f, "%.3f", ImGuiSliderFlags_NoInput);
    }


    if (ImGui::CollapsingHeader("XXTransform", ImGuiTreeNodeFlags_DefaultOpen)) {
        float matrixTranslation[3], matrixRotation[3], matrixScale[3];

        br_matrix34 m34;
        br_matrix4 m4;

        m34 = root->t.t.mat;
        BrMatrix4Copy34(&m4, &m34);

        ImGuizmo::DecomposeMatrixToComponents((float*)m4.m, matrixTranslation, matrixRotation, matrixScale);
        ImGui::DragFloat3("Tr", matrixTranslation, 0.01f);
        ImGui::DragFloat3("Rt", matrixRotation, 0.01f);
        ImGui::DragFloat3("Sc", matrixScale, 0.01f);
    }

    ImGui::End();
    ImGui::PopID();
}
