#include <gtest/gtest.h>

#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/ADT/IntrusiveRefCntPtr.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Frontend/ASTUnit.h>

#include <unordered_map>
#include <memory>
#include <string>
#include <algorithm>

#include <SASTGeneratorActionFactory.h>
#include <SASTGeneratorAction.h>
#include <SAST.h>
#include <JSONFormatPlugin.h>

#include <sstream>
#include <streambuf>
#include <iostream>

struct OutputCapture {
	std::stringstream outBuffer;
	std::stringstream errBuffer;

	std::streambuf* oldOut = nullptr;
	std::streambuf* oldErr = nullptr;

	void start() {
		oldOut = std::cout.rdbuf(outBuffer.rdbuf());
		oldErr = std::cerr.rdbuf(errBuffer.rdbuf());
	}

	void stop() {
		std::cout.rdbuf(oldOut);
		std::cerr.rdbuf(oldErr);
	}

	std::string getStdOut() const { return outBuffer.str(); }
	std::string getStdErr() const { return errBuffer.str(); }
};


using namespace GenTools::GenSerialize;
using namespace clang::tooling;

using namespace GenTools::GenSerialize;

class JSONFormatPluginTest : public ::testing::Test {
protected:
	std::unordered_map<std::string, std::vector<std::shared_ptr<SASTNode>>> globalSASTTrees;
	std::unordered_map<std::string, std::shared_ptr<SASTNode>> globalSASTMap;

	static void AssertCodeEqual(const std::string& actual, const std::string& expected) {
		std::string normA, normB;
		std::remove_copy(actual.begin(), actual.end(), std::back_inserter(normA), '\r');
		std::remove_copy(expected.begin(), expected.end(), std::back_inserter(normB), '\r');
		EXPECT_EQ(normA, normB);
	}


	void GenerateSASTFromSources(const std::vector<std::pair<std::string, std::string>>& virtualFiles) {
		// Extract paths
		std::vector<std::string> sourcePaths;
		for (const auto& [filename, _] : virtualFiles) {
			sourcePaths.push_back(filename);
		}

		std::vector<std::string> compilationArgs = {
			"-xc++", "-std=c++20", "-fsyntax-only", "-Wno-pragma-once-outside-header", "-nostdinc++"
		};

		auto Compilations = std::make_unique<FixedCompilationDatabase>(".", compilationArgs);

		// === Create Virtual FS ===
		using namespace llvm;
		using namespace clang::tooling;

		auto RealFS = vfs::getRealFileSystem();
		auto InMemFS = llvm::makeIntrusiveRefCnt<vfs::InMemoryFileSystem>();

		// Add SerializationMacros.h
		auto EmptyBuffer = llvm::MemoryBuffer::getMemBuffer("", "EmptyBuffer");
		InMemFS->addFile("SerializationMacros.h", 0, std::move(EmptyBuffer));

		// Add dummy generated.h includes
		for (const auto& [filename, _] : virtualFiles) {
			std::filesystem::path path(filename);
			std::string genHeader = path.filename().replace_extension(".generated.h").string();
			auto EmptyBufferGen = llvm::MemoryBuffer::getMemBuffer("", "EmptyBufferGen");
			InMemFS->addFile(genHeader, 0, std::move(EmptyBufferGen));
		}

		auto OverlayFS = llvm::makeIntrusiveRefCnt<vfs::OverlayFileSystem>(InMemFS);
		OverlayFS->pushOverlay(RealFS);

		// === Set up ClangTool with overlay FS ===
		SASTGeneratorActionFactory factory;
		ClangTool tool(*Compilations, sourcePaths, std::make_shared<clang::PCHContainerOperations>(), OverlayFS);

		for (const auto& [filename, content] : virtualFiles) {
			tool.mapVirtualFile(filename, content);
		}

		OutputCapture capture;
		capture.start();

		int result = tool.run(&factory);

		capture.stop();

		// If the tool fails, show diagnostics
		if (result != 0) {
			std::cerr << "ClangTool failed\n";
			std::cerr << "Captured stdout:\n" << capture.getStdOut();
			std::cerr << "Captured stderr:\n" << capture.getStdErr();
		}

		ASSERT_EQ(result, 0) << "Clang tool run failed";

		// Merge results
		factory.MergeResults(globalSASTTrees, globalSASTMap);
	}
};


