#ifndef HL1IMPORTER_HPP
#define HL1IMPORTER_HPP

#include "XImporter.hpp"

class HL1Importer : public XImporter
{
public:
    static void Import(const aiScene *scene) noexcept;

protected:
    HL1Importer() = default;

    br_material *ConvertMaterial(const aiMaterial *mat) noexcept override;
};

#endif /* HL1IMPORTER_HPP */
