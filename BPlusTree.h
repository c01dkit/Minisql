#ifndef BPLUSH
#define BPLUSH

#include <vector>
#include <stdio.h>
#include <string.h>
#include "BufferManager.h"
#include <string>

using namespace std;
static BufferManager bm;

//定义树节点
template <typename KeyType>
class TreeNode {
public:
    size_t count;  //key的总数
    TreeNode* parent;
    vector <KeyType> keys;
    vector <int> values;
    vector <TreeNode*> childs;
    TreeNode* next;
    bool isleaf;
    int degree;

public:
    TreeNode(int indegree, bool newleaf = false);
    ~TreeNode();
    bool isRoot();//判断是否是根
    bool search(KeyType key, size_t& index);//查询
    TreeNode* splite(KeyType& key);//分离
    size_t insert(KeyType& key); //插入非叶节点
    size_t insert(KeyType& key, int value); //插入叶节点
    bool Delete(size_t index);//删除
};



//定义B+树
template <typename KeyType>
class BPlusTree
{
private:
    typedef TreeNode<KeyType>* Node;

    struct searchNode
    {
        Node N;
        size_t index;
        bool if_found;
    };
    string filename;
    Node root;
    Node leafhead;
    size_t keycount;
    size_t level;
    size_t nodecount;
    fileNode* file;
    int keysize;
    int degree;

public:
    BPlusTree(string Name, int Keysize, int Degree);
    ~BPlusTree();

    int search(KeyType& key); // search the value of specific key
    bool insertKey(KeyType& key, int value);
    bool deleteKey(KeyType& key);
    void dropTree(Node node);
    void readFromDiskAll();
    void writtenbackToDiskAll();
    void readFromDisk(blockNode* btmp);

private:
    void init_tree();
    bool adjust(Node TNode, int flag); //flag = 0 ,插入后调整；flag = 1，删除后调整
    void findToLeaf(Node TNode, KeyType key, searchNode& s);

};

template <class KeyType>
TreeNode<KeyType>::TreeNode(int indegree, bool newleaf) {
    count = 0, parent = NULL, next = NULL, isleaf = newleaf, degree = indegree;
    for (size_t i = 0; i < degree + 1; i++) {
        childs.push_back(NULL);
        keys.push_back(KeyType());
        values.push_back(int());
    }
    childs.push_back(NULL);
}

template <class KeyType>
TreeNode<KeyType>::~TreeNode() {

}

template <class KeyType>
bool TreeNode<KeyType>::isRoot() {
    if (parent != NULL) return false;
    else return true;
}

template <class KeyType>
bool TreeNode<KeyType>::search(KeyType key, size_t& index) {
    if (count == 0) {
        index = 0;
        return false;
    }
    else if (count == 1) {
        if (keys[0] == key) {
            index = 0;
            return true;
        }
        else {
            index = 0;
            return false;
        }
    }
    else {
        if (keys[count - 1] < key) { //判断是否越界
            index = count;
            return false;
        }
        else if (keys[0] > key) {
            index = 0;
            return false;
        }
        else { //用二分法提高效率
            size_t left = 0, right = count - 1, pos = 0;
            while (right > left + 1) {
                pos = (right + left) / 2;
                if (keys[pos] == key) {
                    index = pos;
                    return true;
                }
                else if (keys[pos] < key) {
                    left = pos;
                }
                else if (keys[pos] > key) {
                    right = pos;
                }
            }
            if (keys[left] >= key) {
                index = left;
                return (keys[left] == key);
            }
            else if (keys[right] >= key) {
                index = right;
                return (keys[right] == key);
            }
            else if (keys[right] < key) {
                index = right++;
                return false;
            }
        }
    }
    return false;
}

