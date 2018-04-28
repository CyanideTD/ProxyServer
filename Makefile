BASE_PATH := ../network-linux/base
LIB_PATH := $(BASE_PATH)/lib
project.targets += client
project.extra_warning := -Wno-write-strings -fpermissive
client.path := bin/
client.name := client
client.defines := TIXML_USE_STL _UTF8_CODE_
client.sources := $(wildcard *.cpp)
client.includes := $(BASE_PATH)
client.ldadd := $(LIB_PATH)/libSvrPub.a $(LIB_PATH)/libWtseBase.a -lpthread -lmysqlcppconn

include $(BASE_PATH)/Generic.mak
