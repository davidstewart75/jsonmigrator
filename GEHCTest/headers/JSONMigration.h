#pragma once
#include "rapidjson/document.h"
#include <string>
#include <optional>

class JSONMigrator;

class JSONMigration {
public:
    /**
     * @brief Migrates a parsed JSON document to the specified target version.
     *
     * Iterates through each version step from the document's current version up
     * to `toVersion`, applying the appropriate migrator at each step.
     *
     * @param jsonDoc   The source RapidJSON document to migrate.
     * @param resDoc    Output document that receives the fully migrated result.
     * @param toVersion The target schema version to migrate to. Defaults to 0 (no migration).
     * @return true  If migration completed successfully to the target version.
     * @return false If the document lacks a valid "version" field, the current version
     *               already exceeds `toVersion`, or no migrator exists for a required step.
     */
    bool Migrate(const rapidjson::Document& jsonDoc, rapidjson::Document& resDoc, int toVersion = 0);

    /**
     * @brief Loads, validates, and migrates a JSON file to the specified target version.
     *
     * Resolves the file path relative to the source file location, loads the JSON,
     * validates it against its schema, and then delegates to
     * Migrate(const rapidjson::Document&, rapidjson::Document&, int).
     *
     * @param fileLoc   Relative path to the source JSON file.
     * @param resDoc    Output document that receives the fully migrated result.
     * @param toVersion The target schema version to migrate to. Defaults to 0 (no migration).
     * @return true  If the file was loaded, validated, and migrated successfully.
     * @return false If the file could not be loaded, failed validation, or migration failed.
     */
    bool Migrate(std::string fileLoc, rapidjson::Document& resDoc, int toVersion = 0);

private:
    /**
     * @brief Returns the migrator instance responsible for migrating from the given version.
     *
     * Maps a source version integer to its corresponding migrator:
     * version 1 maps to JSONMigration1to2, version 2 maps to JSONMigration2to3.
     *
     * @param version The source version to migrate from.
     * @return A `std::optional` containing the appropriate `JSONMigrator` instance,
     *         or `std::nullopt` if no migrator exists for the given version.
     */
    std::unique_ptr<JSONMigrator> getMigrator(int version);
};