template <class KeyType>
TreeNode<KeyType>* TreeNode<KeyType>::splite(KeyType& key) {
    size_t minnode = (degree - 1) / 2;
    TreeNode* newnode = new TreeNode(degree, this->isleaf);
    if (newnode == NULL) {
        cout << "Failure " << endl;
        exit(2);
    }
    if (isleaf) { //叶节点情况
        key = keys[minnode + 1];
        for (size_t i = minnode + 1; i < degree; i++) {
            newnode->keys[i - minnode - 1] = keys[i];
            keys[i] = KeyType();
            newnode->values[i - minnode - 1] = values[i];
            values[i] = int();
        }
        newnode->next = this->next;
        this->next = newnode;
        newnode->parent = this->parent;
        newnode->count = minnode;
        this->count = minnode + 1;
    }
    else if (!isleaf) { //非叶节点情况
        key = keys[minnode];
        for (size_t i = minnode + 1; i < degree + 1; i++) {
            newnode->childs[i - minnode - 1] = this->childs[i];
            newnode->childs[i - minnode - 1]->parent = newnode;
            this->childs[i] = NULL;
        }
        for (size_t i = minnode + 1; i < degree; i++) {
            newnode->keys[i - minnode - 1] = this->keys[i];
            this->keys[i] = KeyType();
        }
        this->keys[minnode] = KeyType();
        newnode->parent = this->parent;
        newnode->count = minnode;
        this->count = minnode;
    }
    return newnode;
}

template <class KeyType>
size_t TreeNode<KeyType>::insert(KeyType& key) {
    if (count == 0) {
        keys[0] = key;
        count++;
        return 0;
    }
    else {
        size_t index = 0;
        bool if_in = search(key, index);
        if (if_in) {
            cout << "Failure! The key is already in the tree" << endl;
            exit(3);
        }
        else {
            for (size_t i = count; i > index; i--) {
                keys[i] = keys[i - 1];
            }
            keys[index] = key;
            for (size_t i = count + 1; i > index + 1; i--) {
                childs[i] = childs[i - 1];
            }
            childs[index + 1] = NULL;
            count++;
            return index;
        }
    }
}

template <class KeyType>
size_t TreeNode<KeyType>::insert(KeyType& key, int value) {
    if (count == 0) {
        keys[0] = key;
        values[0] = value;
        count++;
        return 0;
    }
    else {
        size_t index = 0;
        bool if_in = search(key, index);
        if (if_in) {
            cout << "Failure! The key is already in the tree" << endl;
            exit(3);
        }
        else {
            for (size_t i = count; i > index; i--) {
                keys[i] = keys[i - 1];
                values[i] = values[i - 1];
            }
            keys[index] = key;
            values[index] = value;
            count++;
            return index;
        }
    }
}

template <class KeyType>
bool TreeNode<KeyType>::Delete(size_t index) {
    if (index > count) {
        cout << "Failure! Out of range" << endl;
        return false;
    }
    else {
        if (isleaf) {
            for (size_t i = index; i < count - 1; i++) {
                keys[i] = keys[i + 1];
                values[i] = values[i + 1];
            }
            keys[count - 1] = KeyType();
            values[count - 1] = int();
        }
        else {
            for (size_t i = index; i < count - 1; i++) {
                keys[i] = keys[i + 1];
            }
            for (size_t i = index + 1; i < count; i++) {
                childs[i] = childs[i + 1];
            }
            keys[count - 1] = KeyType();
            childs[count] = NULL;
        }
        count--;
        return true;
    }
}

//B+树部分
template <class KeyType>
BPlusTree<KeyType>::BPlusTree(string Name, int Keysize, int Degree) {
    filename = Name;
    keycount = 0, level = 0, nodecount = 0;
    root = NULL, leafhead = NULL, file = NULL;
    keysize = Keysize, degree = Degree;
    init_tree();
    readFromDiskAll();
}

template <class KeyType>
BPlusTree<KeyType>:: ~BPlusTree() {
    dropTree(root);
    keycount = 0;
    root = NULL;
    level = 0;
}

template <class KeyType>
void BPlusTree<KeyType>::init_tree() {
    root = new TreeNode<KeyType>(degree, true);
    keycount = 0;
    level = 1;
    nodecount = 1;
    leafhead = root;
}


