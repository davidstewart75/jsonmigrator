# jsonmigrator
This is a tool for migrating JSON data from one schema to another. It allows you to define a migration plan that specifies how to transform your JSON data from the old schema to the new schema.

## Installation
Unzip the file in the desired location. It can be run directly through the save_migrator app at the root of the folder.

The application can be built by building the provided .sln file in the .zip. It has been tested against x64 Debug and Release but will need to be rebuilt against those targets to run. The build will generate an .exe in the x64/[Debug|Release] directory at the root of the project. It can also be directly run through visual studio.

## Usage
save_migrator validate --in → prints OK or errors. Exit codes: 0 valid, 2 invalid.
save_migrator migrate --in --out --to <2|3>

## Notes
In building the project I selected rapidjson as the JSON library. It is useful for this problem in multiple ways:
	1) it is a header-only library, so it is easy to include in the project without needing to worry about linking against a separate library.
	2) it provides a simple and efficient API for parsing and manipulating JSON data, which makes it easy to implement the migration logic.
	3) it is widely used and well-maintained, which means that it is likely to be reliable and performant for this use case.
	4) rapidjson is known for having a stable output format, which is important for this project since we want to ensure that the migrated JSON data is consistent and predictable.
	5) rapidjson provides built in validation through schemas.

I also established a separate test project using cppunit. Right now it does some basic validation though a deeper test suite would be needed if this were actually deployed
in production. I also added some basic error handling, but again this would need to be expanded for a production environment.

## Future Improvements
There are a few areas where this project could be improved in the future:
	1) More comprehensive testing: As mentioned earlier, the current test suite is quite basic. Adding more tests to cover a wider range of scenarios would help ensure that the migration logic is robust and reliable.
	2) Better error handling: While some basic error handling is currently implemented, there are likely many edge cases that are not currently accounted for. Adding more comprehensive error handling would help ensure that the tool can handle a wider range of input data and migration scenarios.
	3) Expanded validation: Currently, the validation logic is quite basic and only checks for a few specific conditions. Expanding the validation logic to cover a wider range of scenarios would help ensure that the tool can catch more potential issues with the input data before attempting to migrate it.
	4) Cleaner messaging and logging throughout

