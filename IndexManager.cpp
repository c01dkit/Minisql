#pragma  warning(disable:4996)
#ifndef _CRT_SECURE_NO_WARNING
#define _CRT_SECURE_NO_WARNINGS
#endif
#include "IndexManager.h"
#include <iostream>
#include "API.h"
#include <vector>
using namespace std;
static const int TYPE_FLOAT = Attribute::TYPE_FLOAT;//-1
static const int TYPE_INT = Attribute::TYPE_INT;//0

/**
 *  *  Constructor Function: create the existing index by the index files.
 *  *
 *  *  @param API*
 *  */
IndexManager::IndexManager(API* api) :api(api)
{
    vector<IndexInfo> allIndexInfo;
    api->allIndexAddressInfoGet(&allIndexInfo);
    for (vector<IndexInfo>::iterator i = allIndexInfo.begin(); i != allIndexInfo.end(); i++)
        createIndex(i->indexName, i->type);
}
IndexManager::~IndexManager()
{
    for (intMap::iterator i = indexIntMap.begin(); i != indexIntMap.end(); i++)
    {
        if (!i->second)continue;
        i->second->writtenbackToDiskAll();
        delete i->second;
    }
    for (floatMap::iterator i = indexFloatMap.begin(); i != indexFloatMap.end(); i++)
    {
        if (!i->second)continue;
        i->second->writtenbackToDiskAll();
        delete i->second;
    }
    for (stringMap::iterator i = indexStringMap.begin(); i != indexStringMap.end(); i++)
    {
        if (!i->second)continue;
        i->second->writtenbackToDiskAll();
        delete i->second;
    }
}

/**
 *  * Create index on the specific type.
 *  * If there exists the index before, read data from file path and then rebuild the b+ tree.
 *  *
 *  * @param string the file path
 *  * @param int type
 *  *
 *  * @return void
 *  *
 *  */
void IndexManager::createIndex(string filePath, int type)
{
    if (type < -1) {
        cout << "Type error in IndexManager::createIndex()" << endl;
        return;
    }
    int keySize = getKeySize(type);
    int degree = getDegree(type);
    switch (type)
    {
    case TYPE_INT: {
        BPlusTree<int>* tree = new BPlusTree<int>(filePath, keySize, degree);
        indexIntMap.insert(intMap::value_type(filePath, tree));
        break;
    }
    case TYPE_FLOAT: {
        BPlusTree<float>* tree = new BPlusTree<float>(filePath, keySize, degree);
        indexFloatMap.insert(floatMap::value_type(filePath, tree));
        break;
    }
    default: {//Default to be a string
        BPlusTree<string>* tree = new BPlusTree<string>(filePath, keySize, degree);
        indexStringMap.insert(stringMap::value_type(filePath, tree));
        break;
    }
    }
}

/**
 *  * Drop the specific index.
 *  *
 *  * @param
 *  *
 *  * @return void
 *  *
 *  */
