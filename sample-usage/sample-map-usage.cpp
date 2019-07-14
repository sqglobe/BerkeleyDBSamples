#include <iostream>

#include <experimental/filesystem>
#include <cstring>

#include <db_cxx.h>
#include <dbstl_map.h>

#include <algorithm>
#include <iterator>

const std::string env_dir("sample-map-usage-dir");

namespace fs = std::experimental::filesystem;

void* save_str(const std::string& str, void* dest)
{
  auto size = str.length();
  memcpy(dest, &size, sizeof(size));
  char* charDest = static_cast<char*>(dest) + sizeof(size);
  memcpy(charDest, str.data(), str.length());
  return charDest + str.length();
}

const void* restore_str(std::string& str, const void* src)
{
  std::string::size_type size;
  memcpy(&size, src, sizeof(size));
  str.reserve(size);
  const char* strSrc = static_cast<const char*>(src) + sizeof(size);
  str.insert(0, strSrc, size);
  return strSrc + size;
}

//stored element type
struct TestElement{
    std::string id;
    std::string name;
};

//marshall/unmarshal class
class TestMarshaller {
 public:
  static void restore(TestElement& elem, const void* src)
  {
    src = restore_str(elem.id, src);
    src = restore_str(elem.name, src);
  }
  static u_int32_t size(const TestElement& element)
  {
    u_int32_t size = 0;
    size += sizeof(std::string::size_type) + element.id.length();
    size += sizeof(std::string::size_type) + element.name.length();
    return size;
  }
  static void store(void* dest, const TestElement& elem)
  {
    dest = save_str(elem.id, dest);
    dest = save_str(elem.name, dest);
  }
};


int main()
{
    if(!fs::exists(env_dir) && !fs::create_directory(env_dir)){
       std::cerr << "Failed to create env directory: " << env_dir << std::endl;
       return 1;
    }

    dbstl::dbstl_startup();

    // open an environment and the database
    auto penv = dbstl::open_env(env_dir.c_str(), 0u, DB_INIT_MPOOL | DB_CREATE);
    auto db = dbstl::open_db(penv, "sample-map-usage.db", DB_BTREE, DB_CREATE, 0u);

    //cleare current database data
    db->truncate(nullptr, nullptr, 0u);


    // register callbacks for marshalling/unmarshalling stored element
    auto inst = dbstl::DbstlElemTraits<TestElement>::instance();
    inst->set_size_function(&TestMarshaller::size);
    inst->set_copy_function(&TestMarshaller::store);
    inst->set_restore_function(&TestMarshaller::restore);


    // create a persistant storage
    dbstl::db_map<std::string, TestElement> elementsMap(db, penv);

    {
        // create a values, that will be copied into the persistent storage
        std::map<std::string, TestElement> inputValues{
            {"test key 1", {"test id 1", "test name 1"}},
            {"test key 2", {"test id 2", "test name 2"}},
            {"test key 3", {"test id 3", "test name 3"}},
            {"test key 4", {"test id 4", "test name 4"}},
        };

        // copy the values into storage

        std::copy(std::cbegin(inputValues), std::cend(inputValues), std::inserter(elementsMap, elementsMap.begin(dbstl::ReadModifyWriteOption::read_modify_write())));
    }

    std::cout << "Filled persistant storage" << std::endl;
    std::transform(elementsMap.begin(dbstl::ReadModifyWriteOption::no_read_modify_write(), true), elementsMap.end(), std::ostream_iterator<std::string>(std::cout, "\n"), [](const auto data) -> std::string {
       return  data.first + "=> { id: " + data.second.id + ", name: " + data.second.name + "}";
    });


    elementsMap["added key 1"] = {"added id 1", "added name 1"};

    {
        auto [iter, res] = elementsMap.insert(std::make_pair(std::string("added key 2"), TestElement{"added id 2", "added name 2"}));
        if(!res){
            std::cerr << "Failed to insert value " << std::endl;
        }
    }

    // update a value in the storage with iterator's _DB_STL_StoreElement function
    if(auto findIter = elementsMap.find("test key 1"); findIter != elementsMap.end()){
        auto &ref = *findIter;
        ref.second.id = "new test id 1";
        ref.second.name = "new test name 1";
        ref.second._DB_STL_StoreElement();
    }else{
        std::cerr << "Key " << "test key 1" << " not found " << std::endl;
    }

    //  update a value in the storage with direct assignment
    if(auto findIter = elementsMap.find("test key 2"); findIter != elementsMap.end()){
        findIter->second = {"new test id 2", "new test name 2"};
    }else{
        std::cerr << "Key " << "test key 2" << " not found " << std::endl;
    }

    std::cout << "Changed persistant storage" << std::endl;
    std::transform(elementsMap.begin(dbstl::ReadModifyWriteOption::no_read_modify_write(), true), elementsMap.end(), std::ostream_iterator<std::string>(std::cout, "\n"), [](const auto data) -> std::string {
       return  data.first + "=> { id: " + data.second.id + ", name: " + data.second.name + "}";
    });

    dbstl::dbstl_exit();

    return 0;
}