TEST_F(JSONFormatPluginTest, HandlesVectorOfInts) {
	GenerateSASTFromSources({
		{"VectorType.h", R"cpp(
			#pragma once
			#include <vector>
			#include "SerializationMacros.h"

			class SERIALIZABLE(JSON) VectorType {
				SERIALIZE_FIELD
				std::vector<int> intVec;

				GENERATED_SERIALIZATION_BODY();
			};
		)cpp"}
		});

	ASSERT_NE(globalSASTMap.find("VectorType"), globalSASTMap.end());
	JSONFormatPlugin plugin;
	std::string code = plugin.GenerateCode(globalSASTMap["VectorType"]);
	ASSERT_FALSE(code.empty());

	const char* expected = R"cpp(#include <fstream>

#include <JSONStructure.h>

static void JSONSerialize(JSONObject& jsonReceiver, const VectorType& objSource)
{
	{
		JSONArray intVec_json;
		for(const auto& item_1 : objSource.intVec)
		{
			intVec_json.AddMember(JSONNumber(static_cast<double>(item_1)));
		}
		jsonReceiver.AddMember("intVec", std::move(intVec_json));
	}

}

static void JSONDeserialize(VectorType& objReceiver, const JSONObject& jsonSource)
{
	{
		for(const auto& item_1 : jsonSource.GetMember("intVec").as<JSONArray>().GetItems())
		{
			auto& elem_1 = objReceiver.intVec.emplace_back();
			elem_1 = item_1.as<JSONNumber>().value;
		}
	}

}

static void JSONSerialize(std::ostream& osReceiver, const VectorType& objSource)
{
	JSONStructure jsonRep;
	{
		JSONArray intVec_json;
		for(const auto& item_1 : objSource.intVec)
		{
			intVec_json.AddMember(JSONNumber(static_cast<double>(item_1)));
		}
		jsonRep.AddMember("intVec", std::move(intVec_json));
	}

	osReceiver << jsonRep.Stringify();
}

static void JSONDeserialize(VectorType& objReceiver, const std::istream& isSource)
{
	JSONStructure jsonRep = JSONStructure::Parse(isSource);
	{
		for(const auto& item_1 : jsonRep.GetMember("intVec").as<JSONArray>().GetItems())
		{
			auto& elem_1 = objReceiver.intVec.emplace_back();
			elem_1 = item_1.as<JSONNumber>().value;
		}
	}

}
)cpp";

	AssertCodeEqual(code, expected);
}

TEST_F(JSONFormatPluginTest, HandlesUnorderedMapOfPODs) {
	GenerateSASTFromSources({
		{"Header.h", R"cpp(
			#pragma once
			#include <unordered_map>
			#include <string>
			#include "SerializationMacros.h"
			#include "Header.generated.h"
			struct SERIALIZABLE_POD POD {
				int a;
				std::string b;
			};

			class SERIALIZABLE(JSON) MyType {
				SERIALIZE_FIELD
				std::unordered_map<int, POD> podMap;

				GENERATED_SERIALIZATION_BODY();
			};
		)cpp"}
		});

	auto it = globalSASTMap.find("MyType");
	ASSERT_NE(it, globalSASTMap.end());

	JSONFormatPlugin plugin;
	std::string code = plugin.GenerateCode(it->second);
	ASSERT_FALSE(code.empty());

	const char* expected = R"cpp(#include <fstream>

#include <JSONStructure.h>

static void JSONSerialize(JSONObject& jsonReceiver, const MyType& objSource)
{
	{
		JSONObject podMap_json;
		for(const auto& [key_1, value_1] : objSource.podMap)
		{
			{
				JSONObject nested_json;
				nested_json.AddMember("a", JSONNumber(static_cast<double>(value_1.a)));
				nested_json.AddMember("b", JSONString(value_1.b));
				podMap_json.AddMember(std::to_string(key_1), std::move(nested_json));
			}
		}
		jsonReceiver.AddMember("podMap", std::move(podMap_json));
	}

}

static void JSONDeserialize(MyType& objReceiver, const JSONObject& jsonSource)
{
	{
		for(const auto& [key_1, value_1] : jsonSource.GetMember("podMap").as<JSONObject>().GetMembers())
		{
			auto& elem_1 = objReceiver.podMap[std::stoi(key_1)];
			elem_1.a = value_1.as<JSONObject>().GetMember("a").as<JSONNumber>().value;
			elem_1.b = value_1.as<JSONObject>().GetMember("b").as<JSONString>().value;
		}
	}

}

static void JSONSerialize(std::ostream& osReceiver, const MyType& objSource)
{
	JSONStructure jsonRep;
	{
		JSONObject podMap_json;
		for(const auto& [key_1, value_1] : objSource.podMap)
		{
			{
				JSONObject nested_json;
				nested_json.AddMember("a", JSONNumber(static_cast<double>(value_1.a)));
				nested_json.AddMember("b", JSONString(value_1.b));
				podMap_json.AddMember(std::to_string(key_1), std::move(nested_json));
			}
		}
		jsonRep.AddMember("podMap", std::move(podMap_json));
	}

	osReceiver << jsonRep.Stringify();
}

static void JSONDeserialize(MyType& objReceiver, const std::istream& isSource)
{
	JSONStructure jsonRep = JSONStructure::Parse(isSource);
	{
		for(const auto& [key_1, value_1] : jsonRep.GetMember("podMap").as<JSONObject>().GetMembers())
		{
			auto& elem_1 = objReceiver.podMap[std::stoi(key_1)];
			elem_1.a = value_1.as<JSONObject>().GetMember("a").as<JSONNumber>().value;
			elem_1.b = value_1.as<JSONObject>().GetMember("b").as<JSONString>().value;
		}
	}

}
)cpp";

	AssertCodeEqual(code, expected);
}