void IndexManager::dropIndex(string filePath, int type)
{
    if (type < -1) {
        cout << "Type error in IndexManager::dropIndex()" << endl;
        return;
    }
    switch (type)
    {
    case TYPE_INT: {
        intMap::iterator i = indexIntMap.find(filePath);
        if (i != indexIntMap.end())
        {
            delete i->second;
            indexIntMap.erase(i);
            return;
        }
        break;
    }
    case TYPE_FLOAT: {
        floatMap::iterator i = indexFloatMap.find(filePath);
        if (i != indexFloatMap.end())
        {
            delete i->second;
            indexFloatMap.erase(i);
            return;
        }
        break;
    }
    default: {//Default to be a string
        stringMap::iterator i = indexStringMap.find(filePath);
        if (i != indexStringMap.end())
        {
            delete i->second;
            indexStringMap.erase(i);
            return;
        }
        break;
    }
    }
    cout << "IndexManager::dropIndex() Error: No such a index ' " << filePath << " ' exits" << endl;
}
int IndexManager::searchIndex(string filePath, string key, int type)
{
    setKey(type, key);
    if (type < -1) {
        cout << "Type error in IndexManager::searchIndex()" << endl;
        return -1;
    }
    switch (type)
    {
    case TYPE_INT: {
        intMap::iterator i = indexIntMap.find(filePath);
        if (i != indexIntMap.end()) return i->second->search(keyContent.intContent);
        break;
    }
    case TYPE_FLOAT: {
        floatMap::iterator i = indexFloatMap.find(filePath);
        if (i != indexFloatMap.end()) return i->second->search(keyContent.floatContent);
        break;
    }
    default: {//Default to be a string
        stringMap::iterator i = indexStringMap.find(filePath);
        if (i != indexStringMap.end()) return i->second->search(key);
        break;
    }
    }
    cout << "IndexManager::searchIndex() Error: No such a index ' " << filePath << " ' exits" << endl;
    return -1;
}
void IndexManager::insertIndex(string filePath, string key, int blockOffset, int type)
{
    if (type < -1) {
        cout << "Type error in IndexManager::insertIndex()" << endl;
        return;
    }
    setKey(type, key);
    switch (type)
    {
    case TYPE_INT: {
        intMap::iterator i = indexIntMap.find(filePath);
        if (i != indexIntMap.end()) i->second->insertKey(keyContent.intContent, blockOffset);
        else cout << "IndexManager::insertIndex() Error: No such a index ' " << filePath << " ' exits" << endl;
        break;
    }
    case TYPE_FLOAT: {
        floatMap::iterator i = indexFloatMap.find(filePath);
        if (i != indexFloatMap.end()) i->second->insertKey(keyContent.floatContent, blockOffset);
        else cout << "IndexManager::insertIndex() Error: No such a index ' " << filePath << " ' exits" << endl;
        break;
    }
    default: {//Default to be a string
        stringMap::iterator i = indexStringMap.find(filePath);
        if (i != indexStringMap.end()) i->second->insertKey(key, blockOffset);
        else cout << "IndexManager::insertIndex() Error: No such a index ' " << filePath << " ' exits" << endl;
        break;
    }
    }
    return;
}
void IndexManager::deleteIndexByKey(string filePath, string key, int type)
{
    if (type < -1) {
        cout << "Type error in IndexManager::deleteIndex()" << endl;
        return;
    }
    setKey(type, key);
    switch (type)
    {
    case TYPE_INT: {
        intMap::iterator i = indexIntMap.find(filePath);
        if (i != indexIntMap.end()) i->second->deleteKey(keyContent.intContent);
        else cout << "IndexManager::deleteIndex() Error: No such a index ' " << filePath << " ' exits" << endl;
        break;
    }
    case TYPE_FLOAT: {
        floatMap::iterator i = indexFloatMap.find(filePath);
        if (i != indexFloatMap.end()) i->second->deleteKey(keyContent.floatContent);
        else cout << "IndexManager::deleteIndex() Error: No such a index ' " << filePath << " ' exits" << endl;
        break;
    }
    default: {//Default to be a string
        stringMap::iterator i = indexStringMap.find(filePath);
        if (i != indexStringMap.end()) i->second->deleteKey(key);
        else cout << "IndexManager::deleteIndex() Error: No such a index ' " << filePath << " ' exits" << endl;
        break;
    }
    }
    return;
}
int IndexManager::getDegree(int type)
{
    int degree = bm.getBlockSize() / (getKeySize(type) + sizeof(int));
    return degree - !(degree & 1);//if degree is an even, return degree - 1
}
int IndexManager::getKeySize(int type)
{
    if (type < -1)
    {
        cout << "Invalid type in IndexManager::getKeySize()" << endl;
        return -100;
    }
    switch (type)
    {
    case TYPE_INT:   return sizeof(int);
    case TYPE_FLOAT: return sizeof(float);
    default:         return type + 1;
    }
}
void IndexManager::setKey(int type, string key)
{
    if (type < -1)
    {
        cout << "Invalid type in IndexManager::setKey()" << endl;
        return;
    }
    stringstream ss;
    ss << key;
    switch (type)
    {
    case TYPE_INT:   ss >> this->keyContent.intContent; return;
    case TYPE_FLOAT: ss >> this->keyContent.floatContent; return;
    default:         ss >> this->keyContent.stringContent; return;
    }
}