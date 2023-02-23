#ifndef MATERIAL_FILE_H
#define MATERIAL_FILE_H

#include "xml/xml_file.h"
#include "string"
#include "materials/material.h"

namespace cell {

    class MaterialFile : protected undicht::tools::XmlFile {

      protected:

        std::string _file_path; // path to the file that is currently open
        std::string _file_name; // name of the current file
        std::string _mat_file; // path + name

      public:

        MaterialFile() = default;
        MaterialFile(const std::string& file_name);

        /// @brief loads in the content stored in the material file
        /// @return true if the file exists and is a correct material file
        bool open(const std::string& file_name);

        void newMatFile();

        // load materials

        bool loadAllMaterials(std::vector<Material>& load_to) const;

    };

} // cell

#endif // MATERIAL_FILE_H