template <class KeyType>
void BPlusTree<KeyType>::findToLeaf(Node TNode, KeyType key, searchNode& s) {
    size_t index = 0;
    if (TNode->search(key, index)) {
        if (TNode->isleaf) {
            s.N = TNode;
            s.index = index;
            s.if_found = true;
        }
        else {
            TNode = TNode->childs[index + 1];
            while (!TNode->isleaf) {
                TNode = TNode->childs[0];
            }
            s.N = TNode;
            s.index = 0;
            s.if_found = true;
        }
    }
    else {
        if (TNode->isleaf) {
            s.N = TNode;
            s.index = index;
            s.if_found = false;
        }
        else {
            findToLeaf(TNode->childs[index], key, s);
        }
    }
}

template <class KeyType>
bool BPlusTree<KeyType>::insertKey(KeyType& key, int value) {
    searchNode s;
    if (!root) init_tree();
    findToLeaf(root, key, s);
    if (s.if_found) {
        cout << "Failure!The key is already in the tree" << endl;
        return false;
    }
    else {
        s.N->insert(key, value);
        if (s.N->count == degree) {
            adjust(s.N, 0);
        }
        keycount++;
        return true;
    }
}


template <class KeyType>
bool BPlusTree<KeyType>::adjust(Node TNode, int flag) {
    if (flag == 0) {
        KeyType key;
        Node newnode = TNode->splite(key);
        nodecount++;
        if (TNode->isRoot()) {
            Node root = new TreeNode<KeyType>(degree, false);
            if (root == NULL) {
                cout << "Failure!" << endl;
                exit(1);
            }
            else {
                level++;
                nodecount++;
                this->root = root;
                TNode->parent = root;
                newnode->parent = root;
                root->insert(key);
                root->childs[0] = TNode;
                root->childs[1] = newnode;
                return true;
            }
        }
        else {
            Node parent = TNode->parent;
            size_t index = parent->insert(key);
            parent->childs[index + 1] = newnode;
            newnode->parent = parent;
            if (parent->count == degree)
                return adjust(parent, 0);
            return true;
        }
    }
    else {
        size_t minkey = (degree - 1) / 2;
        //无需调整
        if (((TNode->isleaf) && (TNode->count >= minkey)) || ((degree != 3) && (!TNode->isleaf) && (TNode->count >= minkey - 1)) || ((degree == 3) && (!TNode->isleaf) && (TNode->count < 0)))
        {
            return  true;
        }
        if (TNode->isRoot()) {
            if (TNode->count > 0) { //不调整
                return true;
            }
            else {
                if (root->isleaf) {
                    delete TNode;
                    root = NULL;
                    leafhead = NULL;
                    level--;
                    nodecount--;
                }
                else {
                    root = TNode->childs[0];
                    root->parent = NULL;
                    delete TNode;
                    level--;
                    nodecount--;
                }
            }
        }
        else {
            Node parent = TNode->parent, brother = NULL;
            if (TNode->isleaf) {
                size_t index = 0;
                parent->search(TNode->keys[0], index);
                if ((parent->childs[0] != TNode) && (index + 1 == parent->count)) { //从左兄弟开始调整
                    brother = parent->childs[index];
                    if (brother->count > minkey) {
                        for (size_t i = TNode->count; i > 0; i--) {
                            TNode->keys[i] = TNode->keys[i - 1];
                            TNode->values[i] = TNode->values[i - 1];
                        }
                        TNode->keys[0] = brother->keys[brother->count - 1];
                        TNode->values[0] = brother->values[brother->count - 1];
                        brother->Delete(brother->count - 1);
                        TNode->count++;
                        parent->keys[index] = TNode->keys[0];
                        return true;
                    }
                    else {
                        parent->Delete(index);
                        for (int i = 0; i < TNode->count; i++) {
                            brother->keys[i + brother->count] = TNode->keys[i];
                            brother->values[i + brother->count] = TNode->values[i];
                        }
                        brother->count += TNode->count;
                        brother->next = TNode->next;
                        delete TNode;
                        nodecount--;
                        return adjust(parent, 1);
                    }
                }
                else { //调整右兄弟
                    if (parent->childs[0] == TNode)
                        brother = parent->childs[1];
                    else
                        brother = parent->childs[index + 2];
                    if (brother->count > minkey) {
                        TNode->keys[TNode->count] = brother->keys[0];
                        TNode->values[TNode->count] = brother->values[0];
                        TNode->count++;
                        brother->Delete(0);
                        if (parent->childs[0] == TNode)
                            parent->keys[0] = brother->keys[0];
                        else
                            parent->keys[index + 1] = brother->keys[0];
                        return true;
                    }
                    else {
                        for (int i = 0; i < brother->count; i++) {
                            TNode->keys[TNode->count + i] = brother->keys[i];
                            TNode->values[TNode->count + i] = brother->values[i];
                        }
                        if (TNode == parent->childs[0])
                            parent->Delete(0);
                        else
                            parent->Delete(index + 1);
                        TNode->count += brother->count;
                        TNode->next = brother->next;
                        delete brother;
                        nodecount--;
                        return adjust(parent, 1);
                    }
                }
            }
            else { //调整非叶节点
                size_t index = 0;
                parent->search(TNode->childs[0]->keys[0], index);
                if ((parent->childs[0] != TNode) && (index + 1 == parent->count)) {
                    brother = parent->childs[index];
                    if (brother->count > minkey - 1) {
                        TNode->childs[TNode->count + 1] = TNode->childs[TNode->count];
                        for (size_t i = TNode->count; i > 0; i--) {
                            TNode->childs[i] = TNode->childs[i - 1];
                            TNode->keys[i] = TNode->keys[i - 1];
                        }
                        TNode->childs[0] = brother->childs[brother->count];
                        TNode->keys[0] = parent->keys[index];
                        TNode->count++;
                        //调整父节点
                        parent->keys[index] = brother->keys[brother->count - 1];
                        //调整兄弟和儿子
                        if (brother->childs[brother->count]) {
                            brother->childs[brother->count]->parent = TNode;
                        }
                        brother->Delete(brother->count - 1);
                        return true;
                    }
                    else {
                        brother->keys[brother->count] = parent->keys[index];
                        parent->Delete(index);
                        brother->count++;
                        for (int i = 0; i < TNode->count; i++) {
                            brother->childs[brother->count + i] = TNode->childs[i];
                            brother->keys[brother->count + i] = TNode->keys[i];
                            brother->childs[brother->count + i]->parent = brother;
                        }
                        brother->childs[brother->count + TNode->count] = TNode->childs[TNode->count];
                        brother->childs[brother->count + TNode->count]->parent = brother;
                        brother->count += TNode->count;
                        delete TNode;
                        nodecount--;
                        return adjust(parent, 1);
                    }
                }
                else {
                    if (parent->childs[0] == TNode)
                        brother = parent->childs[1];
                    else
                        brother = parent->childs[index + 2];
                    if (brother->count > minkey - 1) {
                        TNode->childs[TNode->count + 1] = brother->childs[0];
                        TNode->keys[TNode->count] = brother->keys[0];
                        TNode->childs[TNode->count + 1]->parent = TNode;
                        TNode->count++;
                        if (TNode == parent->childs[0])
                            parent->keys[0] = brother->keys[0];
                        else
                            parent->keys[index + 1] = brother->keys[0];
                        brother->childs[0] = brother->childs[1];
                        brother->Delete(0);
                        return true;
                    }
                    else {
                        TNode->keys[TNode->count] = parent->keys[index];
                        if (TNode == parent->childs[0])
                            parent->Delete(0);
                        else
                            parent->Delete(index + 1);
                        TNode->count++;
                        for (int i = 0; i < brother->count; i++) {
                            TNode->childs[TNode->count + i] = brother->childs[i];
                            TNode->keys[TNode->count + i] = brother->keys[i];
                            TNode->childs[TNode->count + i]->parent = TNode;
                        }
                        TNode->childs[TNode->count + brother->count] = brother->childs[brother->count];
                        TNode->childs[TNode->count + brother->count]->parent = TNode;
                        TNode->count += brother->count;
                        delete brother;
                        nodecount--;
                        return adjust(parent, 1);
                    }
                }
            }
        }
        return false;
    }
}

