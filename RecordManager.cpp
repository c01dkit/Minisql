#pragma  warning(disable:4996)
#ifndef _CRT_SECURE_NO_WARNING
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <iostream>
#include "RecordManager.h"
#include <cstring>
#include "API.h"

/**
 * @Description : 一个表有数据文件与索引文件，但是表名是一样的。此函数用于传入表名，获取索引文件名
 */
string RecordManager::indexFileNameGet(string indexName){
    string newIndexName = indexName + "_index";
    return newIndexName;
}

/**
 * @Description : 一个表有数据文件与索引文件，但是表名是一样的。此函数用于传入表名，获取数据文件名
 */
string RecordManager::tableFileNameGet(string tableName) {
    string newIndexName = tableName + "_data";
    return newIndexName;
}
/**
 * @Description : 此函数用于创建数据文件，一张表至少用一个文件。这个文件纯粹存放数据，表名用文件名表示。
 *                表名、字段名、列数、键的属性等各种非记录本身的信息由cm保管在别的文件里
 */

bool RecordManager::tableCreate(string tableName) {
    FILE * fp;
    string newTableName = tableFileNameGet(tableName);
    if (!(fp = fopen(newTableName.c_str(),"w+"))){
        return false;
    } else {
        fclose(fp);
        return true;
    }
}

/**
 * @Description : 此函数用于创建索引文件
 */
bool RecordManager::indexCreate(string indexName) {
    FILE * fp;
    string newTableName = indexFileNameGet(indexName);
    if (!(fp = fopen(newTableName.c_str(),"w+"))){
        return false;
    } else {
        fclose(fp);
        return true;
    }
}

/**
 * @Description : 此函数用于删除数据文件
 *  remove 函数用于删除文件，删除成功返回0，文件不存在返回-1
 */
bool RecordManager::tableDrop(string tableName) {
    string dataFileName = tableFileNameGet(tableName);
    bm.delete_fileNode(dataFileName.c_str());
    return remove(dataFileName.c_str()) == 0;
}

/**
 * @Description : 此函数用于删除索引文件
 *  remove 函数用于删除文件，删除成功返回0，文件不存在返回-1
 */
bool RecordManager::indexDrop(string indexName) {
    string newIndexName = indexFileNameGet(indexName);
    return remove(newIndexName.c_str()) == 0;
}

/**
 * @Description : 插入一条记录
 */
int RecordManager::recordInsert(string tableName,char* record, int recordSize) {
    fileNode *ftmp = bm.getFile(tableFileNameGet(tableName).c_str());
    blockNode *btmp = bm.getBlockHead(ftmp);
    while (true) {
        if (btmp == NULL) return -1;
        if (bm.getBlockSize() - recordSize >= bm.get_usingSize(*btmp)){
            // 可以插入一条记录
            char* position = bm.get_content(*btmp) + bm.get_usingSize(*btmp);
            memcpy(position, record, recordSize);
            bm.set_usingSize(*btmp, bm.get_usingSize(*btmp) + recordSize);
            bm.set_dirty(*btmp);
            return btmp->offset;
        }
        btmp = bm.getNextBlock(ftmp, btmp);
    }
}

/**
 * @Description : 一个数据文件存储该表所有数据，而且可能包含多个块。此函数用于没有找到索引时从头遍历所有块
 *                搜索的记录全部在此文件：不会出现两个文件保存同一个表的情况
 */
