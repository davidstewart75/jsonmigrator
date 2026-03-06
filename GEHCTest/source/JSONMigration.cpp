#include "../headers/JSONMigration.h"
#include "rapidjson/document.h"
#include "rapidjson/allocators.h"
#include "../headers/JSONValidation.h"
#include <optional>
#include <unordered_map>
#include "../headers/LoadJson.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#include <algorithm>

class JSONMigrator {
public:
    /**
     * @brief Migrates a JSON document to the next schema version.
     *
     * Base implementation — always returns false. Derived classes override
     * this method to perform the actual migration logic.
     *
     * @param jsonDoc The source RapidJSON document to migrate.
     * @param resDoc  Output document that receives the migrated result.
     * @return true  If migration succeeded.
     * @return false Always in this base implementation.
     */
    virtual bool migrate(const rapidjson::Document& jsonDoc, rapidjson::Document &resDoc) {
        return false;
    }
};

class JSONMigration1to2 : public JSONMigrator {
public:
    /**
     * @brief Migrates a v1 JSON document to the v2 schema.
     *
     * Restructures the flat v1 layout into the nested v2 player object,
     * grouping stats and consolidating duplicate inventory entries into
     * id/count pairs.
     *
     * @param jsonDoc The source v1 RapidJSON document.
     * @param resDoc  Output document that receives the migrated v2 result.
     * @return true  If migration completed successfully.
     * @return false If migration failed.
     */
    bool migrate(const rapidjson::Document& jsonDoc, rapidjson::Document& resDoc) {
        // Get the allocator from the document
        rapidjson::Document::AllocatorType& allocator = resDoc.GetAllocator();

        resDoc.AddMember("version", 2, resDoc.GetAllocator());


        // stats object
        rapidjson::Value stats(rapidjson::kObjectType);
        stats.AddMember("hp", jsonDoc["hp"].GetInt(), allocator);
        stats.AddMember("mp", 0, allocator);

        // player object
        rapidjson::Value player(rapidjson::kObjectType);
        player.AddMember("name", rapidjson::Value(jsonDoc["name"], allocator).Move(), allocator);
        player.AddMember("stats", stats, allocator);

        rapidjson::Value inventory(rapidjson::kArrayType);
        getInventory(jsonDoc, inventory, allocator);
        player.AddMember("inventory", inventory.Move(), allocator);
        player.AddMember("xp", jsonDoc["xp"].GetInt(), allocator);

        resDoc.AddMember("player", player, allocator);
        return true;
    }

private:
    /**
     * @brief Builds a v2 inventory array from a v1 document's inventory.
     *
     * Aggregates duplicate item IDs from the v1 flat inventory array into
     * id/count pair objects for the v2 schema, and writes them into `inventory`.
     *
     * @param jsonDoc   The source v1 RapidJSON document containing the flat inventory array.
     * @param inventory Output RapidJSON array value to populate with id/count objects.
     * @param allocator The RapidJSON allocator to use when constructing new values.
     */
    void getInventory(const rapidjson::Document& jsonDoc, rapidjson::Value &inventory,
        rapidjson::Document::AllocatorType &allocator) {
        // prep the array for migration of inventory items
        std::unordered_map<std::string, int> items = {};

        auto v1Inventory = jsonDoc["inventory"].GetArray();

        for (auto itr = v1Inventory.Begin(); itr != v1Inventory.End(); ++itr) {
            if (items.find(itr->GetString()) == items.end()) {
                items[itr->GetString()] = 1;
            }
            else {
                items[itr->GetString()]++;
            }
        }

        // build the inventory array for the new JSON structure
        for (const auto& item : items) {
            rapidjson::Value newItem(rapidjson::kObjectType);
            newItem.AddMember("id", rapidjson::Value(item.first.c_str(), allocator ).Move(), allocator);
            newItem.AddMember("count", item.second, allocator);
            inventory.PushBack(newItem, allocator);
        }
    }
};

