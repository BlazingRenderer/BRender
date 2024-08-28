#include <memory>
#include <vector>
#include <brender.h>
#include <assimp/Importer.hpp>
#include <assimp/BaseImporter.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct br_pixelmap_deleter {
    using pointer = br_pixelmap *;
    void operator()(pointer p) const noexcept
    {
        BrPixelmapFree(p);
    }
};
using br_pixelmap_ptr = std::unique_ptr<br_pixelmap, br_pixelmap_deleter>;

struct br_material_deleter {
    using pointer = br_material *;
    void operator()(pointer p) const noexcept
    {
        BrMaterialFree(p);
    }
};
using br_material_ptr = std::unique_ptr<br_material, br_material_deleter>;

struct br_model_deleter {
    using pointer = br_model *;
    void operator()(pointer p) const noexcept
    {
        BrModelFree(p);
    }
};
using br_model_ptr = std::unique_ptr<br_model, br_model_deleter>;

static br_actor *add_mesh_actor(const aiScene *scene, const aiNode *root, size_t idx)
{
    br_actor     *brap;
    const aiMesh *mesh = scene->mMeshes[idx];

    brap             = BrActorAllocate(BR_ACTOR_MODEL, nullptr);
    brap->identifier = BrResStrDup(brap, mesh->mName.C_Str());

    // TODO: get model

    return brap;
}

static br_actor *asdfasd(const aiScene *scene, const aiNode *root, br_actor *brparent)
{
    br_actor  *brap;
    br_matrix4 m;

    brap             = BrActorAllocate(BR_ACTOR_NONE, nullptr);
    brap->identifier = BrResStrDup(brap, root->mName.C_Str());

    if(brparent != nullptr)
        BrActorAdd(brparent, brap);

    // clang-format off
    m = {{
        {root->mTransformation.a1, root->mTransformation.a2, root->mTransformation.a3, root->mTransformation.a4},
        {root->mTransformation.b1, root->mTransformation.b2, root->mTransformation.b3, root->mTransformation.b4},
        {root->mTransformation.c1, root->mTransformation.c2, root->mTransformation.c3, root->mTransformation.c4},
        {root->mTransformation.d1, root->mTransformation.d2, root->mTransformation.d3, root->mTransformation.d4},
    }};
    // clang-format on

    brap->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34Copy4(&brap->t.t.mat, &m);

    if(root->mNumMeshes != 0) {
        for(size_t i = 0; i < root->mNumMeshes; ++i) {
            BrActorAdd(brap, add_mesh_actor(scene, root, root->mMeshes[i]));
        }
    }

    for(size_t i = 0; i < root->mNumChildren; ++i) {
        asdfasd(scene, root->mChildren[i], brap);
    }

    return brap;
}

static br_pixelmap *create_texture(const aiTexture *tex)
{
    br_pixelmap *brpp;

    if(tex->mHeight == 0) {
        /* TODO: compressed textures? */
        return nullptr;
    }

    brpp = BrPixelmapAllocate(BR_PMT_RGBA_8888, static_cast<br_int_32>(tex->mWidth),
                              static_cast<br_int_32>(tex->mHeight), nullptr, BR_PMAF_NORMAL);

    brpp->identifier = BrResStrDup(brpp, tex->mFilename.C_Str());

    memcpy(brpp->pixels, tex->pcData, brpp->width * brpp->height * sizeof(br_uint_32));
    return brpp;
}

const aiScene *current_scene = nullptr;

static br_pixelmap *BR_CALLBACK assimp_texture_find_failed_load_hook(const char *name)
{
    br_pixelmap *pm = nullptr;

    if(current_scene == nullptr)
        return nullptr;

    if(const aiTexture *tex = current_scene->GetEmbeddedTexture(name); tex != nullptr) {
        pm = create_texture(tex);
        goto found;
    }

    // TODO: attempt to load from disk

found:
    if(pm != nullptr)
        BrMapAdd(pm);

    return pm;
}

static br_material *create_material(const aiMaterial *mat)
{
    br_material *brmp;

    brmp = BrMaterialAllocate(mat->GetName().C_Str());

#if 0
    aiTextureMapping mapping = aiTextureMapping_UV;
    mat->Get(AI_MATKEY_MAPPING(aiTextureMapping_UV, 0), mapping);

    int uv_index = -1;
    mat->Get(AI_MATKEY_UVWSRC(0, 0), uv_index);
#endif

    aiTextureMapMode map_u = aiTextureMapMode_Wrap;
    mat->Get(AI_MATKEY_MAPPINGMODE_U(0, 0), map_u);

    aiTextureMapMode map_v = aiTextureMapMode_Wrap;
    mat->Get(AI_MATKEY_MAPPINGMODE_V(0, 0), map_v);

    aiString path;
    mat->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), path);
    if(path.length > 0) {
        brmp->colour_map = BrMapFind(path.C_Str());
    }

    unsigned int     uvindex  = 1000;
    ai_real          blend    = 4.0f;
    aiTextureOp      op       = aiTextureOp_Multiply;
    aiTextureMapMode map_mode = aiTextureMapMode_Decal;

    return brmp;
}