int RecordManager::recordAllShow(string tableName, vector<string>* attributeNameVector, vector<Condition>* conditionVector){
    fileNode *ftmp = bm.getFile(tableFileNameGet(tableName).c_str());
    blockNode *btmp = bm.getBlockHead(ftmp);

    int recordNumber = 0;
    int newlyAdd;
    while (true) {
        if (btmp == NULL) return -1;
        newlyAdd = recordBlockShow(tableName, attributeNameVector, conditionVector, btmp); //根据表名、字段名、条件块指针获取这一块符合条件的记录数
        recordNumber += newlyAdd;
        if (btmp->if_end) return recordNumber;
        btmp = bm.getNextBlock(ftmp, btmp);
    }
}
/**
* @Description : 根据传入的块指针进行块内查找，输出查找结果并返回符合条件的记录数
*/
int RecordManager::recordBlockShow(string tableName, vector<string>* attributeNameVector, vector<Condition>* conditionVector, blockNode* block){
    if (block == NULL) return -1;
    vector<Attribute> attributeVector;
    api->attributeGet(tableName, &attributeVector);
    int recordSize = api->recordSizeGet(tableName);
    char* recordStart = bm.get_content(*block);
    char* blockStart = bm.get_content(*block);
    int blockUsingSize = bm.get_usingSize(*block);
    int totalNum = 0;
    while (recordStart - blockStart < blockUsingSize) { //当前记录是有效记录
        if (recordSatisfied(recordStart, &attributeVector, conditionVector)){
            //输出结果
            int i = 0;
            for (char* tempStart = recordStart; i < attributeNameVector->size(); i++) {
                switch (attributeVector[i].type) {
                    case (Attribute::TYPE_INT): {
                        printf("%d ",*(int*)tempStart);
                        break;
                    }
                    case (Attribute::TYPE_FLOAT): {
                        printf("%f ",*(float*)tempStart);
                        break;
                    }
                    default:{
                        printf("%s ",tempStart);
                    }
                }
                tempStart += api->typeSizeGet(attributeVector[i].type);
            }
            totalNum += 1;
            printf("\n");
        }
        recordStart += recordSize;
    }
    return totalNum;
}
/**
 * @Description : 这一函数是RecordManager内部函数，用于判断此记录是否满足搜索条件.为了提高效率，默认文件数据存放顺序与索引文件存放顺序一致
 */
bool RecordManager::recordSatisfied(char* recordStart, vector<Attribute>* attributeVector, vector<Condition>* conditionVector){
    if (conditionVector->empty()) return true; //select * 没有判断条件
    char temp[255];
    int tempSize;
    for (int i = 0; i < attributeVector->size(); i++) {
        tempSize = api->typeSizeGet((*attributeVector)[i].type);
        for (int j = 0; j < conditionVector->size(); j++) {
            if ((*attributeVector)[i].name != (*conditionVector)[j].attributeName) continue; //继续查看下一个条件
            memcpy(temp, recordStart, 255);
            //用最大255作为临时空间，检测整数和浮点数进行转化；否则直接比较字符串内容
            switch ((*attributeVector)[i].type) {
                case (Attribute::TYPE_INT):{
                    int tempInt = *((int*)temp);
                    if (!(*conditionVector)[j].ifRight(tempInt)) return false;
                    break;
                }
                case (Attribute::TYPE_FLOAT): {
                    float tempFloat = *((float*)temp);
                    if (!(*conditionVector)[j].ifRight(tempFloat)) return false;
                    break;
                }
                default:{
                    if (!(*conditionVector)[j].ifRight(temp)) return false;
                }
            }
        }
        recordStart += tempSize;
    }
    return true;
}
/**
 * @Description : 根据传入的块偏移量进行块内查找，输出查找结果并返回符合条件的记录数
 */
int RecordManager::recordBlockShow(string tableName, vector<string>* attributeNameVector, vector<Condition>* conditionVector, int blockOffset){
    fileNode *ftmp = bm.getFile(tableFileNameGet(tableName).c_str());
    blockNode* block = bm.getBlockByOffset(ftmp, blockOffset);
    if (block == NULL) return -1;
    return recordBlockShow(tableName, attributeNameVector, conditionVector, block);
}
/**
 * @Description : 这一函数返回一共找到了多少条记录；插入时使用，维护唯一键性质
 */
int RecordManager::recordAllFind(string tableName, vector<Condition>* conditionVector){
    fileNode *ftmp = bm.getFile(tableFileNameGet(tableName).c_str());
    blockNode *btmp = bm.getBlockHead(ftmp);

    int recordNumber = 0;
    int newlyAdd;
    while (true) {
        if (btmp == NULL) return -1;
        newlyAdd = recordBlockFind(tableName, conditionVector, btmp);
        recordNumber += newlyAdd;
        if (btmp->if_end) return recordNumber;
        btmp = bm.getNextBlock(ftmp, btmp);
    }
}
/**
 * @Description : 在给定块中搜索满足条件的记录数
 */
int RecordManager::recordBlockFind(string tableName, vector<Condition>* conditionVector, blockNode* block) {
    if (block == NULL) return 0;
    int recordSize = api->recordSizeGet(tableName);
    char* recordStart = bm.get_content(*block);
    char* blockStart = bm.get_content(*block);
    int blockUsingSize = bm.get_usingSize(*block);
    int totalNum = 0;
    vector<Attribute> attributeVector;
    api->attributeGet(tableName, &attributeVector);

    while (recordStart - blockStart < blockUsingSize) { //当前记录是有效记录
        if (recordSatisfied(recordStart, &attributeVector, conditionVector)) totalNum++;
        recordStart += recordSize;
    }
    return totalNum;
}

