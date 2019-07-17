#include <iostream>

#include <experimental/filesystem>
#include <cstring>

#include <db_cxx.h>
#include <dbstl_vector.h>

#include <algorithm>
#include <iterator>

#include <thread>

const std::string ENV_FOLDER="sample-queue-multithread-usage-dir";

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

std::ostream& operator<<(std::ostream& os, const TestElement &el){
    os << "{ id: " << el.id << ", name: " << el.name << "}";
    return os;
}

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


// shows basic vector operations
void vector_modifications(dbstl::db_vector<TestElement> &container){
    {
        // create a values, that will be copied into the persistent storage
        std::list<TestElement> inputValues{
            {"test id 1", "test name 1"},
            {"test id 2", "test name 2"},
            {"test id 3", "test name 3"},
            {"test id 4", "test name 4"},
        };

        // copy the values into storage

        std::copy(std::cbegin(inputValues), std::cend(inputValues),  std::back_inserter(container));
    }

    std::cout << "Filled persistant storage" << std::endl;
    std::copy(container.begin(dbstl::ReadModifyWriteOption::no_read_modify_write(), true),
                   container.end(),
                   std::ostream_iterator<TestElement>(std::cout, "\n"));

    {
        // use basic STL algorithm to find an element
        auto iter = std::find_if(container.begin(), container.end(), [](const auto &val){
           return val.id == "test id 2" ;
        });

        if(iter != container.end()){
            auto &ref = *iter;
            ref.id = "new db1";
            ref.name = "new name 1";
            ref._DB_STL_StoreElement();
        }
    }

    // modify an element by index
    container[2] = {"new db 2", "new name 2"};

    std::cout << "Changed persistant storage" << std::endl;
    std::copy(container.begin(dbstl::ReadModifyWriteOption::no_read_modify_write(), true),
                   container.end(),
                   std::ostream_iterator<TestElement>(std::cout, "\n"));
}

std::thread make_thread(dbstl::db_vector<TestElement> &container, Db *db, DbEnv *env){

    return std::thread([&container, db, env](){
        dbstl::register_db_env(env);
        dbstl::register_db(db);

        while(!container.empty()){
           auto val = container.front();
           container.pop_front();
           std::this_thread::sleep_for(std::chrono::milliseconds(10));
           std::cout << "Fetch in thread: " << val << std::endl;
        }

        dbstl::dbstl_thread_exit();
    });
}

int main()
{
    if(!fs::exists(ENV_FOLDER) && !fs::create_directory(ENV_FOLDER)){
       std::cerr << "Failed to create env directory: " << ENV_FOLDER << std::endl;
       return 1;
    }

    dbstl::dbstl_startup();


    auto penv = dbstl::open_env(ENV_FOLDER.c_str(), 0u, DB_INIT_MPOOL | DB_CREATE | DB_INIT_LOCK | DB_THREAD);

    // open an environment and the database
    auto db = new Db(nullptr, DB_CXX_NO_EXCEPTIONS);

    db->set_flags(DB_RENUMBER);

    if(0 != db->open(nullptr, "sample-queue-multithread-usage.db", nullptr, DB_RECNO, DB_CREATE | DB_THREAD, 0600)){
        std::cerr << "Failed to open DB in path " << "sample-map-usage.db" << std::endl;
        delete db;
        return 1;
    }

    dbstl::register_db(db);


    //cleare current database data
    db->truncate(nullptr, nullptr, 0u);


    // register callbacks for marshalling/unmarshalling stored element
    auto inst = dbstl::DbstlElemTraits<TestElement>::instance();
    inst->set_size_function(&TestMarshaller::size);
    inst->set_copy_function(&TestMarshaller::store);
    inst->set_restore_function(&TestMarshaller::restore);


    // create a persistant storage

    dbstl::db_vector<TestElement> container(db, penv);


    vector_modifications(container);

    //create a thrad for value extraction
    auto workThread = make_thread(container, db, penv);

    std::initializer_list<TestElement> addValues = {
        {"thread key 1", "thread name 1"},
        {"thread key 2", "thread name 2"},
        {"thread key 3", "thread name 3"},
        {"thread key 4", "thread name 4"},
        {"thread key 5", "thread name 5"},
    };

    for(const auto &val : addValues){
        container.push_back(val);
    }


    if(workThread.joinable()){
       workThread.join();
    }

    dbstl::dbstl_exit();

    return 0;
}
