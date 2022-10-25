#include "finder.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QIODevice>
#include <QStandardPaths>

Finder::Finder(QObject *parent)
    : QObject(parent)
{
}

Finder::~Finder()
{
}

bool Finder::loadJson(QString path)
{
    // Unload current loaded parameters
    m_searches.clear();

    // Read the JSON file and load its content in a QByteArray
    QFile json_file(path);
    if(!json_file.open(QIODevice::ReadOnly))
        return false;

    QByteArray json_bytes = json_file.readAll();
    json_file.close();

    // Try to load the QByteArray in a JSON document
    QJsonParseError json_err;
    QJsonDocument json_doc = QJsonDocument::fromJson(json_bytes, &json_err);
    if(json_err.error != QJsonParseError::NoError)
        return false;

    // Load the QJsonObject from the QJsonDocument
    if(!json_doc.isObject())
        return false;
    QJsonObject json_object = json_doc.object();

    // Load the settings of the JSON file into the Finder arguments
    // Qt does not support JSON Schema, so we have to check every field

    if(!(json_object.contains("searchObjects")
         && json_object["searchObjects"].isArray()))
        return false;

    // Set target_folder if defined in the JSON file
    if(json_object.contains("targetFolder")
       && json_object["targetFolder"].isString())
    {
        m_target_folder.clear();
        m_target_folder = json_object["targetFolder"].toString();
    }

    QJsonArray search_objects = json_object["searchObjects"].toArray();

    // Load each searchObject into m_searches
    for(auto search_object_ref: search_objects){
        QJsonObject search_object = search_object_ref.toObject();
        SearchObject current_object;

        // Check fields of the search object
        if(!(search_object.contains("targetName")
             && search_object["targetName"].isString()
             && search_object.contains("linkName")
             && search_object["linkName"].isString()
             && search_object.contains("rootPaths")
             && search_object["rootPaths"].isArray()))
            continue;

        // Get the recursion level. If not defined, default to 1.
        int recursion_level = 1;
        if(search_object.contains("recursionLevel")
           && search_object["recursionLevel"].isDouble()){
            double rec_val_double = search_object["recursionLevel"].toDouble();
            if(rec_val_double == std::floor(rec_val_double))
            {
                recursion_level = (int)std::floor(rec_val_double);
            }
        }
        current_object.recursion_level = recursion_level;

        // isRegex default to "false" if not defined
        bool is_regex = false;
        if(search_object.contains("isRegex")
           && search_object["isRegex"].isBool())
        {
            is_regex = search_object["isRegex"].toBool();
        }

        QString target_name = search_object["targetName"].toString();
        // if targetName is not a regex, we create a regex from it matching
        // only the targetName
        if(!is_regex){
            target_name = "^" + target_name + "$";
        }
        current_object.target_name = QRegularExpression(target_name);

        // Set link name
        current_object.link_name = search_object["linkName"].toString();

        // Set root paths to search for
        QJsonArray root_paths_array = search_object["rootPaths"].toArray();
        for(const QJsonValue &root_path: root_paths_array){
            if(root_path.isString()){
                RootPath search_path{root_path.toString(), recursion_level, QList<QRegularExpression>()};
                replaceConstants(search_path.root_path);
                current_object.root_paths << search_path;
            }
            else if(root_path.isObject()){
                QJsonObject root_path_object = root_path.toObject();
                if(!(root_path_object.contains("rootPath")
                     && root_path_object["rootPath"].isString()))
                {
                    return false;
                }
                RootPath search_path{root_path_object["rootPath"].toString()
                            , recursion_level
                            , QList<QRegularExpression>()};
                search_path.root_path = root_path_object["rootPath"].toString();
                replaceConstants(search_path.root_path);
                // "recursionLevel is optionnal
                if(root_path_object.contains("recursionLevel")){
                    // If present, recursionLevel must be an int
                    if(!root_path_object["recursionLevel"].isDouble())
                        return false;
                    double rec_val_double = root_path_object["recursionLevel"].toDouble();
                    if(rec_val_double != std::floor(rec_val_double))
                       return false;
                    search_path.recursion_level = (int)rec_val_double;
                }

                // "filters" is optionnal
                if(root_path_object.contains("filters")){
                    if(!root_path_object["filters"].isArray())
                        return false;
                    QJsonArray filter_array = root_path_object["filters"].toArray();
                    // The filters list can only be empty or the same size as
                    // the recursion level
                    if(!(filter_array.size() == search_path.recursion_level
                         || filter_array.isEmpty()))
                    {
                        return false;
                    }
                    for(const QJsonValue &filter: filter_array){
                        if(!filter.isString())
                            return false;
                        search_path.regexs << QRegularExpression(filter.toString());
                    }
                }

                current_object.root_paths << search_path;

            }
            else return false;
        }
        // Do not add a search object without any root path to search
        if(current_object.root_paths.isEmpty())
            continue;

        m_searches << current_object;
    }

    // Return false if no search object has been loaded
    if(m_searches.isEmpty())
        return false;

    return true;
}

