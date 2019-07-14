# BerkeleyDBSamples
Примеры использования STL-интерфейса BerkeleyDB

## Зависимости

Для выполнения сборки Вам потребуется:

* git
* cmake версии **2.8** и выше
* [Berkeley DB](https://www.oracle.com/database/technologies/related/berkeleydb.html) версии **6.2.32** и выше.
* gcc7 или аналогичный компилятор с поддержкой С++17

В приложении используется модуль [FindBerkeleyDB](https://gitlab.com/sum01/FindBerkeleyDB) для подключения **Berkeley DB** к проекту.

## Сборка проекта

Перед началом необходимо склонировать этот репозиторий:

```
git clone https://github.com/sqglobe/BerkeleyDBSamples.git
```

Далее следует получить все подмодули проекта:
```
cd BerkeleyDBSamples
git submodule init
git submodule update
```

Проект собирается следующим образом. В директории `BerkeleyDBSamples` создаем папку `build` и 
запускаем собственно сборку:

```
cd build
cmake ../
make
```

При необходимости дирректория с установленной библиотекой **Berkeley DB** указывается в параметре `BerkeleyDB_ROOT_DIR`:

```
cd build
cmake -DBerkeleyDB_ROOT_DIR=/home/nick/libs  ../
make
```

После завершения сборки готовые бинарные файлы будут располагаться в **build/samples**.

Если используется статическая сборка Berkeley DB, может понадобится установить флаг `THREADS_PREFER_PTHREAD_FLAG` в **ON**.