static br_model *create_mesh(const aiMesh *mesh)
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

#include <unordered_map>
#include <string_view>
#include "XImporter.hpp"
#include "HL1Importer.hpp"

std::unordered_map<std::string_view, XImporter::ImporterProc> s = {
    {"hl1", HL1Importer::Import},
};

int main(int argc, char **argv)
{
    Assimp::Importer importer;

    BrBegin();

    BrWriteModeSet(BR_FS_MODE_TEXT);

    //    // clang-format off
    //    BrModelEnum(nullptr, [](br_model *m, void *arg) -> br_uint_32 {
    //        BrModelRemove(m);
    //        BrModelFree(m);
    //        return 0;
    //    }, nullptr);
    //
    //    BrMaterialEnum(nullptr, [](br_material *mat, void *arg) -> br_uint_32 {
    //        BrMaterialRemove(mat);
    //        BrMaterialFree(mat);
    //        return 0;
    //    }, nullptr);
    //
    //    BrMapEnum(nullptr, [](br_pixelmap *mat, void *arg) -> br_uint_32 {
    //        BrMapRemove(mat);
    //        BrPixelmapFree(mat);
    //        return 0;
    //    }, nullptr);
    //    // clang-format on

    //BrMapFindHook(assimp_texture_find_failed_load_hook);

    for(int i = 1; i < argc; ++i) {
        const aiScene *scene = importer.ReadFile(argv[i], aiProcessPreset_TargetRealtime_Quality | aiProcess_TransformUVCoords |
                                                              aiProcess_OptimizeMeshes /* | aiProcess_OptimizeGraph */);
        if(scene == nullptr) {
            BrLogError("APP", "Failed to import \"%s\": %s", argv[i], importer.GetErrorString());
            continue;
        }

        HL1Importer::Import(scene);
        continue;
        for(size_t ii = 0; ii < importer.GetImporterCount(); ++ii) {
            auto                  aii = importer.GetImporterInfo(ii);
            Assimp::BaseImporter *bi  = importer.GetImporter(ii);
            int                   xxx = 0;
        }
        current_scene = scene;

        std::vector<br_pixelmap_ptr> pixelmaps;
        pixelmaps.reserve(scene->mNumTextures);

        std::vector<br_material_ptr> materials;
        materials.reserve(scene->mNumMaterials);

        std::vector<br_model_ptr> models;
        models.reserve(scene->mNumMeshes);

        //        for(size_t t = 0; t < scene->mNumTextures; ++t) {
        //            br_pixelmap *pm = create_texture(scene->mTextures[t]);
        //            BrMapAdd(pm);
        //            pixelmaps.emplace_back(pm);
        //        }

        for(size_t m = 0; m < scene->mNumMaterials; ++m) {
            materials.emplace_back(create_material(scene->mMaterials[m]));
        }

        br_actor *world = asdfasd(scene, scene->mRootNode, nullptr);

        for(size_t m = 0; m < scene->mNumMeshes; ++m) {
            models.emplace_back(create_mesh(scene->mMeshes[m]));
        }

        BrPixelmapSaveMany("sci.pix", (br_pixelmap **)pixelmaps.data(), pixelmaps.size());
        BrModelSaveMany("sci.mod", (br_model **)models.data(), models.size());
        BrActorSave("sci.txt", world);
        BrActorFree(world);
    }

    BrEnd();
}