TEST_F(JSONFormatPluginTest, HandlesNestedObjectType)
{
	GenerateSASTFromSources({
	{"NestedObjectType.h", R"cpp(
		#pragma once
		#include <string>
		#include "SerializationMacros.h"
		#include "NestedObjectType.generated.h"

		class SERIALIZABLE(JSON) InnerObject {
			SERIALIZE_FIELD
			int innerVal;

			SERIALIZE_FIELD
			std::string innerText;

			GENERATED_SERIALIZATION_BODY();
		};

		class SERIALIZABLE(JSON) OuterObject {
			SERIALIZE_FIELD
			InnerObject obj;

			GENERATED_SERIALIZATION_BODY();
		};
	)cpp"}
		});

	auto it = globalSASTMap.find("OuterObject");
	ASSERT_NE(it, globalSASTMap.end());

	JSONFormatPlugin plugin;
	std::string code = plugin.GenerateCode(it->second);
	ASSERT_FALSE(code.empty());

	const char* expected = R"cpp(#include <fstream>

#include <JSONStructure.h>

static void JSONSerialize(JSONObject& jsonReceiver, const OuterObject& objSource)
{
	{
		JSONObject obj_json;
		JSONSerialize(obj_json, objSource.obj);
		jsonReceiver.AddMember("obj", std::move(obj_json));
	}

}

static void JSONDeserialize(OuterObject& objReceiver, const JSONObject& jsonSource)
{
	JSONDeserialize(objReceiver.obj, jsonSource.GetMember("obj").as<JSONObject>());

}

static void JSONSerialize(std::ostream& osReceiver, const OuterObject& objSource)
{
	JSONStructure jsonRep;
	{
		JSONObject obj_json;
		JSONSerialize(obj_json, objSource.obj);
		jsonRep.AddMember("obj", std::move(obj_json));
	}

	osReceiver << jsonRep.Stringify();
}

static void JSONDeserialize(OuterObject& objReceiver, const std::istream& isSource)
{
	JSONStructure jsonRep = JSONStructure::Parse(isSource);
	JSONDeserialize(objReceiver.obj, jsonRep.GetMember("obj").as<JSONObject>());

}
)cpp";

	AssertCodeEqual(code, expected);
}