class JSONMigration2to3 : public JSONMigrator {
public:
    /**
     * @brief Migrates a v2 JSON document to the v3 schema.
     *
     * Restructures the v2 player object to the v3 layout: adds a generated
     * player `id`, promotes `xp` out of stats, and converts the v2 inventory
     * array into a flat key/value `bag` object sorted by item ID.
     *
     * @param jsonDoc The source v2 RapidJSON document.
     * @param resDoc  Output document that receives the migrated v3 result.
     * @return true  If migration completed successfully.
     * @return false If migration failed.
     */
    bool migrate(const rapidjson::Document& jsonDoc, rapidjson::Document& resDoc) {
        auto origPlayerObj = jsonDoc["player"].GetObject();
        auto origStatsObj = origPlayerObj["stats"].GetObject();

        // Get the allocator from the document
        rapidjson::Document::AllocatorType& allocator = resDoc.GetAllocator();

        resDoc.AddMember("version", 3, resDoc.GetAllocator());

        rapidjson::Value player(rapidjson::kObjectType);
        rapidjson::Value bag(rapidjson::kObjectType);

        getBag(jsonDoc, bag, allocator);

        // stats object
        rapidjson::Value stats(rapidjson::kObjectType);
        stats.AddMember("hp", origStatsObj["hp"].GetInt(), allocator);
        stats.AddMember("mp", origStatsObj["mp"].GetInt(), allocator);

        // player Object
        player.AddMember("name", rapidjson::Value(origPlayerObj["name"], allocator).Move(), allocator);
        player.AddMember("id", rapidjson::Value(("player:" + std::string(origPlayerObj["name"].GetString())).c_str(), allocator).Move(), allocator);
        player.AddMember("stats", stats, allocator);
        player.AddMember("bag", bag, allocator);
        player.AddMember("xp", origPlayerObj["xp"].GetInt(), allocator);

        resDoc.AddMember("player", player, allocator);

        return true;
    }

private:
    /**
     * @brief Builds a v3 bag object from a v2 document's inventory array.
     *
     * Reads each id/count entry from the v2 inventory array and writes them
     * as direct key/value members of `bag`, sorted alphabetically by item ID.
     *
     * @param jsonDoc   The source v2 RapidJSON document containing the inventory array.
     * @param bag       Output RapidJSON object value to populate with item key/count pairs.
     * @param allocator The RapidJSON allocator to use when constructing new values.
     */
    void getBag(const rapidjson::Document& jsonDoc, rapidjson::Value& bag,
        rapidjson::Document::AllocatorType& allocator) {

        // prep the array for migration of inventory items
        std::unordered_map<std::string, int> items = {};

        auto v1Inventory = jsonDoc["player"]["inventory"].GetArray();

        for (auto itr = v1Inventory.Begin(); itr != v1Inventory.End(); ++itr) {
			std::string id = (*itr)["id"].GetString();
			int count = (*itr)["count"].GetInt();
            items[id] = count;
        }

        // build the bag for the new JSON structure
        std::vector<std::pair<std::string, int>> sorted_items(items.begin(), items.end());
        std::sort(sorted_items.begin(), sorted_items.end(), [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
            return a.first < b.first;
            });
        for (const auto& pair : sorted_items) {
            bag.AddMember(rapidjson::Value(pair.first.c_str(), allocator).Move(), pair.second, allocator);
        }
    }
};


/**
 * @brief Migrates a parsed JSON document to the specified target version.
 *
 * Iterates through each version step from the document's current version up
 * to `toVersion`, applying the appropriate migrator at each step.
 *
 * @param jsonDoc   The source RapidJSON document to migrate.
 * @param resDoc    Output document that receives the fully migrated result.
 * @param toVersion The target schema version to migrate to.
 * @return true  If migration completed successfully to the target version.
 * @return false If the document lacks a valid "version" field, the current version
 *               already exceeds `toVersion`, or no migrator exists for a required step.
 */
bool JSONMigration::Migrate(const rapidjson::Document &jsonDoc, rapidjson::Document &resDoc, int toVersion) {
    const std::string versionKey = "version";
    rapidjson::Document curDoc;
    rapidjson::Document nextDoc;
    nextDoc.SetObject();

    if (!jsonDoc.HasMember(versionKey.c_str()) || !jsonDoc[versionKey.c_str()].IsInt()) {
        return false;
    }
    int version = jsonDoc[versionKey.c_str()].GetInt();

    if (version > toVersion) {
        return false;
    }
    
    curDoc.CopyFrom(jsonDoc, curDoc.GetAllocator());
    while (version < toVersion) {
        auto migrator = getMigrator(version++);
        if (!migrator) {
            return false;
        }
        migrator->migrate(curDoc, nextDoc);
        curDoc.CopyFrom(nextDoc, curDoc.GetAllocator());
        nextDoc.SetObject();
    }
    resDoc.CopyFrom(curDoc, resDoc.GetAllocator());
    return true;
}

/**
 * @brief Loads, validates, and migrates a JSON file to the specified target version.
 *
 * Resolves the file path relative to the source file location, loads the JSON,
 * validates it against its schema, and then delegates to
 * Migrate(const rapidjson::Document&, rapidjson::Document&, int).
 *
 * @param fileLoc   Relative path to the source JSON file.
 * @param resDoc    Output document that receives the fully migrated result.
 * @param toVersion The target schema version to migrate to.
 * @return true  If the file was loaded, validated, and migrated successfully.
 * @return false If the file could not be loaded, failed validation, or migration failed.
 */
bool JSONMigration::Migrate(std::string fileLoc, rapidjson::Document &resDoc, int toVersion) {
    JSONValidation val;

    auto srcJson = loadJson(fileLoc);
    if (srcJson.has_value() && val.validate(srcJson.value())) {
        Migrate(srcJson.value(), resDoc, toVersion);
    }
    return false;
}

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
std::unique_ptr<JSONMigrator> JSONMigration::getMigrator(int version) {
    static JSONMigration1to2 migrator1to2;
    static JSONMigration2to3 migrator2to3;

    switch (version) {
    case 1:
        return std::make_unique<JSONMigration1to2>();
    case 2:
        return std::make_unique<JSONMigration2to3>();
    default:
        return nullptr;
    }
}