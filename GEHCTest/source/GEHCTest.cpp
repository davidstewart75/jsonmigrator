// GEHCTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <fstream>
#include "../headers/JSONValidation.h"
#include "../headers/JSONMigration.h"
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"

// Exit codes
static const int EXIT_VALID   = 0;
static const int EXIT_USAGE   = 1;
static const int EXIT_INVALID = 2;

/**
 * @brief Prints usage instructions to stderr.
 *
 * Displays the available command-line argument combinations for
 * validation-only and validation-with-migration modes.
 *
 * @param programName The name of the running executable (argv[0]).
 */
static void printUsage(const std::string& programName) {
    std::cout << "Usage:\n"
              << "  Validate only:  " << programName << " --in <file>\n"
              << "  Validate & migrate: " << programName << " --in <file> --out <file> --to <2|3>\n";
}

static int doValidation(int argc, char* argv[]) {
    std::string inFile;
    // --- Parse arguments ---
    for (int i = 2; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg == "--in") {
            if (i + 1 >= argc) {
                std::cout << "Error: --in requires a file path argument.\n";
                printUsage(argv[0]);
                return EXIT_USAGE;
            }
            inFile = argv[++i];
        }
        else {
            std::cout << "Error: Unknown argument: " << arg << "\n";
            printUsage(argv[0]);
            return EXIT_USAGE;
        }
    }
    // --in is mandatory
    if (inFile.empty()) {
        std::cout << "Error: --in <file> is required.\n";
        printUsage(argv[0]);
        return EXIT_USAGE;
    }
    JSONValidation validator;
    if (!validator.validate(inFile)) {
        std::cout << "Validation failed for file: " << inFile << "\n";
        return EXIT_INVALID;
    }
    std::cout << "OK";
    return EXIT_VALID;
}

static int doMigration(int argc, char* argv[]) {
    std::string inFile;
    std::string outFile;
    int         targetVersion = -1;
    // --- Parse arguments ---
    for (int i = 2; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg == "--in") {
            if (i + 1 >= argc) {
                std::cout << "Error: --in requires a file path argument.\n";
                printUsage(argv[0]);
                return EXIT_USAGE;
            }
            inFile = argv[++i];
        }
        else if (arg == "--out") {
            if (i + 1 >= argc) {
                std::cout << "Error: --out requires a file path argument.\n";
                printUsage(argv[0]);
                return EXIT_USAGE;
            }
            outFile = argv[++i];
        }
        else if (arg == "--to") {
            if (i + 1 >= argc) {
                std::cout << "Error: --to requires a version argument (2 or 3).\n";
                printUsage(argv[0]);
                return EXIT_USAGE;
            }
            std::string verStr(argv[++i]);
            if (verStr == "2")      targetVersion = 2;
            else if (verStr == "3") targetVersion = 3;
            else {
                std::cout << "Error: --to only accepts 2 or 3, got: " << verStr << "\n";
                printUsage(argv[0]);
                return EXIT_USAGE;
            }
        }
        else {
            std::cout << "Error: Unknown argument: " << arg << "\n";
            printUsage(argv[0]);
            return EXIT_USAGE;
        }
    }
    // --in is mandatory
    if (inFile.empty()) {
        std::cout << "Error: --in <file> is required.\n";
        printUsage(argv[0]);
        return EXIT_USAGE;
    }
    // --out and --to must both be present or both absent
    if (!outFile.empty() && targetVersion == -1) {
        std::cout << "Error: --out requires --to <2|3>.\n";
        printUsage(argv[0]);
        return EXIT_USAGE;
    }

	JSONMigration migrator;
	rapidjson::Document migratorDoc;
	migrator.Migrate(inFile, migratorDoc, targetVersion);

    errno_t err;
    FILE* filepoint;
    if ((err = fopen_s(&filepoint, outFile.c_str(), "w")) != 0) {
        std::cout << "Error: Cannot open output file for writing: " << outFile << "\n";
		return EXIT_USAGE;
    }

    char writeBuffer[65536]; // A buffer for efficient writing
    rapidjson::FileWriteStream os(filepoint, writeBuffer, sizeof(writeBuffer));

    rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
    migratorDoc.Accept(writer);

    std::fclose(filepoint);

    return EXIT_VALID;
}

/**
 * @brief Program entry point. Parses arguments, validates, and optionally migrates a JSON file.
 *
 * Accepts the following command-line arguments:
 * - `--in <file>`   (required) Path to the input JSON file to validate.
 * - `--out <file>`  (optional) Path to write the migrated output JSON file.
 * - `--to <2|3>`    (optional) Target schema version to migrate to (must accompany `--out`).
 *
 * If only `--in` is provided, the file is validated against its schema.
 * If `--out` and `--to` are also provided, the file is additionally migrated.
 * `--out` and `--to` must always be specified together.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line argument strings.
 * @return EXIT_VALID   (0) on success.
 * @return EXIT_USAGE   (1) if arguments are invalid or missing.
 * @return EXIT_INVALID (2) if the input JSON fails schema validation.
 */
int main(int argc, char* argv[])
{
    std::string inFile;
    std::string outFile;
    int         targetVersion = -1;

    if (argc > 0) {
        std::string firstArg(argv[1]);
		int retVal = EXIT_USAGE;

        if (firstArg == "validate") {
            retVal = doValidation(argc, argv);
        }
        else if (firstArg == "migrate") {
            retVal = doMigration(argc, argv);
        }
        else {
			std::cout << "Error: First argument must be 'validate' or 'migrate'.\n";
            printUsage(argv[0]);
        }

        return retVal;
	}
}
