#ifndef HL1IMPORTER_HPP
#define HL1IMPORTER_HPP

#include <vector>
#include <array>
#include "XImporter.hpp"

class HL1Importer : public XImporter
{
public:
    static void Import(const aiScene *scene) noexcept;

protected:
    HL1Importer() = default;

    br_material *ConvertMaterial(const aiMaterial *mat) noexcept override;

private:
    br_material *ConvertMaterial(const aiMaterial *mat, size_t skin_index) noexcept;

    struct Skin {
        std::array<br_material *, 32> materials;
        size_t                        count;
    };

    struct Model {
        br_model                  *model;
        std::array<br_actor *, 32> actors;
        size_t                     count;
    };

    std::vector<br_pixelmap *> mTextures;
    std::vector<Skin>          mSkins;
    std::vector<Model>         mModels;
};

#endif /* HL1IMPORTER_HPP */