template <class KeyType>
int BPlusTree<KeyType>::search(KeyType& key) {
    if (!root) return -1;
    searchNode s;
    findToLeaf(root, key, s);
    if (!s.if_found) {
        return -1;
    }
    else {
        return s.N->values[s.index];
    }
}

template <class KeyType>
bool BPlusTree<KeyType>::deleteKey(KeyType& key) {
    searchNode s;
    if (!root) {
        cout << "Failure! " << endl;
        return false;
    }
    else {
        findToLeaf(root, key, s);
        if (!s.if_found) {
            cout << "Failure!" << endl;
            return false;
        }
        else {
            if (s.N->isRoot()) {
                s.N->Delete(s.index);
                keycount--;
                return adjust(s.N, 1);
            }
            else {
                if (s.index == 0 && leafhead != s.N) { //key存在于index节点中
                    size_t index = 0;
                    Node new_parent = s.N->parent;
                    bool if_found_branch = new_parent->search(key, index);
                    while (!if_found_branch) {
                        if (new_parent->parent)
                            new_parent = new_parent->parent;
                        else {
                            break;
                        }
                        if_found_branch = new_parent->search(key, index);
                    }
                    new_parent->keys[index] = s.N->keys[1];
                    s.N->Delete(s.index);
                    keycount--;
                    return adjust(s.N, 1);
                }
                else { //key同样存在在叶节点
                    s.N->Delete(s.index);
                    keycount--;
                    return adjust(s.N, 1);
                }
            }
        }
    }
}

