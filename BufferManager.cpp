#pragma  warning(disable:4996)
#ifndef _CRT_SECURE_NO_WARNING
#define _CRT_SECURE_NO_WARNINGS
#endif
#include "BufferManager.h"
#include <string>
#include <cstring>
#include <queue>

static int replaced_block = -1; //替换操作时的初始编号设-1

void BufferManager::init_block(struct blockNode& block)
{
    memset(block.address, 0, BLOCK_SIZE);
    size_t init_usage = 0;
    memcpy(block.address, (char*)&init_usage, sizeof(size_t)); //设置block head
    block.using_size = sizeof(size_t);
    block.preblock = NULL;
    block.nextblock = NULL;
    block.offset = -1;
    block.if_dirty = false;
    block.if_lock = false;
    block.if_end = false;
    block.reference = false;
    memset(block.filename, 0, FILE_NAME);
}

void BufferManager::init_file(fileNode& file)
{
    file.nextfile = NULL;
    file.prefile = NULL;
    file.blockhead = NULL;
    file.if_lock = false;
    memset(file.filename, 0, FILE_NAME);
}

//BufferManager的Ctor
BufferManager::BufferManager() :total_block(0), total_file(0), filehead(NULL) {
    for (int i = 0; i < FILE_NUM; i++) { //初始化file_pool
        file_pool[i].filename = new char[FILE_NAME];
        if (file_pool[i].filename == NULL) {
            printf("Failure! File name can't be null\n");
            exit(1);
        }
        init_file(file_pool[i]);
    }
    for (int i = 0; i < BLOCK_NUM; i++) {
        block_pool[i].address = new char[BLOCK_SIZE];
        if (block_pool[i].address == NULL) {
            printf("Failure! address can't be null\n");
            exit(1);
        }
        block_pool[i].filename = new char[FILE_NAME];
        if (block_pool[i].filename == NULL) {
            printf("Failure! File name can't be null\n");
            exit(1);
        }
        init_block(block_pool[i]);
    }
}

//BufferManager的Dtor
BufferManager::~BufferManager()
{
    writeback(NULL, NULL, 1);
    for (int i = 0; i < FILE_NUM; i++) {
        delete[] file_pool[i].filename;
    }
    for (int i = 0; i < BLOCK_NUM; i++) {
        delete[] block_pool[i].address;
    }
}


//获取file
fileNode* BufferManager::getFile(const char* fileName, bool if_lock)
{
    blockNode* tmpblock = NULL;
    fileNode* tmpfile = NULL;
    if (filehead != NULL) {
        for (tmpfile = filehead; tmpfile != NULL; tmpfile = tmpfile->nextfile) {
            if (!strcmp(fileName, tmpfile->filename)) { //file已经存在
                tmpfile->if_lock = if_lock;
                return tmpfile;
            }
        }
    }
    if (total_file == 0) { //目前没有file
        tmpfile = &file_pool[total_file];
        total_file++;
        filehead = tmpfile;
    }
    else if (total_file < FILE_NUM) { // 还有空余的file node 可以使用
        tmpfile = &file_pool[total_file];
        file_pool[total_file - 1].nextfile = tmpfile;
        tmpfile->prefile = &file_pool[total_file - 1];
        total_file++;
    }
    else { //总数大于file最大值，开始调整
        tmpfile = filehead;
        while (tmpfile->if_lock) {
            if (tmpfile->nextfile != NULL) tmpfile = tmpfile->nextfile;
            else {
                printf("There are not enough file node!");
                exit(2);
            }
        }
        for (tmpblock = tmpfile->blockhead; tmpblock != NULL; tmpblock = tmpblock->nextblock) {
            if (tmpblock->preblock != NULL) {
                init_block(*(tmpblock->preblock));
                total_block--;
            }
            writeback(tmpblock->filename, tmpblock, 0);
        }
        init_file(*tmpfile);
    }
    if (strlen(fileName) + 1 > FILE_NAME) {
        printf("The file name is too long\n");
        exit(3);
    }
    strncpy(tmpfile->filename, fileName, FILE_NAME);
    filelock(*tmpfile, if_lock);
    return tmpfile;
}

