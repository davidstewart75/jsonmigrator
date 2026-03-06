#include "rapidjson/document.h"
#include "rapidjson/schema.h"
#include "rapidjson/error/en.h"
#include "rapidjson/filereadstream.h"
#include <iostream>
#include <string>
#include <optional>
#include "../headers/JSONValidation.h"
#include "../headers/LoadJson.h"


static std::vector<std::string> SCHEMA_FILE_PATHS = {
    "schemas/v1Schema.json",
    "schemas/v2Schema.json",
    "schemas/v3Schema.json"
};

/**
 * @brief Validates a JSON file at the given path against its versioned schema.
 *
 * Loads the JSON document from the specified file path, then delegates
 * validation to the overloaded validate(const rapidjson::Document&) method.
 *
 * @param path The file system path to the JSON file to validate.
 * @return true  If the JSON file was loaded successfully and passes schema validation.
 * @return false If the file could not be loaded or fails schema validation.
 */
bool JSONValidation::validate(const std::string& path) {
	std::optional<rapidjson::Document> jsonDoc = loadJson(path);

    if (!jsonDoc.has_value()) {
        std::cout << "Failed to load JSON document from path: " << path << std::endl;
        return false;
	}
    return validate(jsonDoc.value());
}

/**
 * @brief Validates a parsed JSON document against its versioned schema.
 *
 * Determines the schema version from the document's "version" field, loads
 * the corresponding schema file, and validates the document against it.
 * Prints detailed schema validation error information to stdout on failure.
 *
 * @param jsonDoc The parsed RapidJSON document to validate.
 * @return true  If the document passes schema validation.
 * @return false If the schema could not be loaded, or the document fails validation.
 */
bool JSONValidation::validate(const rapidjson::Document& jsonDoc) {
	std::string versionKey = findVersion(jsonDoc);
    std::string thisFile(__FILE__);
    std::string dir = thisFile.substr(0, thisFile.find_last_of("\\/") + 1);
    std::string fullPath = dir + "../" + versionKey;

    std::optional<rapidjson::Document> schemaJson = loadJson(fullPath);

    if (!schemaJson.has_value()) {
        std::cout << "Failed to load JSON schema for version: " << versionKey << std::endl;
        return false;
	}

    rapidjson::SchemaDocument schema(schemaJson.value());
    rapidjson::SchemaValidator validator(schema);

    if (jsonDoc.Accept(validator)) {
        return true;
    }
    else {
        std::cout << "Input JSON is invalid." << std::endl;
        // Optionally, retrieve and print validation error details
        rapidjson::StringBuffer sb;
        validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);
        std::cout << "Invalid schema: " << sb.GetString() << std::endl;
        std::cout << "Invalid keyword: " << validator.GetInvalidSchemaKeyword() << std::endl;
        sb.Clear();
        validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);
        std::cout << "Invalid document: " << sb.GetString() << std::endl;
        return false;
    }

    return false;
}

/**
 * @brief Resolves the schema file path corresponding to the document's version.
 *
 * Reads the "version" integer field from the JSON document and maps it to a
 * schema file path from the internal SCHEMA_FILE_PATHS list.
 * Supported versions are 1 through 3.
 *
 * @param jsonDoc The parsed RapidJSON document from which to read the version.
 * @return The relative path to the matching schema file (e.g. "schemas/v2Schema.json"),
 *         or an empty string if the "version" field is missing or out of the supported range.
 */
const std::string JSONValidation::findVersion(const rapidjson::Document& jsonDoc) {
    std::string versionKey = "version";
    if (jsonDoc.HasMember(versionKey.c_str())) {
        int version = jsonDoc[versionKey.c_str()].GetInt();
        if (version < 1 || version > 3) {
            std::cout << "Unsupported JSON version: " << version << std::endl;
            return "";
		}
		return SCHEMA_FILE_PATHS[version - 1];
    }
    else {
        std::cout << "JSON document does not contain a valid 'version' field." << std::endl;
        return "";
    }
}