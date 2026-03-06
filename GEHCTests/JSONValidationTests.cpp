#include "pch.h"
#include "CppUnitTest.h"

#include "../GEHCTest/source/JSONValidation.cpp"
// Pull in the headers and sources under test directly so the test project
// does not need a separate lib target.
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
        return dir + "..\\GEHCTest\\SampleJSON\\" + filename;
    }

    // =========================================================================
    // JSONValidation — validate(path) overload
    // =========================================================================
    TEST_CLASS(JSONValidationFileTests)
    {
    public:

        // ---------------------------------------------------------------------
        // v1Sample.json — valid v1 document
        // ---------------------------------------------------------------------
        TEST_METHOD(ValidateFile_V1Sample_ReturnsTrue)
        {
            JSONValidation validator;
            bool result = validator.validate(samplePath("v1Sample.json"));
            Assert::IsTrue(result, L"v1Sample.json should pass v1 schema validation");
        }

        // ---------------------------------------------------------------------
        // v2Sample.json — valid v2 document
        // ---------------------------------------------------------------------
        TEST_METHOD(ValidateFile_V2Sample_ReturnsTrue)
        {
            JSONValidation validator;
            bool result = validator.validate(samplePath("v2Sample.json"));
            Assert::IsTrue(result, L"v2Sample.json should pass v2 schema validation");
        }

        // ---------------------------------------------------------------------
        // v3Sample.json — valid v3 document
        // ---------------------------------------------------------------------
        TEST_METHOD(ValidateFile_V3Sample_ReturnsTrue)
        {
            JSONValidation validator;
            bool result = validator.validate(samplePath("v3Sample.json"));
            Assert::IsTrue(result, L"v3Sample.json should pass v3 schema validation");
        }

        // ---------------------------------------------------------------------
        // Non-existent file — should return false gracefully
        // ---------------------------------------------------------------------
        TEST_METHOD(ValidateFile_MissingFile_ReturnsFalse)
        {
            JSONValidation validator;
            bool result = validator.validate("does_not_exist.json");
            Assert::IsFalse(result, L"A missing file should return false");
        }
    };

    // =========================================================================
    // JSONValidation — validate(Document&) overload
    // =========================================================================
    TEST_CLASS(JSONValidationDocumentTests)
    {
    public:

        // ---------------------------------------------------------------------
        // v1 inline document — valid
        // ---------------------------------------------------------------------
        TEST_METHOD(ValidateDocument_ValidV1_ReturnsTrue)
        {
            const char* json = R"({
                "version": 1,
                "name": "Test Player",
                "xp": 10,
                "hp": 100,
                "inventory": ["sword", "shield"]
            })";

            rapidjson::Document doc;
            doc.Parse(json);
            Assert::IsFalse(doc.HasParseError(), L"Inline v1 JSON should parse without error");

            JSONValidation validator;
            Assert::IsTrue(validator.validate(doc), L"Valid v1 document should pass validation");
        }

        // ---------------------------------------------------------------------
        // v2 inline document — valid
        // ---------------------------------------------------------------------
        TEST_METHOD(ValidateDocument_ValidV2_ReturnsTrue)
        {
            const char* json = R"({
                "version": 2,
                "player": {
                    "name": "Test Player",
                    "stats": { "hp": 50, "mp": 30 },
                    "inventory": [{ "id": "sword", "count": 1 }],
                    "xp": 5
                }
            })";

            rapidjson::Document doc;
            doc.Parse(json);
            Assert::IsFalse(doc.HasParseError(), L"Inline v2 JSON should parse without error");

            JSONValidation validator;
            Assert::IsTrue(validator.validate(doc), L"Valid v2 document should pass validation");
        }

        // ---------------------------------------------------------------------
        // v3 inline document — valid
        // ---------------------------------------------------------------------
        TEST_METHOD(ValidateDocument_ValidV3_ReturnsTrue)
        {
            const char* json = R"({
                "version": 3,
                "player": {
                    "name": "Test Player",
                    "id": "player:Test Player",
                    "stats": { "hp": 50, "mp": 30 },
                    "xp": 5,
                    "bag": { "sword": 1 }
                }
            })";

            rapidjson::Document doc;
            doc.Parse(json);
            Assert::IsFalse(doc.HasParseError(), L"Inline v3 JSON should parse without error");

            JSONValidation validator;
            Assert::IsTrue(validator.validate(doc), L"Valid v3 document should pass validation");
        }

        // ---------------------------------------------------------------------
        // Missing version field — should return false
        // ---------------------------------------------------------------------
        TEST_METHOD(ValidateDocument_MissingVersion_ReturnsFalse)
        {
            const char* json = R"({ "name": "No Version" })";

            rapidjson::Document doc;
            doc.Parse(json);

            JSONValidation validator;
            Assert::IsFalse(validator.validate(doc), L"Document without version should fail validation");
        }

        // ---------------------------------------------------------------------
        // Unsupported version number — should return false
        // ---------------------------------------------------------------------
        TEST_METHOD(ValidateDocument_UnsupportedVersion_ReturnsFalse)
        {
            const char* json = R"({ "version": 99, "player": {} })";

            rapidjson::Document doc;
            doc.Parse(json);

            JSONValidation validator;
            Assert::IsFalse(validator.validate(doc), L"Document with unsupported version should fail validation");
        }

        // ---------------------------------------------------------------------
        // v2 document with wrong type for stats.hp — should return false
        // ---------------------------------------------------------------------
        TEST_METHOD(ValidateDocument_V2WrongHpType_ReturnsFalse)
        {
            const char* json = R"({
                "version": 2,
                "player": {
                    "name": "Test Player",
                    "stats": { "hp": "notANumber", "mp": 30 },
                    "inventory": [],
                    "xp": 5
                }
            })";

            rapidjson::Document doc;
            doc.Parse(json);

            JSONValidation validator;
            Assert::IsFalse(validator.validate(doc), L"v2 document with string hp should fail validation");
        }

        // ---------------------------------------------------------------------
        // v3 document missing required 'bag' field — should return false
        // ---------------------------------------------------------------------
        TEST_METHOD(ValidateDocument_V3MissingBag_ReturnsFalse)
        {
            const char* json = R"({
                "version": 3,
                "player": {
                    "name": "Test Player",
                    "id": "player:Test Player",
                    "stats": { "hp": 50, "mp": 30 },
                    "xp": 5
                }
            })";

            rapidjson::Document doc;
            doc.Parse(json);

            JSONValidation validator;
            Assert::IsFalse(validator.validate(doc), L"v3 document missing 'bag' should fail validation");
        }
    };
}