/**
 * @Description : 一个数据文件存储该表所有数据，而且可能包含多个块。此函数用于没有找到索引时从头遍历所有块
 *                搜索的记录全部在此文件：不会出现两个文件保存同一个表的情况
 */
int RecordManager::recordAllDelete(string tableName, vector<Condition>* conditionVector){
    fileNode *ftmp = bm.getFile(tableFileNameGet(tableName).c_str());
    blockNode *btmp = bm.getBlockHead(ftmp);
    if (btmp == NULL) return -1;
    int recordNumber = 0;
    int newlyAdd;
    while (btmp->if_end) {
        newlyAdd = recordBlockDelete(tableName, conditionVector, btmp); //根据表名、字段名、条件块指针获取这一块符合条件的记录数
        recordNumber += newlyAdd;
        btmp = bm.getNextBlock(ftmp, btmp);
    }
    return recordNumber;
}

/**
 * @Description : 根据传入的块偏移量进行块内查找，删除对应记录并返回符合条件的记录数
 */
int RecordManager::recordBlockDelete(string tableName, vector<Condition>* conditionVector, int blockOffset){
    fileNode *ftmp = bm.getFile(tableFileNameGet(tableName).c_str());
    blockNode* block = bm.getBlockByOffset(ftmp, blockOffset);
    if (block == NULL) return -1;
    return recordBlockDelete(tableName, conditionVector, block);
}

/**
 * @Description : 根据传入的块指针进行块内查找，删除对应记录并返回符合条件的记录数
 */
int RecordManager::recordBlockDelete(string tableName, vector<Condition>* conditionVector, blockNode* block){
    if (block == NULL) return -1;
    vector<Attribute> attributeVector;
    api->attributeGet(tableName, &attributeVector);
    int recordSize = api->recordSizeGet(tableName);
    char* recordStart = bm.get_content(*block);
    int totalNum = 0;
    int i;
    while (recordStart - bm.get_content(*block) < bm.get_usingSize(*block)) { //当前记录是有效记录
        if (recordSatisfied(recordStart, &attributeVector, conditionVector)){
            api->recordIndexDelete(recordStart, recordSize, &attributeVector, block->offset);
            for (i = 0; i + recordSize + recordStart - bm.get_content(*block) < bm.get_usingSize(*block); i++) {
                recordStart[i] = recordStart[i + recordSize]; //一条记录满足则将此块剩余所有记录前移一个单位
            }
            memset(recordStart + i, 0, recordSize);
            bm.set_usingSize(*block, bm.get_usingSize(*block) - recordSize);
            bm.set_dirty(*block);
            totalNum += 1;
        }
        recordStart += recordSize;
    }
    return totalNum;
}

/**
 * @Description : 插入一个索引
 */
int RecordManager::indexRecordAllAlreadyInsert(string tableName,string indexName) {
    fileNode *ftmp = bm.getFile(tableFileNameGet(tableName).c_str());
    blockNode *btmp = bm.getBlockHead(ftmp);
    int totalNum = 0;
    int newlyAdd;

    while (true) {
        if (btmp == NULL) return -1;
        newlyAdd = indexRecordBlockAlreadyInsert(tableName, indexName, btmp);
        totalNum += newlyAdd;
        if (btmp->if_end) return totalNum;
        btmp = bm.getNextBlock(ftmp, btmp);
    }
}

int RecordManager::indexRecordBlockAlreadyInsert(string tableName, string indexName, blockNode* block) {
    if (block == NULL) return -1;
    int count = 0;
    int type;
    int typeSize;
    char * contentBegin;
    char* recordBegin = bm.get_content(*block);
    int recordSize = api->recordSizeGet(tableName);
    vector<Attribute> attributeVector;
    api->attributeGet(tableName, &attributeVector);
    while (recordBegin - bm.get_content(*block)  < bm.get_usingSize(*block)) {
        contentBegin = recordBegin;
        for (int i = 0; i < attributeVector.size(); i++){
            type = attributeVector[i].type;
            typeSize = api->typeSizeGet(type);
            if (attributeVector[i].index == indexName){
                api->indexInsert(indexName, contentBegin, type, block->offset);
                count++;
            }
            contentBegin += typeSize;
        }
        recordBegin += recordSize;
    }
    return count;
}