TEST_F(JSONFormatPluginTest, HandlesCStyleStaticArray)
{
	GenerateSASTFromSources({
	{"StaticArrayType.h", R"cpp(
		#pragma once
		#include <string>
		#include "SerializationMacros.h"
		#include "StaticArrayType.generated.h"

		class SERIALIZABLE(JSON) StaticArrayType {
			SERIALIZE_FIELD
			STATIC_ARRAY
			int fixedArray[3];

			GENERATED_SERIALIZATION_BODY();
		};
	)cpp"}
		});

	auto it = globalSASTMap.find("StaticArrayType");
	ASSERT_NE(it, globalSASTMap.end());

	const auto& node = it->second;
	ASSERT_EQ(node->fields.size(), 1u);
	EXPECT_EQ(node->fields[0].type, SASTType::Array);
	EXPECT_EQ(node->fields[0].originalTypeName, "int[3]");

	JSONFormatPlugin plugin;
	std::string code = plugin.GenerateCode(node);
	ASSERT_FALSE(code.empty());

	const char* expected = R"cpp(#include <fstream>

#include <JSONStructure.h>

static void JSONSerialize(JSONObject& jsonReceiver, const StaticArrayType& objSource)
{
	{
		JSONArray fixedArray_json;
		for(size_t i_1 = 0; i_1 < sizeof(objSource.fixedArray) / sizeof(objSource.fixedArray[0]); i_1++)
		{
			fixedArray_json.AddMember(JSONNumber(static_cast<double>(objSource.fixedArray[i_1])));
		}
		jsonReceiver.AddMember("fixedArray", std::move(fixedArray_json));
	}

}

static void JSONDeserialize(StaticArrayType& objReceiver, const JSONObject& jsonSource)
{
	{
		JSONArray fixedArray_json = jsonSource.GetMember("fixedArray").as<JSONArray>();
		for(size_t i_1 = 0; i_1 < sizeof(objReceiver.fixedArray) / sizeof(objReceiver.fixedArray[0]); i_1++)
		{
			objReceiver.fixedArray[i_1] = fixedArray_json[i_1].as<JSONNumber>().value;
		}
	}

}

static void JSONSerialize(std::ostream& osReceiver, const StaticArrayType& objSource)
{
	JSONStructure jsonRep;
	{
		JSONArray fixedArray_json;
		for(size_t i_1 = 0; i_1 < sizeof(objSource.fixedArray) / sizeof(objSource.fixedArray[0]); i_1++)
		{
			fixedArray_json.AddMember(JSONNumber(static_cast<double>(objSource.fixedArray[i_1])));
		}
		jsonRep.AddMember("fixedArray", std::move(fixedArray_json));
	}

	osReceiver << jsonRep.Stringify();
}

static void JSONDeserialize(StaticArrayType& objReceiver, const std::istream& isSource)
{
	JSONStructure jsonRep = JSONStructure::Parse(isSource);
	{
		JSONArray fixedArray_json = jsonRep.GetMember("fixedArray").as<JSONArray>();
		for(size_t i_1 = 0; i_1 < sizeof(objReceiver.fixedArray) / sizeof(objReceiver.fixedArray[0]); i_1++)
		{
			objReceiver.fixedArray[i_1] = fixedArray_json[i_1].as<JSONNumber>().value;
		}
	}

}
)cpp";

	AssertCodeEqual(code, expected);
}

