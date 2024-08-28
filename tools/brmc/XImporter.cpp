#include "XImporter.hpp"

XImporter *XImporter::current_importer = nullptr;

br_pixelmap *XImporter::ConvertTexture(const aiTexture *tex) noexcept
{
    br_pixelmap *pm;

    if(tex->mHeight == 0) {
        // TODO: compressed textures?
        return nullptr;
    }

    pm = BrPixelmapAllocate(BR_PMT_RGBA_8888, static_cast<br_int_32>(tex->mWidth), static_cast<br_int_32>(tex->mHeight),
                            nullptr, BR_PMAF_NORMAL);

    pm->identifier = BrResStrDup(pm, tex->mFilename.C_Str());

    memcpy(pm->pixels, tex->pcData, pm->width * pm->height * sizeof(br_uint_32));

    BrMapAdd(pm);
    return pm;
}

br_pixelmap *XImporter::FindTexture(const char *name) noexcept
{
    br_pixelmap *pm;
    XImporter   *prev;

    prev                        = XImporter::current_importer;
    XImporter::current_importer = this;

    BrMapFindHook(XImporter::PixelmapFindFailedHook);

    pm                          = BrMapFind(name);
    XImporter::current_importer = prev;

    return pm;
}

br_pixelmap *XImporter::OnPixelmapFindFailed(const char *name) noexcept
{
    br_pixelmap *pm    = nullptr;
    aiScene     *scene = nullptr; // TODO:

    if(const aiTexture *tex = scene->GetEmbeddedTexture(name); tex != nullptr) {
        pm = this->ConvertTexture(tex);
        goto found;
    }

    // TODO: attempt to load from disk

found:
    if(pm != nullptr)
        BrMapAdd(pm);

    return pm;
}

br_material *XImporter::FindMaterial(const char *name) noexcept
{
    br_material *mat;
    XImporter   *prev;

    prev                        = XImporter::current_importer;
    XImporter::current_importer = this;

    BrMaterialFindHook(XImporter::MaterialFindFailedHook);

    mat                         = BrMaterialFind(name);
    XImporter::current_importer = prev;

    return mat;
}

br_material *XImporter::OnMaterialFindFailed(const char *name) noexcept
{
    return nullptr; // TODO:
}

br_model *XImporter::ConvertMesh(const aiMesh *mesh) noexcept
{
    br_model *brmp;

    if(!(mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE))
        return nullptr;

    brmp = BrModelAllocate(mesh->mName.C_Str(), static_cast<int>(mesh->mNumVertices), static_cast<int>(mesh->mNumFaces));

    /*
     * Process vertices.
     */
    for(size_t v = 0; v < mesh->mNumVertices; v++) {
        const aiVector3D pos  = mesh->mVertices[v];
        const aiVector3D norm = mesh->mNormals[v];

        br_vertex *brvp = brmp->vertices + v;

        brvp->p = BR_VECTOR3(BR_SCALAR(pos.x), BR_SCALAR(pos.y), BR_SCALAR(pos.z));
        brvp->n = BR_VECTOR3(BR_SCALAR(norm.x), BR_SCALAR(norm.y), BR_SCALAR(norm.z));

        if(mesh->mTextureCoords[0] != nullptr) {
            const aiVector3D uv = mesh->mTextureCoords[0][v];
            brvp->map           = BR_VECTOR2(BR_SCALAR(uv.x), BR_SCALAR(uv.y));
        }

        if(mesh->mColors[0] != nullptr) {
            const aiColor4D col = mesh->mColors[0][v];
            brvp->red           = static_cast<br_uint_8>(col.r * 255.0f);
            brvp->grn           = static_cast<br_uint_8>(col.g * 255.0f);
            brvp->blu           = static_cast<br_uint_8>(col.b * 255.0f);
        }
    }

    /*
     * Process faces.
     */
    for(size_t f = 0; f < mesh->mNumFaces; ++f) {
        const aiFace *face = mesh->mFaces + f;
        br_face      *fp   = brmp->faces + f;

        fp->vertices[0] = static_cast<br_uint_16>(face->mIndices[0]);
        fp->vertices[1] = static_cast<br_uint_16>(face->mIndices[1]);
        fp->vertices[2] = static_cast<br_uint_16>(face->mIndices[2]);
    }

    return brmp;
}

br_model *XImporter::FindModel(const char *name) noexcept
{
    br_model  *model;
    XImporter *prev;

    prev                        = XImporter::current_importer;
    XImporter::current_importer = this;

    BrModelFindHook(XImporter::ModelFindFailedHook);

    model                       = BrModelFind(name);
    XImporter::current_importer = prev;

    return model;
}

br_model *XImporter::OnModelFindFailed(const char *name) noexcept
{
    return nullptr; // TODO:
}

br_pixelmap *XImporter::PixelmapFindFailedHook(const char *name)
{
    return XImporter::current_importer->OnPixelmapFindFailed(name);
}

br_material *XImporter::MaterialFindFailedHook(const char *name)
{
    return XImporter::current_importer->OnMaterialFindFailed(name);
}

br_model *XImporter::ModelFindFailedHook(const char *name)
{
    return XImporter::current_importer->OnModelFindFailed(name);
}