void Finder::search_targets()
{
    // Search for every object
    for(SearchObject &object: m_searches){
        // Search for every path defined in the object
        for(RootPath &root_path: object.root_paths){
        // for(QString &root_path: object.root_paths){
            QString path_found = search_target(
                                     object.target_name,
                                     root_path.root_path,
                                     root_path.recursion_level,
                                     root_path.regexs);

            // If a path have been found, register it in the object and search
            // the next object
            if(!path_found.isEmpty()){
                object.path = path_found;
                break;
            }
        }
    }

}

void Finder::create_shortcuts()
{
    // Don't create a shortcut if the target folder is not defined
    if(m_target_folder.isEmpty())
        return;

    QDir target_dir(m_target_folder);
    // Try to create the target folder if it does not exists
    if(!target_dir.exists())
        if(!target_dir.mkpath("."))
            return;

    // Create a shortcut in the target folder for each target found
    for(SearchObject &object: m_searches){
        if(object.path.isEmpty())
            continue;

        QFile::link(object.path,
                    m_target_folder
                        + "/"
                        + object.link_name
                        + ".lnk");
    }
}

QString Finder::search_target(QRegularExpression target_name,
        QFileInfo root_path,
        int recursion_level,
        QList<QRegularExpression> regexs)
{
    if(!root_path.exists()){
        return QString();
    }

    // Check if the root path is a valid target
    if(target_name.match(root_path.fileName()).hasMatch()){
        return root_path.absoluteFilePath();
    }
    // Return if the recursion level has been reached
    if(recursion_level == 0)
        return QString();


    // If root_path is not a dir, we cannot search deeper
    if(!root_path.isDir()){
       return QString();
    }

    // Search the target in the folder
    QDir path_dir(root_path.filePath());
    for(QFileInfo &sub_path
        : path_dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot))
    {

        if(regexs.isEmpty() // Search the folder if no regex is defined
           || regexs[regexs.size() - recursion_level] // Level of recursion
               .match(sub_path.fileName()).hasMatch() // Check if the folder name match the regex
           )
        {
            QString res = search_target(target_name, sub_path, recursion_level-1);
            if(!res.isEmpty())
                return res;
        }
    }

    // If nothing found, default to an empty QString.
    return QString();
}

void Finder::replaceConstants(QString &path){
    path.replace("<AppData>", std::getenv("AppData"));
    path.replace("<AppDataLocal>", QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation));
    path.replace("AppDataStartDir>", "C:/ProgramData/Microsoft/Windows/Start Menu/Programs");
    path.replace("<ProgramFiles>", std::getenv("PROGRAMFILES"));
    path.replace("<ProgramFilesX86>", std::getenv("PROGRAMFILES(X86)"));
    path.replace("<Home>", QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
}
