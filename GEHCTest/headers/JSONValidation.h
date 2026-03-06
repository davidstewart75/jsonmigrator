#pragma once

#include <string>
#include <unordered_map>
#include "rapidjson/document.h"

class JSONValidation {
public:
    /**
     * @brief Validates a JSON file at the given path against its versioned schema.
     *
     * Loads the JSON document from the specified file path, then delegates
     * validation to the overloaded validate(const rapidjson::Document&) method.
     *
     * @param path The file system path to the JSON file to validate.
     * @return true  If the file was loaded successfully and passes schema validation.
     * @return false If the file could not be loaded or fails schema validation.
     */
    bool validate(const std::string& path);

    /**
     * @brief Validates a parsed JSON document against its versioned schema.
     *
     * Determines the schema version from the document's "version" field, loads
     * the corresponding schema file, and validates the document against it.
     * Prints detailed validation error information to stdout on failure.
     *
     * @param jsonDoc The parsed RapidJSON document to validate.
     * @return true  If the document passes schema validation.
     * @return false If the schema could not be loaded, or the document fails validation.
     */
    bool validate(const rapidjson::Document& jsonDoc);

private:
    /**
     * @brief Resolves the schema file path corresponding to the document's version.
     *
     * Reads the "version" integer field from the JSON document and maps it to a
     * schema file path. Supported versions are 1 through 3.
     *
     * @param jsonDoc The parsed RapidJSON document from which to read the version.
     * @return The relative path to the matching schema file (e.g. "schemas/v2Schema.json"),
     *         or an empty string if the "version" field is missing or out of the supported range.
     */
    const std::string findVersion(const rapidjson::Document& jsonDoc);
};