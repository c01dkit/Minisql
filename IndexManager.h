#ifndef __IndexManager__
#define __IndexManager__

#include "Attribute.h"
#include "BPlusTree.h"
#include <stdio.h>
#include <map>
#include <string>
#include <sstream>

class API;

class IndexInfo
{
public:
	IndexInfo(string indexName,string tableName,string Attribute,int type)
    :indexName(indexName),tableName(tableName),Attribute(Attribute),type(type){}
    int type;
    string indexName;
    string tableName;
    string Attribute;
};

class IndexManager{
public:
    IndexManager(API *api);
    ~IndexManager();
    int searchIndex(string filePath,string key,int type);
    void createIndex(string filePath,int type);
    void insertIndex(string filePath,string key,int blockOffset,int type); 
    void dropIndex(string filePath,int type);
    void deleteIndexByKey(string filePath,string key,int type);
private:
    API *api;
    /* Map */
    typedef map<string,BPlusTree<int>    *> intMap;
    typedef map<string,BPlusTree<string> *> stringMap;
    typedef map<string,BPlusTree<float>  *> floatMap;
    intMap indexIntMap;
    stringMap indexStringMap;
    floatMap indexFloatMap;
    /* Key:储存特定类型单元 */
    struct Content{
        int intContent;
        float floatContent;
        string stringContent;
    };
    struct Content keyContent;
    int getKeySize(int type);
    int getDegree(int type);
    void setKey(int type,string key);
};

#endif