TEST_F(JSONFormatPluginTest, HandlesDynamicArrayWithLengthVar)
{
	GenerateSASTFromSources({
	{"DynamicArrayType.h", R"cpp(
		#pragma once
		#include "SerializationMacros.h"
		#include "DynamicArrayType.generated.h"

		class SERIALIZABLE(JSON) DynamicArrayType {
			SERIALIZE_FIELD
			DYNAMIC_ARRAY(arraySize)
			int* dynamicArray;

			int arraySize;

			GENERATED_SERIALIZATION_BODY();
		};
	)cpp"}
		});

	auto it = globalSASTMap.find("DynamicArrayType");
	ASSERT_NE(it, globalSASTMap.end());

	const auto& node = it->second;
	ASSERT_EQ(node->fields.size(), 1u);
	EXPECT_EQ(node->fields[0].type, SASTType::Dynamic_Array);
	EXPECT_EQ(node->fields[0].lengthVar, "arraySize");
	EXPECT_EQ(node->fields[0].originalTypeName, "int *");

	JSONFormatPlugin plugin;
	std::string code = plugin.GenerateCode(node);
	ASSERT_FALSE(code.empty());

	const char* expected = R"cpp(#include <fstream>

#include <JSONStructure.h>

static void JSONSerialize(JSONObject& jsonReceiver, const DynamicArrayType& objSource)
{
	{
		JSONArray dynamicArray_json;
		for(size_t i_1 = 0; i_1 < arraySize; i_1++)
		{
			dynamicArray_json.AddMember(JSONNumber(static_cast<double>(objSource.dynamicArray[i_1])));
		}
		jsonReceiver.AddMember("dynamicArray", std::move(dynamicArray_json));
	}

}

static void JSONDeserialize(DynamicArrayType& objReceiver, const JSONObject& jsonSource)
{
	{
		JSONArray dynamicArray_json = jsonSource.GetMember("dynamicArray").as<JSONArray>();
		if (objReceiver.dynamicArray) delete[] objReceiver.dynamicArray;
		objReceiver.dynamicArray = new int[dynamicArray_json.GetItems().size()];
		arraySize = dynamicArray_json.GetItems().size();
		for(size_t i_1 = 0; i_1 < arraySize; i_1++)
		{
			objReceiver.dynamicArray[i_1] = dynamicArray_json[i_1].as<JSONNumber>().value;
		}
	}

}

static void JSONSerialize(std::ostream& osReceiver, const DynamicArrayType& objSource)
{
	JSONStructure jsonRep;
	{
		JSONArray dynamicArray_json;
		for(size_t i_1 = 0; i_1 < arraySize; i_1++)
		{
			dynamicArray_json.AddMember(JSONNumber(static_cast<double>(objSource.dynamicArray[i_1])));
		}
		jsonRep.AddMember("dynamicArray", std::move(dynamicArray_json));
	}

	osReceiver << jsonRep.Stringify();
}

static void JSONDeserialize(DynamicArrayType& objReceiver, const std::istream& isSource)
{
	JSONStructure jsonRep = JSONStructure::Parse(isSource);
	{
		JSONArray dynamicArray_json = jsonRep.GetMember("dynamicArray").as<JSONArray>();
		if (objReceiver.dynamicArray) delete[] objReceiver.dynamicArray;
		objReceiver.dynamicArray = new int[dynamicArray_json.GetItems().size()];
		arraySize = dynamicArray_json.GetItems().size();
		for(size_t i_1 = 0; i_1 < arraySize; i_1++)
		{
			objReceiver.dynamicArray[i_1] = dynamicArray_json[i_1].as<JSONNumber>().value;
		}
	}

}
)cpp";

	AssertCodeEqual(code, expected);
}

TEST_F(JSONFormatPluginTest, HandlesMultiDimensionalStaticArray)
{
	GenerateSASTFromSources({
		{"MultiArrayType.h", R"cpp(
			#pragma once
			#include "SerializationMacros.h"
			#include "MultiArrayType.generated.h"

			class SERIALIZABLE(JSON) MultiArrayType {
				SERIALIZE_FIELD
				STATIC_ARRAY
				int grid[2][3];

				GENERATED_SERIALIZATION_BODY();
			};
		)cpp"}
		});

	auto it = globalSASTMap.find("MultiArrayType");
	ASSERT_NE(it, globalSASTMap.end());

	const auto& node = it->second;
	ASSERT_EQ(node->fields.size(), 1u);
	EXPECT_EQ(node->fields[0].type, SASTType::Array);
	ASSERT_NE(node->fields[0].elementType, nullptr);
	EXPECT_EQ(node->fields[0].elementType->type, SASTType::Array);
	EXPECT_EQ(node->fields[0].elementType->elementType->type, SASTType::Int);

	JSONFormatPlugin plugin;
	std::string code = plugin.GenerateCode(node);
	ASSERT_FALSE(code.empty());

	const char* expected = R"cpp(#include <fstream>

#include <JSONStructure.h>

static void JSONSerialize(JSONObject& jsonReceiver, const MultiArrayType& objSource)
{
	{
		JSONArray grid_json;
		for(size_t i_1 = 0; i_1 < sizeof(objSource.grid) / sizeof(objSource.grid[0]); i_1++)
		{
			{
				JSONArray nested_json;
				for(size_t i_3 = 0; i_3 < sizeof(objSource.grid[i_1]) / sizeof(objSource.grid[i_1][0]); i_3++)
				{
					nested_json.AddMember(JSONNumber(static_cast<double>(objSource.grid[i_1][i_3])));
				}
				grid_json.AddMember(std::move(nested_json));
			}
		}
		jsonReceiver.AddMember("grid", std::move(grid_json));
	}

}

static void JSONDeserialize(MultiArrayType& objReceiver, const JSONObject& jsonSource)
{
	{
		JSONArray grid_json = jsonSource.GetMember("grid").as<JSONArray>();
		for(size_t i_1 = 0; i_1 < sizeof(objReceiver.grid) / sizeof(objReceiver.grid[0]); i_1++)
		{
			{
				JSONArray nested_json = grid_json[i_1].as<JSONArray>();
				for(size_t i_3 = 0; i_3 < sizeof(objReceiver.grid[i_1]) / sizeof(objReceiver.grid[i_1][0]); i_3++)
				{
					objReceiver.grid[i_1][i_3] = nested_json[i_3].as<JSONNumber>().value;
				}
			}
		}
	}

}

static void JSONSerialize(std::ostream& osReceiver, const MultiArrayType& objSource)
{
	JSONStructure jsonRep;
	{
		JSONArray grid_json;
		for(size_t i_1 = 0; i_1 < sizeof(objSource.grid) / sizeof(objSource.grid[0]); i_1++)
		{
			{
				JSONArray nested_json;
				for(size_t i_3 = 0; i_3 < sizeof(objSource.grid[i_1]) / sizeof(objSource.grid[i_1][0]); i_3++)
				{
					nested_json.AddMember(JSONNumber(static_cast<double>(objSource.grid[i_1][i_3])));
				}
				grid_json.AddMember(std::move(nested_json));
			}
		}
		jsonRep.AddMember("grid", std::move(grid_json));
	}

	osReceiver << jsonRep.Stringify();
}

static void JSONDeserialize(MultiArrayType& objReceiver, const std::istream& isSource)
{
	JSONStructure jsonRep = JSONStructure::Parse(isSource);
	{
		JSONArray grid_json = jsonRep.GetMember("grid").as<JSONArray>();
		for(size_t i_1 = 0; i_1 < sizeof(objReceiver.grid) / sizeof(objReceiver.grid[0]); i_1++)
		{
			{
				JSONArray nested_json = grid_json[i_1].as<JSONArray>();
				for(size_t i_3 = 0; i_3 < sizeof(objReceiver.grid[i_1]) / sizeof(objReceiver.grid[i_1][0]); i_3++)
				{
					objReceiver.grid[i_1][i_3] = nested_json[i_3].as<JSONNumber>().value;
				}
			}
		}
	}

}
)cpp";

	AssertCodeEqual(code, expected);
}

