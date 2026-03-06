#include "pch.h"
#include "CppUnitTest.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "../GEHCTest/source/JSONMigration.cpp"
#include <optional>
#include <algorithm>
#include <utility>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace GEHCTests
{
    // ---------------------------------------------------------------------------
    // Helpers
    // ---------------------------------------------------------------------------

    /// Returns the absolute path to a sample JSON file relative to this source
    /// file's directory, so tests work regardless of working directory.
    static std::string samplePath(const std::string& filename)
    {
        // __FILE__ is the full path to this .cpp at compile time.
        std::string thisFile(__FILE__);
        std::string dir = thisFile.substr(0, thisFile.find_last_of("\\/") + 1);
        return dir + "\\fixtures\\" + filename;
    }

    static std::string goldenPath(const std::string& filename)
    {
        // __FILE__ is the full path to this .cpp at compile time.
        std::string thisFile(__FILE__);
        std::string dir = thisFile.substr(0, thisFile.find_last_of("\\/") + 1);
        return dir + "\\goldens\\" + filename;
	}

    static std::string getJsonStr(rapidjson::Document& doc)
    {
        // Use PrettyWriter for formatted output
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> prettyWriter(buffer);
        doc.Accept(prettyWriter);

        return buffer.GetString();
	}

    // =========================================================================
    // JSONValidation — validate(path) overload
    // =========================================================================
    TEST_CLASS(JSONMigrationTests)
    {
    public:
        TEST_METHOD(MigrateV1ToV2)
        {
            JSONMigration migration;

			rapidjson::Document doc;

			std::optional<rapidjson::Document> expectedDoc = loadJson(goldenPath("ConvertedV1toV2Sample.json"));
			migration.Migrate(samplePath("v1Sample.json"), doc, 2);

			JSONValidation validation;
			Assert::IsTrue(validation.validate(doc), L"migrated JSON is not valid");

			Assert::AreEqual(2, doc["version"].GetInt(), L"v1Sample.json should be migrated to version 2");
			Assert::IsTrue(doc==expectedDoc, L"v1Sample.json should be migrated to match the expected v2 structure");
        }

        TEST_METHOD(MigrateV1ToV3)
        {
            JSONMigration migration;

            rapidjson::Document doc;

            std::optional<rapidjson::Document> expectedDoc = loadJson(goldenPath("ConvertedV1toV3Sample.json"));
            migration.Migrate(samplePath("v1Sample.json"), doc, 3);

            auto test1 = getJsonStr(doc);
			auto test2 = getJsonStr(expectedDoc.value());

            auto result = std::mismatch(test1.begin(), test1.end(), test2.begin());

            JSONValidation validation;
            Assert::IsTrue(validation.validate(doc), L"migrated JSON is not valid");

            Assert::AreEqual(3, doc["version"].GetInt(), L"v1Sample.json should be migrated to version 2");
            Assert::IsTrue(doc == expectedDoc, L"v1Sample.json should be migrated to match the expected v2 structure");
        }

    };
}