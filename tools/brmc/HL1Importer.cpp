#include "HL1Importer.hpp"

br_material *HL1Importer::ConvertMaterial(const aiMaterial *mat) noexcept
{
    return this->ConvertMaterial(mat, 0);
}

br_material *HL1Importer::ConvertMaterial(const aiMaterial *mat, size_t skin_index) noexcept
{
    aiString     name;
    br_material *mp;
    br_pixelmap *pm;
    const char  *identifier;

    /*
     * Get the texture, first. We have a 1:1 mapping from texture:material.
     */
    mat->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, skin_index), name);
    if(name.length > 0)
        pm = this->FindTexture(name.C_Str());

    if((mp = BrMaterialAllocate(nullptr)) == nullptr)
        return nullptr;

    mp->colour_map = pm;

    /*
     * Materials don't have names, use the texture's.
     */
    identifier = mat->GetName().C_Str();
    if(*identifier == '\0' && pm != nullptr)
        identifier = pm->identifier;

    if(identifier != nullptr)
        mp->identifier = BrResStrDup(mp, identifier);

    /*
     * Now figure out the flags.
     */
    mp->mode  = BR_MATM_DEPTH_TEST_LE | BR_MATM_MAP_WIDTH_LIMIT_WRAP | BR_MATM_MAP_HEIGHT_LIMIT_WRAP;
    mp->flags = BR_MATF_LIGHT;

    /*
     * Shading mode.
     */
    aiShadingMode shading_mode = aiShadingMode_Blinn;
    mat->Get(AI_MATKEY_SHADING_MODEL, shading_mode);

    if(shading_mode != aiShadingMode_Flat)
        mp->mode |= BR_MATF_SMOOTH;

    /*
     * Blend mode.
     */
    aiBlendMode blend_mode = aiBlendMode_Default;
    mat->Get(AI_MATKEY_BLEND_FUNC, blend_mode);

    if(blend_mode == aiBlendMode_Additive)
        mp->mode |= BR_MATM_BLEND_MODE_SUMMED;

    /*
     * Alpha testing/colour-keying.
     */
    int texture_flags = 0xFFFF;
    mat->Get(AI_MATKEY_TEXFLAGS_DIFFUSE(skin_index), texture_flags);

    if(texture_flags & aiTextureFlags_UseAlpha) {
        aiColor3D key = {0.0f, 0.0f, 0.0f};
        mat->Get(AI_MATKEY_COLOR_TRANSPARENT, key);

        if(pm != nullptr) {
            pm->key = BR_COLOUR_RGB(static_cast<br_uint_8>(key.r * 255.0f), static_cast<br_uint_8>(key.g * 255.0f),
                                    static_cast<br_uint_8>(key.b * 255.0f));
        }
    } else {
        mp->flags |= BR_MATF_DISABLE_COLOUR_KEY;
    }

    /*
     * "My favourite colour is... $chrome"
     */
    int chrome = 0;
    mat->Get("$mat.HL1.chrome", aiTextureType_DIFFUSE, skin_index, chrome);
    if(chrome != 0)
        mp->flags |= BR_MATF_ENVIRONMENT_L;

    BrMaterialAdd(mp);
    return mp;
}

void HL1Importer::Import(const aiScene *scene) noexcept
{
    HL1Importer importer;
    importer.mTextures.reserve(scene->mNumTextures);
    importer.mSkins.reserve(scene->mNumMaterials);
    importer.mModels.reserve(scene->mNumMeshes);

    /*
     * HL1 models have all their textures embedded.
     * Makes things easy.
     */
    for(size_t i = 0; i < scene->mNumTextures; ++i) {
        importer.mTextures.push_back(importer.ConvertTexture(scene->mTextures[i]));
    }

    /*
     * Materials.
     */
    for(size_t i = 0; i < scene->mNumMaterials; ++i) {
        const aiMaterial *mat  = scene->mMaterials[i];
        Skin&             skin = importer.mSkins.emplace_back();

        /*
         * Handle multiple skins.
         */
        skin.count = scene->mMaterials[i]->GetTextureCount(aiTextureType_DIFFUSE);
        for(size_t j = 0; j < skin.count; ++j) {
            skin.materials[j] = importer.ConvertMaterial(mat, j);
        }
    }

    /*
     * Convert all the meshes.
     */
    for(size_t i = 0; i < scene->mNumMeshes; ++i) {
        const aiMesh *mesh  = scene->mMeshes[i];
        const Skin&   skin  = importer.mSkins[mesh->mMaterialIndex];
        Model&        model = importer.mModels.emplace_back();

        model.model = importer.ConvertMesh(scene->mMeshes[i]);
        model.count = skin.count;

        for(size_t j = 0; j < skin.count; ++j) {
            br_actor *brap   = BrActorAllocate(BR_ACTOR_MODEL, nullptr);
            brap->identifier = BrResSprintf(brap, "mesh_%zu_skin_%zu", i, j);
            brap->t.type     = BR_TRANSFORM_MATRIX34;
            brap->model      = model.model;
            brap->material   = skin.materials[j];
            model.actors[j]  = brap;
        }
    }

    const aiNode *bodyparts = scene->mRootNode->FindNode("<MDL_bodyparts>");

    br_actor *root   = BrActorAllocate(BR_ACTOR_NONE, nullptr);
    root->identifier = BrResStrDup(root, "root");

    for(size_t i = 0; i < bodyparts->mNumChildren; ++i) {
        const aiNode *part = bodyparts->mChildren[i];

        br_actor *brap   = BrActorAdd(root, BrActorAllocate(BR_ACTOR_NONE, nullptr));
        brap->identifier = BrResStrDup(brap, part->mName.C_Str());
        brap->t.type     = BR_TRANSFORM_IDENTITY; /* We're for grouping only. */

        int base = -1;
        part->mMetaData->Get("Base", base);

        for(size_t j = 0; j < part->mNumChildren; ++j) {
            const aiNode *model = part->mChildren[j];

            for(size_t k = 0; k < model->mNumMeshes; ++k) {
                const Model& mdl = importer.mModels[model->mMeshes[k]];

                mdl.model->identifier = BrResSprintf(mdl.model, "%s_%zu", model->mName.C_Str(), k);

                if(mdl.count == 1) {
//                    if(mdl.actors[0]->identifier != nullptr)
//                        BrResFree(mdl.actors[0]->identifier);

                    //mdl.actors[0]->identifier

                    BrActorAdd(brap, mdl.actors[0]);
                    continue;
                }

                br_actor *group   = BrActorAdd(brap, BrActorAllocate(BR_ACTOR_NONE, nullptr));
                group->identifier = BrResSprintf(group, "%s_%zu", brap->identifier, k);
                group->t.type     = BR_TRANSFORM_IDENTITY;

                for(size_t w = 0; w < mdl.count; ++w) {
//                    if(mdl.actors[w]->identifier != nullptr)
//                        BrResFree(mdl.actors[w]->identifier);
//                    mdl.actors[w]->identifier = BrResSprintf();

                    BrActorAdd(group, mdl.actors[w]);
                }
            }
        }
    }

    BrActorSave("sci.txt", root);
}