TEST_F(JSONFormatPluginTest, HandlesPODWithArray)
{
	GenerateSASTFromSources({
		{"PODArrayTest.h", R"cpp(
			#pragma once
			#include "SerializationMacros.h"
			#include "PODArrayTest.generated.h"

			struct SERIALIZABLE_POD PodArray {
				STATIC_ARRAY
				int arr[4];
			};

			class SERIALIZABLE(JSON) Wrapper {
				SERIALIZE_FIELD
				PodArray pod;

				GENERATED_SERIALIZATION_BODY();
			};
		)cpp"}
		});

	auto it = globalSASTMap.find("Wrapper");
	ASSERT_NE(it, globalSASTMap.end());

	const auto& node = it->second;
	ASSERT_EQ(node->fields.size(), 1u);
	EXPECT_EQ(node->fields[0].type, SASTType::POD); // because pod gets expanded
	EXPECT_EQ(node->fields[0].formattedName, "pod");

	JSONFormatPlugin plugin;
	std::string code = plugin.GenerateCode(node);
	ASSERT_FALSE(code.empty());

	const char* expected = R"cpp(#include <fstream>

#include <JSONStructure.h>

static void JSONSerialize(JSONObject& jsonReceiver, const Wrapper& objSource)
{
	{
		JSONObject pod_json;
		{
			JSONArray arr_json;
			for(size_t i_2 = 0; i_2 < sizeof(objSource.pod.arr) / sizeof(objSource.pod.arr[0]); i_2++)
			{
				arr_json.AddMember(JSONNumber(static_cast<double>(objSource.pod.arr[i_2])));
			}
			pod_json.AddMember("arr", std::move(arr_json));
		}
		jsonReceiver.AddMember("pod", std::move(pod_json));
	}

}

static void JSONDeserialize(Wrapper& objReceiver, const JSONObject& jsonSource)
{
	{
		JSONArray arr_json = jsonSource.GetMember("pod").as<JSONObject>().GetMember("arr").as<JSONArray>();
		for(size_t i_1 = 0; i_1 < sizeof(objReceiver.pod.arr) / sizeof(objReceiver.pod.arr[0]); i_1++)
		{
			objReceiver.pod.arr[i_1] = arr_json[i_1].as<JSONNumber>().value;
		}
	}

}

static void JSONSerialize(std::ostream& osReceiver, const Wrapper& objSource)
{
	JSONStructure jsonRep;
	{
		JSONObject pod_json;
		{
			JSONArray arr_json;
			for(size_t i_2 = 0; i_2 < sizeof(objSource.pod.arr) / sizeof(objSource.pod.arr[0]); i_2++)
			{
				arr_json.AddMember(JSONNumber(static_cast<double>(objSource.pod.arr[i_2])));
			}
			pod_json.AddMember("arr", std::move(arr_json));
		}
		jsonRep.AddMember("pod", std::move(pod_json));
	}

	osReceiver << jsonRep.Stringify();
}

static void JSONDeserialize(Wrapper& objReceiver, const std::istream& isSource)
{
	JSONStructure jsonRep = JSONStructure::Parse(isSource);
	{
		JSONArray arr_json = jsonRep.GetMember("pod").as<JSONObject>().GetMember("arr").as<JSONArray>();
		for(size_t i_1 = 0; i_1 < sizeof(objReceiver.pod.arr) / sizeof(objReceiver.pod.arr[0]); i_1++)
		{
			objReceiver.pod.arr[i_1] = arr_json[i_1].as<JSONNumber>().value;
		}
	}

}
)cpp";

	AssertCodeEqual(code, expected);
}

