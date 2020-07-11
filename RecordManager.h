#ifndef RECORDMANAGER_H
#define RECORDMANAGER_H
#include "Condition.h"
#include "Attribute.h"
#include "RecordManager.h"
#include "BufferManager.h"
#include <string>
#include <vector>
using namespace std;
class API;
class RecordManager{
    public:
	    RecordManager(){}
        BufferManager bm;
        API *api;
        bool tableCreate(string tableName);
        bool tableDrop(string tableName);
        bool indexCreate(string indexName);
        bool indexDrop(string indexName);

        
        int recordInsert(string tableName, char* record, int recordSize);
        
        int recordAllShow(string tableName, vector<string>* attributeNameVector, vector<Condition>* conditionVector);
        int recordBlockShow(string tableName, vector<string>* attributeNameVector, vector<Condition>* conditionVector, int blockOffset);
        
        int recordAllFind(string tableName, vector<Condition>* conditionVector);
        
        int recordAllDelete(string tableName, vector<Condition>* conditionVector);
        int recordBlockDelete(string tableName,  vector<Condition>* conditionVector, int blockOffset);
        
        int indexRecordAllAlreadyInsert(string tableName,string indexName);
        
        string tableFileNameGet(string tableName);
        string indexFileNameGet(string indexName);



    private:
        bool recordSatisfied(char* record, vector<Attribute>* attributeNameVector, vector<Condition>* conditionVector);
        int recordBlockShow(string tableName, vector<string>* attributeNameVector, vector<Condition>* conditionVector, blockNode* block);
        int recordBlockFind(string tableName, vector<Condition>* conditionVector, blockNode* block);
        int recordBlockDelete(string tableName,  vector<Condition>* conditionVector, blockNode* block);
        int indexRecordBlockAlreadyInsert(string tableName,string indexName, blockNode* block);

};
#endif