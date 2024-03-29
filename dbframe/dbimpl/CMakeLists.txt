
find_package(MySQL)

set(DB_HEADERS
  AdhocStatement.h
  PreparedStatement.h
  DatabaseEnv.h
  DatabaseEnvFwd.h
  DatabaseLoader.h
  DatabaseWorker.h
  DatabaseWorkerPool.h
  Field.h
  MySQLConnection.h
  MySQLHacks.h
  MySQLPreparedStatement.h
  MySQLThreading.h
  MySQLWorkaround.h
  QueryCallback.h
  QueryHolder.h
  QueryResult.h
  SQLOperation.h
  Transaction.h
  Implementation/SakilaDatabase.h
  Define.h
  CompilerDefs.h
  ProducerConsumerQueue.h
  Errors.h
  Log.h
  StringFormat.h
  Duration.h
  Timer.h
  AsyncCallbackProcessor.h
)

set(DB_SOURCES
  AdhocStatement.cpp
  PreparedStatement.cpp
  DatabaseEnv.cpp
  DatabaseLoader.cpp
  DatabaseWorker.cpp
  DatabaseWorkerPool.cpp
  Field.cpp
  MySQLConnection.cpp
  MySQLPreparedStatement.cpp
  MySQLThreading.cpp
  QueryCallback.cpp
  QueryHolder.cpp
  QueryResult.cpp
  Transaction.cpp
  Implementation/SakilaDatabase.cpp
  Errors.cpp
)

add_library(dbimpl STATIC ${DB_SOURCES} ${DB_HEADERS})

# MYSQL_INCLUDE_DIR 定义在 FindMySQL.cmake 里面，这个变量里面写了在哪儿能找到mysql所需的文件
target_include_directories(dbimpl
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/fmt/include
    ${MYSQL_INCLUDE_DIR})

# MYSQL_LIBRARY 也是定义在 FindMySQL.cmake 里面
target_link_libraries(dbimpl
  PUBLIC
    pthread
    fmt
    ${MYSQL_LIBRARY})