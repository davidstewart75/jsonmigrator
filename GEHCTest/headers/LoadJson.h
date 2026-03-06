#pragma once

#include <optional>
#include "rapidjson/document.h"
#include <string>
#include "rapidjson/filereadstream.h"
#include <iostream>

/**
 * @brief Loads and parses a JSON file from the given file path.
 *
 * Opens the file at the specified path, reads its contents into a RapidJSON
 * document, and checks for parse errors. Prints diagnostic messages to stdout
 * on failure.
 *
 * @param filePath The file system path to the JSON file to load.
 * @return A `std::optional` containing the parsed `rapidjson::Document` on success,
 *         or `std::nullopt` if the file could not be opened or contains invalid JSON.
 */
inline std::optional<rapidjson::Document> loadJson(const std::string& filePath) {
    rapidjson::Document schemaDocument;

    errno_t err;
    FILE* filepoint;
    // Open the schema file
    if ((err = fopen_s(&filepoint, filePath.c_str(), "r")) != 0) {
        std::cout << "Schema file not found" << std::endl;
        return std::nullopt;
    }

    char readBuffer[4096];
    rapidjson::FileReadStream is(filepoint, readBuffer, sizeof(readBuffer));

    // Parse the stream into the schema document
    schemaDocument.ParseStream(is);
    std::fclose(filepoint);

    // Check for parse errors in the schema file itself
    if (schemaDocument.HasParseError()) {
        std::cout << "Schema file is not a valid JSON" << std::endl;
        // ... (error handling using GetParseError() and GetErrorOffset())
        return std::nullopt;
    }
    return schemaDocument;
}