//获取特定位置的block，根据位置的可用情况，进行调整
blockNode* BufferManager::getBlock(fileNode* file, blockNode* position, bool if_lock) {
    const char* tmpfilename = file->filename;
    blockNode* tmpblock = NULL;
    if (total_block == 0) {
        tmpblock = &block_pool[0];
        total_block++;
    }
    else if (total_block < BLOCK_NUM) { // 依然有空余的block
        for (int i = 0; i < BLOCK_NUM; i++) {
            if (block_pool[i].offset == -1) {
                tmpblock = &block_pool[i];
                total_block++;
                break;
            }
            else continue;
        }
    }
    else { //没有空余的block，需要进行替换
        int i = replaced_block;
        while (true) {
            i++;
            if (i >= total_block) i = 0;
            if (!block_pool[i].if_lock) {
                if (block_pool[i].reference == true)
                    block_pool[i].reference = false;
                else //进行替换
                {
                    tmpblock = &block_pool[i];
                    if (tmpblock->nextblock != NULL) {
                        tmpblock->nextblock->preblock = tmpblock->preblock;
                    }
                    if (tmpblock->preblock != NULL) {
                        tmpblock->preblock->nextblock = tmpblock->nextblock;
                    }
                    if (tmpblock == file->blockhead) {
                        file->blockhead = tmpblock->nextblock;
                    }
                    replaced_block = i;
                    writeback(tmpblock->filename, tmpblock, 0); //把替换好的结果写回
                    init_block(*tmpblock); //重新初始化临时的block
                    break;
                }
            }
            else continue; //已经上锁，跳过
        }
    }
    //插入block
    if (position != NULL) {
        if (position->nextblock == NULL) {
            tmpblock->preblock = position;
            position->nextblock = tmpblock;
            tmpblock->offset = position->offset + 1;
        }
        else {
            tmpblock->preblock = position;
            tmpblock->nextblock = position->nextblock;
            position->nextblock->preblock = tmpblock;
            position->nextblock = tmpblock;
            tmpblock->offset = position->offset + 1;

        }
    }
    else {
        tmpblock->offset = 0; //放到head
        if (file->blockhead != NULL) {
            file->blockhead->preblock = tmpblock;
            tmpblock->nextblock = file->blockhead;
        }
        file->blockhead = tmpblock;
    }
    blocklock(*tmpblock, if_lock);
    if (strlen(tmpfilename) + 1 > FILE_NAME) {
        printf("The file name is too long\n");
        exit(3);
    }
    strncpy(tmpblock->filename, tmpfilename, FILE_NAME);

    //读回到block
    FILE* f;
    if ((f = fopen(tmpfilename, "ab+")) == NULL) {
        printf("Failure!In opening the file");
        exit(1);
    }
    else {
        if (fseek(f, tmpblock->offset * BLOCK_SIZE, 0) == 0) {
            if (fread(tmpblock->address, 1, BLOCK_SIZE, f) == 0) { //已经到底
                tmpblock->if_end = true;
            }
            tmpblock->using_size = getUsingSize(tmpblock);
        }
        else {
            printf("Failure! In seeking the file");
            exit(1);
        }
        fclose(f);
    }
    return tmpblock;
}