template <class KeyType>
void BPlusTree<KeyType>::dropTree(Node node) {
    if (!node) return;
    if (!node->isleaf) {
        for (size_t i = 0; i <= node->count; i++) {
            dropTree(node->childs[i]);
            node->childs[i] = NULL;
        }
    }
    delete node;
    nodecount--;
    return;
}

template <class KeyType>
void BPlusTree<KeyType>::readFromDiskAll() {
    file = bm.getFile(filename.c_str());
    blockNode* tmpblock = bm.getBlockHead(file);
    while (true) {
        if (tmpblock == NULL) {
            return;
        }
        readFromDisk(tmpblock);
        if (tmpblock->if_end) break;
        tmpblock = bm.getNextBlock(file, tmpblock);
    }
}

template <class KeyType>
void BPlusTree<KeyType>::readFromDisk(blockNode* tmpblock) {
    int valueSize = sizeof(int);
    char* indexBegin = bm.get_content(*tmpblock);
    char* valueBegin = indexBegin + keysize;
    KeyType key;
    int value;
    while (valueBegin - bm.get_content(*tmpblock) < bm.get_usingSize(*tmpblock)) {
        key = *(KeyType*)indexBegin;
        value = *(int*)valueBegin;
        insertKey(key, value);
        valueBegin += keysize + valueSize;
        indexBegin += keysize + valueSize;
    }
}

template <class KeyType>
void BPlusTree<KeyType>::writtenbackToDiskAll() {
    blockNode* tmpblock = bm.getBlockHead(file);
    Node tmpnode = leafhead;
    int valueSize = sizeof(int);
    while (tmpnode != NULL) {
        bm.set_usingSize(*tmpblock, 0);
        bm.set_dirty(*tmpblock);
        for (int i = 0; i < tmpnode->count; i++) {
            char* key = (char*)&(tmpnode->keys[i]);
            char* value = (char*)&(tmpnode->values[i]);
            memcpy(bm.get_content(*tmpblock) + bm.get_usingSize(*tmpblock), key, keysize);
            bm.set_usingSize(*tmpblock, bm.get_usingSize(*tmpblock) + keysize);
            memcpy(bm.get_content(*tmpblock) + bm.get_usingSize(*tmpblock), value, valueSize);
            bm.set_usingSize(*tmpblock, bm.get_usingSize(*tmpblock) + valueSize);
        }
        tmpblock = bm.getNextBlock(file, tmpblock);
        tmpnode = tmpnode->next;
    }
    while (true) {
        if (tmpblock->if_end)
            break;
        bm.set_usingSize(*tmpblock, 0);
        bm.set_dirty(*tmpblock);
        tmpblock = bm.getNextBlock(file, tmpblock);
    }
}

#endif
