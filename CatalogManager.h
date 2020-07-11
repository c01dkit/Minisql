#ifndef CATALOGMANAGER_H
#define CATALOGMANAGER_H

#include <string>
#include <vector>
#include "Attribute.h"
#include "BufferManager.h"
#include "BPlusTree.h"
#include "IndexManager.h"
using namespace std;
class CatalogManager {
public:
    BufferManager bm;
    CatalogManager();
    virtual ~CatalogManager();
    /* Table */
    int isTable(string tableName);
    int createTable(string tableName, vector<Attribute>* attributeVector, string primaryKeyName, int primaryKeyLocation);
    int dropTable(string tableName);
    /* Index */
    int isIndex(string indexName);
    int createIndex(string indexName, string tableName, string attributeName, int type);
    int dropIndex(const string& index);
    /* Index:get */
    int getAllIndex(vector<IndexInfo>* indexs);
    int getIndexType(string indexName);
    int getIndexNamebyTableName(string tableName, vector<string>* indexNameVector);
    /* Index:set */
    int setIndexOnAttribute(string tableName, string AttributeName, string indexName);
    int deleteIndexOnAttribute(string tableName, string AttributeName, string indexName);
    /* Record */
    int getRecordNum(string tableName);
    void getRecordString(string tableName, vector<string>* recordContent, char* recordResult);
    int insertRecord(string tableName, int recordNum);  // increment the number of record
    int deleteRecord(string tableName, int deleteNum);// delete the number of record
    /* Attribute */
    int getAttribute(string tableName, vector<Attribute>* attributeVector);
    Attribute* attributePointerInit(string tableName, int& size);
    /* Length */
    int calcuteTableLenth(string tableName);
    int calcuteTypeLenth(int type);
};




#endif