TEST_F(JSONFormatPluginTest, HandlesPODWithVector)
{
	GenerateSASTFromSources({
		{"PODVectorTest.h", R"cpp(
			#pragma once
			#include <vector>
			#include "SerializationMacros.h"
			#include "PODVectorTest.generated.h"

			struct SERIALIZABLE_POD PodVec {
				std::vector<float> values;
			};

			class SERIALIZABLE(JSON) Wrapper {
				SERIALIZE_FIELD
				PodVec pod;

				GENERATED_SERIALIZATION_BODY();
			};
		)cpp"}
		});

	auto it = globalSASTMap.find("Wrapper");
	ASSERT_NE(it, globalSASTMap.end());

	const auto& node = it->second;
	ASSERT_EQ(node->fields.size(), 1u);
	EXPECT_EQ(node->fields[0].type, SASTType::POD);
	EXPECT_EQ(node->fields[0].formattedName, "pod");

	JSONFormatPlugin plugin;
	std::string code = plugin.GenerateCode(node);
	ASSERT_FALSE(code.empty());

	const char* expected = R"cpp(#include <fstream>

#include <JSONStructure.h>

static void JSONSerialize(JSONObject& jsonReceiver, const Wrapper& objSource)
{
	{
		JSONObject pod_json;
		{
			JSONArray values_json;
			for(const auto& item_2 : objSource.pod.values)
			{
				values_json.AddMember(JSONNumber(static_cast<double>(item_2)));
			}
			pod_json.AddMember("values", std::move(values_json));
		}
		jsonReceiver.AddMember("pod", std::move(pod_json));
	}

}

static void JSONDeserialize(Wrapper& objReceiver, const JSONObject& jsonSource)
{
	{
		for(const auto& item_1 : jsonSource.GetMember("pod").as<JSONObject>().GetMember("values").as<JSONArray>().GetItems())
		{
			auto& elem_1 = objReceiver.pod.values.emplace_back();
			elem_1 = item_1.as<JSONNumber>().value;
		}
	}

}

static void JSONSerialize(std::ostream& osReceiver, const Wrapper& objSource)
{
	JSONStructure jsonRep;
	{
		JSONObject pod_json;
		{
			JSONArray values_json;
			for(const auto& item_2 : objSource.pod.values)
			{
				values_json.AddMember(JSONNumber(static_cast<double>(item_2)));
			}
			pod_json.AddMember("values", std::move(values_json));
		}
		jsonRep.AddMember("pod", std::move(pod_json));
	}

	osReceiver << jsonRep.Stringify();
}

static void JSONDeserialize(Wrapper& objReceiver, const std::istream& isSource)
{
	JSONStructure jsonRep = JSONStructure::Parse(isSource);
	{
		for(const auto& item_1 : jsonRep.GetMember("pod").as<JSONObject>().GetMember("values").as<JSONArray>().GetItems())
		{
			auto& elem_1 = objReceiver.pod.values.emplace_back();
			elem_1 = item_1.as<JSONNumber>().value;
		}
	}

}
)cpp";

	AssertCodeEqual(code, expected);
}