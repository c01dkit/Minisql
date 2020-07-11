#ifndef BUFFERMANAGERH
#define BUFFERMANAGERH
#include <stdio.h>
#include <time.h>
#include <iostream>
using namespace std;

#define FILE_NUM 40
#define BLOCK_NUM 300
#define FILE_NAME 100

struct blockNode
{
    int offset; // block�е�ƫ����offset
    bool if_lock;  // �ж����block�Ƿ�����
    bool if_end; // �ж��Ƿ��ļ����ײ�
    char* filename;
    char* address;
    blockNode* preblock;
    blockNode* nextblock;
    bool reference;
    bool if_dirty; // �ж�block�Ƿ�dirty
    size_t using_size; // block��ʹ�õ�byte size�������blockhead
};

struct fileNode
{
    char* filename;
    blockNode* blockhead;
    fileNode* prefile;
    fileNode* nextfile;
    bool if_lock;
};

extern clock_t start;
extern void print();

class BufferManager
{
private:
    fileNode* filehead;
    int total_block;
    int total_file;
    fileNode file_pool[FILE_NUM];
    blockNode block_pool[BLOCK_NUM];
    void init_block(blockNode& block);
    void init_file(fileNode& file);
    void writeback(const char* fileName, blockNode* block, int flag);
    void clean_dirty(blockNode& block);
    size_t getUsingSize(blockNode* block);
    static const int BLOCK_SIZE = 4096;

public:
    BufferManager();
    ~BufferManager();
    void delete_fileNode(const char* fileName);
    blockNode* getBlock(fileNode* file, blockNode* position, bool if_lock = false);
    blockNode* getNextBlock(fileNode* file, blockNode* block);
    blockNode* getBlockHead(fileNode* file);
    blockNode* getBlockByOffset(fileNode* file, int offest);
    fileNode* getFile(const char* fileName, bool if_lock = false);
    void set_dirty(blockNode& block);
    void blocklock(blockNode& block, bool pin);
    void filelock(fileNode& file, bool pin);
    void set_usingSize(blockNode& block, size_t usage);
    size_t get_usingSize(blockNode& block);
    char* get_content(blockNode& block);
    static int getBlockSize() {
        return BLOCK_SIZE - sizeof(size_t);
    }
};

#endif



