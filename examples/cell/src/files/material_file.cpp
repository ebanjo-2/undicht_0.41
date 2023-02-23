#include "material_file.h"
#include "file_tools.h"

#include "debug.h"

using namespace undicht;
using namespace tools;

namespace cell {

    const std::string CURRENT_MAT_VERSION = "0.0.1";

    MaterialFile::MaterialFile(const std::string& file_name) {

        open(file_name);
    }

    bool MaterialFile::open(const std::string& file_name) {
        /// @brief loads in the content stored in the material file
        /// @return true if the file exists and is a correct material file

        _file_path = getFilePath(file_name);
        _file_name = getFileName(file_name);
        _mat_file = file_name;

        if(!XmlFile::open(file_name)) return false;

        // check if the file is a correct material file

        // root element tag name
        if(getName().compare("MATERIAL_FILE")) return false;
        
        // version attribute
        XmlTagAttrib* version = getAttribute("version");
        if(!version) return false;
        if(version->m_value.compare(CURRENT_MAT_VERSION)) UND_WARNING << "material file version does not match the current version (version " << CURRENT_MAT_VERSION << ")\n";

        // materials element
        XmlElement* mats = getElement({"MATERIALS"});
        if(!mats) return false;

        return true;
    }

    void MaterialFile::newMatFile() {

        // root element
        setName("MATERIAL_FILE");
        addTagAttrib("version=" + CURRENT_MAT_VERSION);

        // add a materials element
        addChildElement("MATERIALS");

        // should create a new file if it doesnt exist already
        XmlFile::write(_mat_file);
    }

    /////////////////////////////////////// load materials //////////////////////////////

    bool MaterialFile::loadAllMaterials(std::vector<Material>& load_to) const {

        XmlElement* materials = getElement({"MATERIALS"});
        if(!materials) {
            UND_LOG << "failed to read materials from material file (no MATERIALS element)\n"; 
            return false;
        }

        std::vector<XmlElement*> mat_elements = materials->getAllElements({"MATERIAL"});

        for(XmlElement* mat : mat_elements) {

            XmlTagAttrib* name = mat->getAttribute("name");
            XmlElement* albedo = mat->getElement({"ALBEDO_ROUGHNESS"});
            XmlElement* normal = mat->getElement({"NORMAL_METALNESS"});

            std::string name_str, albedo_str, normal_str;

            if(name) name_str = name->m_value;
            else UND_WARNING << "loaded material has no name\n";

            if(albedo) albedo_str = _file_path + albedo->getContent();
            else UND_WARNING << "loaded material has no albedo\n";

            if(normal) normal_str = _file_path + normal->getContent();
            // else UND_WARNING << "loaded material has no normal\n"; // not specifying a normal / metalness map is fine

            load_to.emplace_back(Material(name_str, albedo_str, normal_str));
        }

        return true;
    }


} // cell