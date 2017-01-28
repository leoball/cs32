//  DiskMultiMap.cpp

#include "DiskMultiMap.h"
#include <iostream>

//===============================


//implementation of iterator

DiskMultiMap::Iterator& DiskMultiMap::Iterator::operator++()
{
    if ( !isvalid )
            return *this;                 //check if it is valid
    
    node cur;
    
    node next;        //read the node, and store that in cur and next
    
    bfile->read(cur, m_num);
    m_num = cur.next;
    
    if (m_num == 0)             //means there is no next node, so we can't add
    {
        
        isvalid = false;
        
        return *this;
        
    }
        
        bfile->read(next, m_num);
        while ( strcmp(next.m_key, m_key.c_str()) != 0 )      //iterates into next until find the matching
        {
            m_num = next.next;
            if (m_num == 0)
            {
                isvalid = false;
                return *this;
            }
            bfile->read(next, m_num);
        }
        return *this;
}

MultiMapTuple DiskMultiMap::Iterator::operator*()
{
    
    MultiMapTuple m_dereference;
    
    if ( !isvalid)
        {
            m_dereference.key = "";
            m_dereference.value = "";
            m_dereference.context = "";
        }
    
    else
        {
            node temp;
            bfile->read(temp, m_num);
            m_dereference.key = temp.m_key;
            m_dereference.value = temp.m_value;
            m_dereference.context = temp.m_context;
        }
    
    return m_dereference;
}

    
    




















//==============================
BinaryFile::Offset DiskMultiMap::m_hashFunc(const std::string key, int numOfBuckets)
{
    size_t hash = std::hash<std::string>{} (key);
    unsigned int hashresult = hash % numOfBuckets;
    
    
    return (static_cast<BinaryFile::Offset>(sizeof(header)+hashresult*sizeof(BinaryFile::Offset)));//convert that in to right type 
}


DiskMultiMap::DiskMultiMap()
{}

DiskMultiMap::~DiskMultiMap()
{
    close();
}

bool DiskMultiMap::createNew(const std::string& filename, unsigned int numBuckets)
{
    close();       //close the file that is being opend before
   
    if (filename.size() > 120)
        return false;
    
    if (m_binaryfile.createNew(filename))
    {
        header new_head(numBuckets, 0);
        m_binaryfile.write(new_head, 0);
        
       
       //m_binaryfile.write(numBuckets, 0);
        
        BinaryFile::Offset begin = sizeof(header);
        BinaryFile::Offset max = numBuckets*sizeof(BinaryFile::Offset)+begin;
        
        for( BinaryFile::Offset i = begin; i < max; i += sizeof(BinaryFile::Offset))
            m_binaryfile.write(0, i);
        
        return true;
    }
   
    return false;
}

bool DiskMultiMap::openExisting(const std::string &filename)
{
    close();       //close older file
    
    if (m_binaryfile.openExisting(filename))
        return true;
    
    return false;
}

void DiskMultiMap::close()
{
   m_binaryfile.close();
}


DiskMultiMap::Iterator DiskMultiMap::search(const std::string& key)
{
    header new_head(0, 0);
    
    m_binaryfile.read(new_head, 0);
    
    unsigned int hashValue = m_hashFunc(key, new_head.numOfBuckets);  //find which buckets to look at
    
    unsigned int pos;
    m_binaryfile.read(pos, hashValue);
    
    
    
    if (pos == 0)
        return Iterator();  //return an invalid iterator
    
    node temp;
    m_binaryfile.read(temp, pos);
    
    while ( strcmp(temp.m_key,key.c_str()) != 0 )  //not the same traverse to next
    {
        pos = temp.next;
        if (pos == 0)
            return Iterator();
        
        m_binaryfile.read(temp, pos);
    }
    return Iterator(key,&m_binaryfile,pos);
}

//============================================================
//the function of erase and insert


bool DiskMultiMap::insert(const std::string& key, const std::string& value, const std::string& context)
{
    if (key.size() > 120 || value.size() > 120 || context.size() > 120)
        return false;
    //check the precondition for inserting
    header myHead(0, 0);
    m_binaryfile.read(myHead, 0);
    BinaryFile::Offset hashValue = m_hashFunc(key, myHead.numOfBuckets);//check which bucket to insert
    
    BinaryFile::Offset pos;
    m_binaryfile.read(pos, hashValue);
    
    node newNode(key.c_str(), value.c_str(), context.c_str(), pos, 0);
    
    if (myHead.newEmpty == 0)
    {
        BinaryFile::Offset tempPos = m_binaryfile.fileLength();
        if ( !m_binaryfile.write(newNode, tempPos))
            return false;
        
        if (pos != 0)                      //use the same algorithms like linking the node in linklist
        {
            node in;
            m_binaryfile.read(in, pos);
            in.prev = tempPos;
            m_binaryfile.write(in, pos);
        }
        m_binaryfile.write(tempPos, hashValue);
    }
    else     //we have to reuse the empty space
    {
        node re;
        
        m_binaryfile.read(re, myHead.newEmpty);
        
        if (!m_binaryfile.write(newNode, myHead.newEmpty))
            return false;
        
        if (pos != 0)
        {
            node n;
            m_binaryfile.read(n, pos);
            n.prev = myHead.newEmpty;
            m_binaryfile.write(n, pos);
        }                                  //link with previousnode
        
        pos = myHead.newEmpty;
        myHead.newEmpty = re.next;
        
        m_binaryfile.write(myHead, 0);
        m_binaryfile.write(pos, hashValue);
    }
    return true;
    
    
}


int DiskMultiMap::erase(const std::string& key, const std::string& value, const std::string& context)
{   int num = 0;
    BinaryFile::Offset pos;
    
    header myHead(0, 0);
    
    node newNode;
    
    
    m_binaryfile.read(myHead, 0);
    
    
    BinaryFile::Offset hashValue = m_hashFunc(key, myHead.numOfBuckets); //consider which bucket to erase from
    
    m_binaryfile.read(pos, hashValue);    //store the head pointer to pos
    
   
    BinaryFile::Offset curr = pos;
     BinaryFile::Offset next;    //inorder to link them together
    
    for ( ; curr != 0; curr = next)
    {
        m_binaryfile.read(newNode, curr);
        next = newNode.next;
       
        if (strcmp(newNode.m_key, key.c_str()) == 0 && strcmp(newNode.m_value, value.c_str()) == 0 && strcmp(newNode.m_context, context.c_str()) == 0)  //we find the item needed to erase
        {
            num++;              //add num of erasing
           
            newNode.next = myHead.newEmpty;
            myHead.newEmpty = curr;
            m_binaryfile.write(newNode, curr);
            m_binaryfile.write(myHead, 0); //for test
           
            
            if(next!=0)    // a node after it, link them
            {
                node nextOne;
                m_binaryfile.read(nextOne, next);
                nextOne.prev = newNode.prev;
                m_binaryfile.write(nextOne, next);
            }
            
            if(newNode.prev != 0)//a node before that
            {
                node prevOne;
                m_binaryfile.read(prevOne, newNode.prev);
                prevOne.next = next;
                m_binaryfile.write(prevOne, newNode.prev);
            }
            else
                pos = next;
        }
       
    }
    m_binaryfile.write(pos, hashValue);
    m_binaryfile.write(myHead, 0);
   
    return num;
    }