//
// int main(int argc, char **argv)
//{
//    int i, n;
//    BrBegin();
//
//    BrLogSetLevel(BR_LOG_TRACE);
//
//    for (i = 1; i < argc; i++)
//    {
//        /* import scene */
//
//        unsigned int flags = aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_GenBoundingBoxes | aiProcess_TransformUVCoords;
//        const struct aiScene *scene = aiImportFile(argv[i], flags);
//        if (!scene)
//        {
//            BrLogError("APP", "Failed to import \"%s\": %s", argv[i], aiGetErrorString());
//            continue;
//        }
//
//        BrLogInfo("APP", "Successfully imported \"%s\"", argv[i]);
//
//        /* walk scene nodes */
//        /* node_walk(scene, scene->mRootNode); */
//
//        /* allocate a buffer of models */
//        br_model **models = BrResAllocate(NULL, sizeof(br_model *) * scene->mNumMeshes, BR_MEMORY_MODEL);
//
//        /* process all meshes */
//        for (int m = 0; m < scene->mNumMeshes; m++)
//        {
//            BrLogInfo("APP", "Processing mesh %d: \"%.*s\"", m, scene->mMeshes[m]->mName.length, scene->mMeshes[m]->mName.data);
//
//            models[m] = BrModelAllocate(scene->mMeshes[m]->mName.data, scene->mMeshes[m]->mNumVertices, scene->mMeshes[m]->mNumFaces);
//
//            /* make vertices */
//            for (int v = 0; v < scene->mMeshes[m]->mNumVertices; v++)
//            {
//                models[m]->vertices[v].p.v[0] = BR_SCALAR(scene->mMeshes[m]->mVertices[v].x);
//                models[m]->vertices[v].p.v[1] = BR_SCALAR(scene->mMeshes[m]->mVertices[v].y);
//                models[m]->vertices[v].p.v[2] = BR_SCALAR(scene->mMeshes[m]->mVertices[v].z);
//            }
//
//            /* make uvs */
//            if (scene->mMeshes[m]->mTextureCoords[0])
//            {
//                for (int v = 0; v < scene->mMeshes[m]->mNumVertices; v++)
//                {
//                    models[m]->vertices[v].map.v[0] = BR_SCALAR(scene->mMeshes[m]->mTextureCoords[0][v].x);
//                    models[m]->vertices[v].map.v[1] = BR_SCALAR(scene->mMeshes[m]->mTextureCoords[0][v].y);
//                }
//            }
//
//            /* make colors */
//            if (scene->mMeshes[m]->mColors[0])
//            {
//                for (int v = 0; v < scene->mMeshes[m]->mNumVertices; v++)
//                {
//                    models[m]->vertices[v].red = (br_uint_8)(scene->mMeshes[m]->mColors[0][v].r * 255.0f);
//                    models[m]->vertices[v].grn = (br_uint_8)(scene->mMeshes[m]->mColors[0][v].g * 255.0f);
//                    models[m]->vertices[v].blu = (br_uint_8)(scene->mMeshes[m]->mColors[0][v].b * 255.0f);
//                }
//            }
//
//            /* make normals */
//            if (scene->mMeshes[m]->mNormals)
//            {
//                for (int v = 0; v < scene->mMeshes[m]->mNumVertices; v++)
//                {
//                    models[m]->vertices[v].n.v[0] = scene->mMeshes[m]->mNormals[v].x;
//                    models[m]->vertices[v].n.v[1] = scene->mMeshes[m]->mNormals[v].y;
//                    models[m]->vertices[v].n.v[2] = scene->mMeshes[m]->mNormals[v].z;
//                }
//            }
//
//            /* make faces */
//            for (int f = 0; f < scene->mMeshes[m]->mNumFaces; f++)
//            {
//                models[m]->faces[f].vertices[0] = scene->mMeshes[m]->mFaces[f].mIndices[0];
//                models[m]->faces[f].vertices[1] = scene->mMeshes[m]->mFaces[f].mIndices[1];
//                models[m]->faces[f].vertices[2] = scene->mMeshes[m]->mFaces[f].mIndices[2];
//            }
//
//            BrLogInfo("APP", "Successfully processed mesh %d with %d vertices and %d faces", m, scene->mMeshes[m]->mNumVertices, scene->mMeshes[m]->mNumFaces);
//        }
//
//        /* make filename */
//        char filename[256];
//        char *p = BrStrRChr(argv[i], '.');
//        if (p)
//        {
//            size_t ofs = p - argv[i];
//            BrSprintfN(filename, sizeof(filename), "%s.dat", argv[i]);
//            BrSprintfN(filename + ofs, sizeof(filename) - ofs, ".dat");
//        }
//        else
//        {
//            BrSprintfN(filename, sizeof(filename), "%s.dat", argv[i]);
//        }
//
//        /* finally save to disk */
//        BrModelSaveMany(filename, models, scene->mNumMeshes);
//        BrLogInfo("APP", "Saved %d meshes to disk as \"%s\"", scene->mNumMeshes, filename);
//
//        /* free all models */
//        for (int m = 0; m < scene->mNumMeshes; m++)
//            BrModelFree(models[m]);
//
//        BrResFree(models);
//
//        /* free scene */
//        aiReleaseImport(scene);
//    }
//
//    BrLogInfo("APP", "Processed %d arguments", i - 1);
//
//    BrEnd();
//
//    return 0;
//}
