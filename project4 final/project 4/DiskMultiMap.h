//  Created by li yang on 3/5/16.
//  Copyright © 2016 li yang. All rights reserved.
//


#ifndef DISKMULTIMAP_H
#define DISKMULTIMAP_H

#include <string>
#include <cstring>
#include "MultiMapTuple.h"
#include "BinaryFile.h"

class DiskMultiMap
{
public:
    
    class Iterator
    {
    public:
        Iterator()  {isvalid=false;}
        Iterator(const std::string key,BinaryFile* bf,int num)
        : m_num(num), bfile(bf), m_key(key)
        {
            if (num == 0)
                isvalid = false;
            else
                isvalid= true;
        }
        
        bool isValid() const { return isvalid; }
        
        Iterator& operator++();
        
        MultiMapTuple operator*();
        
    private:
        bool isvalid;
        unsigned int m_num;
        BinaryFile* bfile;
        std::string m_key;
    };
    
    DiskMultiMap();
    ~DiskMultiMap();
    bool createNew(const std::string& filename, unsigned int numBuckets);
    bool openExisting(const std::string& filename);
    void close();
    bool insert(const std::string& key, const std::string& value, const std::string& context);
    Iterator search(const std::string& key);
    int erase(const std::string& key, const std::string& value, const std::string& context);
    
private:
   
    struct header     //header is used to keep track of the number of buckets and availble space to insert
    {
        header(int numOfBucket, BinaryFile::Offset index): numOfBuckets(numOfBucket), newEmpty(index)
        {}
        int numOfBuckets;
        BinaryFile::Offset newEmpty;
    };
    
    struct node
    {//constructor for node
        node(const char* key="", const char* value="", const char* context="", BinaryFile::Offset n=0, BinaryFile::Offset m=0)
        {
            strcpy(m_key, key);
            strcpy(m_value, value);
            strcpy(m_context, context);
            next=n;
            prev=m;
        }
        char m_key[121];
        char m_value[121];
        char m_context[121];
        BinaryFile::Offset next;
        BinaryFile::Offset prev;
    };
    
    BinaryFile::Offset m_hashFunc(const std::string key,int numOfBuckets);

    
    
    BinaryFile m_binaryfile;
    
   };

#endif // DISKMULTIMAP_H
