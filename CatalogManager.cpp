#pragma  warning(disable:4996)
#ifndef _CRT_SECURE_NO_WARNING
#define _CRT_SECURE_NO_WARNINGS
#endif
#include "CatalogManager.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstring>


#define UNKNOWN_FILE 8
#define TABLE_FILE 9
#define INDEX_FILE 10
using namespace std;
CatalogManager::CatalogManager() {
    //cout << "CatalogManager Created!" << endl;
}

CatalogManager::~CatalogManager() {
    //cout << "CatalogManager Destoryed!" << endl;
}
int CatalogManager::isTable(string tableName)
{
    FILE* fp = fopen(tableName.c_str(), "r");
    if (fp)
    {
        fclose(fp);
        return TABLE_FILE;
    }
    else return 0;
}
int CatalogManager::createTable(string tableName, vector<Attribute>* attributeVector, string primaryKeyName = "", int primaryKeyLocation = 0)
{
    if (isTable(tableName) == TABLE_FILE)//TODO::未知是否createTable函数本身需要关心table是否已经存在
    {
        cout << "CatalogManager::createTable(" << tableName << ") Failed, table already exist!" << endl;
        return 0;
    }
    FILE* fp = fopen(tableName.c_str(), "w+");
    if (fp == NULL)
    {
        cout << "Create File Failed in 'CatalogManager::createTable(" << tableName << ")'!" << endl;
        return 0;//失锟杰凤拷锟斤拷0
    }
    fclose(fp);

    fileNode* ftmp = bm.getFile(tableName.c_str());
    blockNode* btmp = bm.getBlockHead(ftmp);
    if (btmp)
    {
        char* pContent = bm.get_content(*btmp);
        //set size = 0 which means zero record,size is a integer,4 bytes in total
        memset(pContent, 0, 4);
        pContent += sizeof(int);
        //what it says
        *pContent = primaryKeyLocation;
        pContent++;
        //attribute number
        *pContent = attributeVector->size();
        pContent++;
        //attributes
        for (int i = 0; i < (*attributeVector).size(); i++)
        {
            memcpy(pContent, &((*attributeVector)[i]), sizeof(Attribute));
            pContent += sizeof(Attribute);
        }
        bm.set_usingSize(*btmp, bm.get_usingSize(*btmp) + (*attributeVector).size() * sizeof(Attribute) + 2 + sizeof(int));
        bm.set_dirty(*btmp);
        return 1;//锟缴癸拷锟斤拷锟斤拷1
    }
    return 0;//失锟杰凤拷锟斤拷0
}
int CatalogManager::dropTable(string tableName)//TODO::能否直接让delete_fileNode返回值,重构代码
{
    bm.delete_fileNode(tableName.c_str());
    return !remove(tableName.c_str());//remove()删除失败返回值为0,但是这说明delete_fileNode()成功,应返回1;反之亦然。
}
int CatalogManager::isIndex(string fileName)
{
    fileNode* ftmp = bm.getFile("Indexs");
    blockNode* btmp = bm.getBlockHead(ftmp);
    size_t indexSize = bm.get_usingSize(*btmp);
    if (btmp)
    {
        IndexInfo* pContent = (IndexInfo*)bm.get_content(*btmp);
        for (int j = 0; j < (indexSize / sizeof(IndexInfo)); j++, pContent++)
            if (pContent->indexName == fileName)
                return INDEX_FILE;
        return UNKNOWN_FILE;
    }
    return 0;
}
int CatalogManager::createIndex(string indexName, string tableName, string Attribute, int type)
{
    fileNode* ftmp = bm.getFile("Indexs");
    blockNode* btmp = bm.getBlockHead(ftmp);
    IndexInfo info(indexName, tableName, Attribute, type);
    char* pContent = NULL;
    while (btmp)
    {
        if (bm.get_usingSize(*btmp) <= bm.getBlockSize() - sizeof(IndexInfo))
        {
            pContent = bm.get_content(*btmp) + bm.get_usingSize(*btmp);
            memcpy(pContent, &info, sizeof(IndexInfo));
            bm.set_usingSize(*btmp, bm.get_usingSize(*btmp) + sizeof(IndexInfo));
            bm.set_dirty(*btmp);
            return setIndexOnAttribute(tableName, Attribute, indexName);//调用成员函数建立index
        }
        btmp = bm.getNextBlock(ftmp, btmp);
    }
    return 0;
}
int CatalogManager::dropIndex(const string& index)
{
    fileNode* ftmp = bm.getFile("Indexs");
    blockNode* btmp = bm.getBlockHead(ftmp);
    if (btmp)
    {
        IndexInfo* info = (IndexInfo*)bm.get_content(*btmp);
        int j;
        for (j = 0; j < (bm.get_usingSize(*btmp) / sizeof(IndexInfo)); j++, info++)
            if (info->indexName == index) break;//寻找index位置
        if (j >= (bm.get_usingSize(*btmp) / sizeof(IndexInfo)))
        {
            cout << "Index does not exist!" << endl;
            return 0;
        }
        deleteIndexOnAttribute(info->tableName, info->Attribute, info->indexName);//调用成员函数删除index
        for (; j < (bm.get_usingSize(*btmp) / sizeof(IndexInfo) - 1); j++, ++info)
            (*info) = *(info + sizeof(IndexInfo));//前移其他index
        bm.set_usingSize(*btmp, bm.get_usingSize(*btmp) - sizeof(IndexInfo));
        bm.set_dirty(*btmp);

        return 1;
    }
    return 0;
}
int CatalogManager::getAllIndex(vector<IndexInfo>* indexs)
{
    fileNode* ftmp = bm.getFile("Indexs");
    blockNode* btmp = bm.getBlockHead(ftmp);
    if (btmp)
    {
        IndexInfo* info = (IndexInfo*)bm.get_content(*btmp);
        for (int j = 0; j < (bm.get_usingSize(*btmp) / sizeof(IndexInfo)); j++, info++)
            indexs->push_back((*info));
    }
    return 1;
}
int CatalogManager::getIndexType(string indexName)
{
    fileNode* ftmp = bm.getFile("Indexs");
    blockNode* btmp = bm.getBlockHead(ftmp);
    if (btmp)
    {
        IndexInfo* info = (IndexInfo*)bm.get_content(*btmp);
        for (int j = 0; j < (bm.get_usingSize(*btmp) / sizeof(IndexInfo)); j++, info++)
            if (info->indexName == indexName)
                return info->type;
    }
    return -2;//type <= -2锟斤拷味锟脚达拷锟斤拷type
}
int CatalogManager::getIndexNamebyTableName(string tableName, vector<string>* indexNameVector)
{
    fileNode* ftmp = bm.getFile("Indexs");
    blockNode* btmp = bm.getBlockHead(ftmp);
    if (btmp)
    {
        IndexInfo* info = (IndexInfo*)bm.get_content(*btmp);
        for (int j = 0; j < (bm.get_usingSize(*btmp) / sizeof(IndexInfo)); j++, info++)
            if (info->tableName == tableName)
                indexNameVector->push_back(info->indexName);
        return 1;
    }
    return 0;
}
int CatalogManager::setIndexOnAttribute(string tableName, string AttributeName, string indexName)
{
    fileNode* ftmp = bm.getFile(tableName.c_str());
    blockNode* btmp = bm.getBlockHead(ftmp);
    if (btmp)
    {
        int size;
        Attribute* pAttribute = attributePointerInit(tableName, size);
        for (int i = 0; i < size; i++, pAttribute++)
        {
            if (pAttribute->name == AttributeName)
            {
                pAttribute->index = indexName;
                bm.set_dirty(*btmp);
                return 1;
            }
        }
    }
    return 0;
}
int CatalogManager::deleteIndexOnAttribute(string tableName, string AttributeName, string indexName)
{
    fileNode* ftmp = bm.getFile(tableName.c_str());
    blockNode* btmp = bm.getBlockHead(ftmp);
    if (btmp)
    {
        int size;
        Attribute* pAttribute = attributePointerInit(tableName, size);
        for (int i = 0; i < size; i++, pAttribute++)
        {
            if (pAttribute->name == AttributeName)
            {
                if (pAttribute->index == indexName)
                {
                    pAttribute->index = "";
                    bm.set_dirty(*btmp);
                }
                else
                {
                    cout << "On table ' " << tableName << " ':" << endl;
                    cout << "Revoke unknown index ' " << indexName << " '." << endl;
                    cout << "Attribute ' " << AttributeName << " ' has index ' " << pAttribute->index << " ' !" << endl;
                }
                return 1;
            }
        }
    }
    return 0;
}
int CatalogManager::getRecordNum(string tableName)
{
    fileNode* ftmp = bm.getFile(tableName.c_str());
    blockNode* btmp = bm.getBlockHead(ftmp);
    if (btmp)
        return *((int*)bm.get_content(*btmp));
    else
        return 0;
}
void CatalogManager::getRecordString(string tableName, vector<string>* recordContent, char* recordResult)
{
    vector<Attribute> attributeVector;
    getAttribute(tableName, &attributeVector);
    char* pContent = recordResult;
    stringstream middlestream;
    string content;
    int type;
    for (int i = 0; i < attributeVector.size(); i++)
    {
        content = (*recordContent)[i];
        Attribute attribute = attributeVector[i];
        type = attribute.type;

        middlestream.clear();
        middlestream << content;
        switch (type)
        {
        case Attribute::TYPE_INT: {//content is a int
            int intTmp;
            middlestream >> intTmp;
            memcpy(pContent, (char*)&intTmp, sizeof(int));
            pContent += sizeof(int);
            break;
        }
        case Attribute::TYPE_FLOAT: {//content is a float
            float floatTmp;
            middlestream >> floatTmp;
            memcpy(pContent, (char*)&floatTmp, sizeof(float));
            pContent += sizeof(float);
            break;
        }
        default://content is a string
            memcpy(pContent, content.c_str(), (int)(type + 1));
            pContent += (int)(type + 1);
            break;
        }
    }
}
int CatalogManager::insertRecord(string tableName, int recordNum)
{
    fileNode* ftmp = bm.getFile(tableName.c_str());
    blockNode* btmp = bm.getBlockHead(ftmp);
    if (btmp)
    {
        int* originalRecordNum = (int*)bm.get_content(*btmp);
        *originalRecordNum += recordNum;
        bm.set_dirty(*btmp);
        return *originalRecordNum;
    }
    return 0;
}
int CatalogManager::deleteRecord(string tableName, int deleteNum)
{
    fileNode* ftmp = bm.getFile(tableName.c_str());
    blockNode* btmp = bm.getBlockHead(ftmp);
    if (btmp)
    {
        int* recordNum = (int*)bm.get_content(*btmp);
        if ((*recordNum) < deleteNum)
        {
            cout << "RecordNum < ReleteNum in CatalogManager::deleteRecord()" << endl;
            return 0;
        }
        else
            (*recordNum) -= deleteNum;

        bm.set_dirty(*btmp);
        return *recordNum;
    }
    return 0;
}
int CatalogManager::getAttribute(string tableName, vector<Attribute>* attributeVector)
{
    fileNode* ftmp = bm.getFile(tableName.c_str());
    blockNode* btmp = bm.getBlockHead(ftmp);
    if (btmp)
    {
        int size;
        Attribute* pAttribute = attributePointerInit(tableName, size);
        for (int i = 0; i < size; i++, pAttribute++)
            attributeVector->push_back(*pAttribute);
        return 1;
    }
    return 0;
}
Attribute* CatalogManager::attributePointerInit(string tableName, int& size)
{
    fileNode* ftmp = bm.getFile(tableName.c_str());
    blockNode* btmp = bm.getBlockHead(ftmp);
    char* pContent = bm.get_content(*btmp) + 1 + sizeof(int);
    size = *pContent;//锟矫碉拷attribute锟斤拷锟斤拷目
    pContent++;
    return (Attribute*)pContent;
}
int CatalogManager::calcuteTableLenth(string tableName)
{
    fileNode* ftmp = bm.getFile(tableName.c_str());
    blockNode* btmp = bm.getBlockHead(ftmp);
    if (btmp)
    {
        int recordSize = 0;
        int size;
        Attribute* pAttribute = attributePointerInit(tableName, size);

        for (int i = 0; i < size; i++, pAttribute++)
        {
            if (pAttribute->type < -1) {
                cout << "Catalog data damaged!" << endl;
                return 0;
            }
            switch (pAttribute->type)
            {
            case Attribute::TYPE_INT: {
                recordSize += sizeof(int);
                break;
            }
            case Attribute::TYPE_FLOAT: {
                recordSize += sizeof(float);
                break;
            }
            default: {
                recordSize += pAttribute->type * sizeof(char);
                break;
            }
            }
        }
        return recordSize;
    }
    return 0;
}
int CatalogManager::calcuteTypeLenth(int type) {
    switch (type)
    {
    case Attribute::TYPE_INT:   return sizeof(int);
    case Attribute::TYPE_FLOAT: return sizeof(float);
    default:                    return (int)(type + 1);
        break;
    }
}