//将block写回储存空间中，flag = 0写回需要的block， flag=1时全部写回
void BufferManager::writeback(const char* fileName, blockNode* block, int flag) {
    if (flag == 0) {
        if (!block->if_dirty) {
            return;
        }
        else {
            FILE* fw = NULL;
            if ((fw = fopen(fileName, "rb+")) == NULL) {
                printf("Failure! In opening the file");
                exit(1);
            }
            else {
                if (fseek(fw, block->offset * BLOCK_SIZE, 0) == 0) {
                    if (fwrite(block->address, block->using_size + sizeof(size_t), 1, fw) != 1) {
                        printf("Failure! In writing the file");
                        exit(1);
                    }
                }
                else
                {
                    printf("Failure! In seeking the file");
                    exit(1);
                }
                fclose(fw);
            }
        }
    }
    else { //全部写回
        blockNode* tmpblock = NULL;
        fileNode* tmpfile = NULL;
        if (filehead != NULL) {
            for (tmpfile = filehead; tmpfile != NULL; tmpfile = tmpfile->nextfile) {
                if (tmpfile->blockhead != NULL) {
                    for (tmpblock = tmpfile->blockhead; tmpblock != NULL; tmpblock = tmpblock->nextblock) {
                        if (tmpblock->preblock != NULL) {
                            init_block(*(tmpblock->preblock));
                        }
                        writeback(tmpblock->filename, tmpblock, 0);
                    }
                }
            }
        }
    }
}

//返回参数中block的next block
blockNode* BufferManager::getNextBlock(fileNode* file, blockNode* block) {
    if (block->nextblock == NULL) {
        if (block->if_end) block->if_end = false;
        return getBlock(file, block);
    }
    else {
        if (block->offset == block->nextblock->offset - 1) { //通过offset判定next
            return block->nextblock;
        }
        else {
            return getBlock(file, block); //else说明block的次序有问题，进行调整
        }
    }
}

//获取blcok head
blockNode* BufferManager::getBlockHead(fileNode* file) {
    blockNode* tmpblock = NULL;
    if (file->blockhead != NULL) {
        if (file->blockhead->offset == 0) { //offset = 0说明处于head
            tmpblock = file->blockhead;
            return tmpblock;
        }
        else {
            tmpblock = getBlock(file, NULL);
        }
    }
    else {
        tmpblock = getBlock(file, NULL);
    }
    return tmpblock;
}

//通过offset获取block
blockNode* BufferManager::getBlockByOffset(fileNode* file, int offset) {
    blockNode* tmpblock = NULL;
    tmpblock = getBlockHead(file);
    if (offset == 0) {
        return tmpblock;
    }
    else {
        while (offset > 0) {
            tmpblock = getNextBlock(file, tmpblock);
            offset--;
        }
        return tmpblock;
    }
}

//删除一个file结点
void BufferManager::delete_fileNode(const char* fileName) {
    fileNode* tmpfile = getFile(fileName);
    blockNode* tmpblock = getBlockHead(tmpfile);
    queue<blockNode*> q;
    while (true) {
        if (tmpblock == NULL) return;
        q.push(tmpblock);
        if (tmpblock->if_end) break;
        tmpblock = getNextBlock(tmpfile, tmpblock);
    }
    total_block = total_block - q.size();
    while (!q.empty()) {
        init_block(*q.back());
        q.pop();
    }
    if (tmpfile->prefile) tmpfile->prefile->nextfile = tmpfile->nextfile;
    if (tmpfile->nextfile) tmpfile->nextfile->prefile = tmpfile->prefile;
    if (filehead == tmpfile) filehead = tmpfile->nextfile;
    init_file(*tmpfile);
    total_file--;
}

//更改block锁的状态
void BufferManager::blocklock(blockNode& block, bool lock)
{
    block.if_lock = lock;
    if (!lock)
        block.reference = true;
}

//更改file锁的状态
void BufferManager::filelock(fileNode& file, bool lock)
{
    file.if_lock = lock;
}

//设定dirty为true
void BufferManager::set_dirty(blockNode& block)
{
    block.if_dirty = true;
}

//设定dirty为false
void BufferManager::clean_dirty(blockNode& block)
{
    block.if_dirty = false;
}

//获取size
size_t BufferManager::getUsingSize(blockNode* block)
{
    return *(size_t*)block->address;
}

void BufferManager::set_usingSize(blockNode& block, size_t usage)
{
    block.using_size = usage;
    memcpy(block.address, (char*)&usage, sizeof(size_t));
}

size_t BufferManager::get_usingSize(blockNode& block)
{
    return block.using_size;
}

//获取block的content
char* BufferManager::get_content(blockNode& block)
{
    return block.address + sizeof(size_t);
}