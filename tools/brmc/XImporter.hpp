#ifndef XIMPORTER_HPP
#define XIMPORTER_HPP

#include <brender.h>
#include <assimp/scene.h>
#include <assimp/texture.h>

class XImporter
{
public:
    using ImporterProc = void(*)(const aiScene *scene);

    XImporter(const XImporter&) = delete;
    XImporter(XImporter&&)      = delete;

    XImporter& operator=(const XImporter&) = delete;
    XImporter& operator=(XImporter&&)      = delete;

protected:
    XImporter() = default;

    virtual ~XImporter() noexcept = default;

    virtual br_pixelmap *ConvertTexture(const aiTexture *tex) noexcept;
    virtual br_pixelmap *FindTexture(const char *name) noexcept;
    virtual br_pixelmap *OnPixelmapFindFailed(const char *name) noexcept;

    virtual br_material *ConvertMaterial(const aiMaterial *mat) noexcept = 0;
    virtual br_material *FindMaterial(const char *name) noexcept;
    virtual br_material *OnMaterialFindFailed(const char *name) noexcept;

    virtual br_model *ConvertMesh(const aiMesh *mesh) noexcept;
    virtual br_model *FindModel(const char *name) noexcept;
    virtual br_model *OnModelFindFailed(const char *name) noexcept;

private:
    static XImporter *current_importer;

    static br_pixelmap *BR_CALLBACK PixelmapFindFailedHook(const char *name);
    static br_material *BR_CALLBACK MaterialFindFailedHook(const char *name);
    static br_model *BR_CALLBACK    ModelFindFailedHook(const char *name);
};

#endif /* XIMPORTER_HPP */
