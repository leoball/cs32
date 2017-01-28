//
//  IntelWeb.cpp
//  project 4
//
//  Created by li yang on 3/8/16.
//  Copyright © 2016 li yang. All rights reserved.
//

#include "IntelWeb.h"
#include <iostream> // needed for any I/O
#include <fstream>  // needed in addition to <iostream> for file I/O
#include <sstream>  // needed in addition to <iostream> for string stream I/O
#include <string>
#include <cstring>
#include"DiskMultiMap.h"
#include <unordered_map>
#include<queue>
using namespace std;




IntelWeb::IntelWeb()
{
    
}
IntelWeb::~IntelWeb()
{
    m_DiskmultiMapforward.close();
    m_DiskmultiMapreverse.close();
    
}
bool IntelWeb::createNew(const std::string &filePrefix, unsigned int maxDataItems)
{
    //close();
   if( m_DiskmultiMapforward.createNew(filePrefix+"forward", static_cast<unsigned int>(maxDataItems*4/3))&&m_DiskmultiMapreverse.createNew(filePrefix+"reverse",static_cast<unsigned int>(maxDataItems*4/3)))
       return true;
    
    return false;
}

bool IntelWeb::openExisting(const std::string &filePrefix)
{
    if(m_DiskmultiMapforward.openExisting(filePrefix+"forward")&&m_DiskmultiMapreverse.openExisting(filePrefix+"reverse"))
    {
      return true;
    }
    else
    {
        close();
       return false;
    }
    
}
void IntelWeb::close()
{
    m_DiskmultiMapforward.close();
    m_DiskmultiMapforward.close();
}

bool IntelWeb::ingest(const std::string &telemetryFile)
{
    // Open the file for input
    ifstream inf(telemetryFile);
		  // Test for failure to open
    if ( ! inf)
    {
      
        return false;
    }
    
    
    string line;
    while (getline(inf, line))
    {
       
        istringstream iss(line);
       
        string key;
        string value;
        string context;
        
               if ( ! (iss >>context >>key>>value) )
        {
            continue;
        }
        
        char dummy;
        if (iss >> dummy)
            continue;
        
     if(!m_DiskmultiMapforward.insert(key, value, context)|| !m_DiskmultiMapreverse.insert(value,key , context))
         return false;


    }
    return true;
    
    
}
unsigned int IntelWeb::crawl(const std::vector<std::string> &indicators, unsigned int minPrevalenceToBeGood, std::vector<std::string> &badEntitiesFound, std::vector<InteractionTuple> &badInteractions)
{
   
    badInteractions.clear();
    badEntitiesFound.clear();
    
    queue<string> badqueue;
    
    set<string> badset;
    set<string> checked;  //to store the value that being checked
    set<InteractionTuple> bad_interaction;
   
    
    for(auto itr=indicators.begin();itr!=indicators.end();itr++)
    {
        
        badqueue.push(*itr);
    }                           //insert all the bad into queue and set
   
    while(!badqueue.empty())
    {
        string bad=badqueue.front();
        badqueue.pop();        //pop out the item from queue
        
        if(!checked.insert(bad).second) //check if the poped item is being checked
               continue;
        
        unsigned long preval=prevalence(bad);
       
        if(preval>=minPrevalenceToBeGood||preval==0)  //means already checked
                continue;
        
        badset.insert(bad);
        
        auto itr=m_DiskmultiMapforward.search(bad);
        
        for(;itr.isValid();++itr)
       {
           InteractionTuple a((*itr).key, (*itr).value, (*itr).context);
        
           bad_interaction.insert(a);  //store the relationship as always needed
       
           badqueue.push((*itr).value);
        
       }
            

        //find reverse order
        
        itr=m_DiskmultiMapreverse.search(bad);
        
        
        for(;itr.isValid();++itr)
        {
            
            InteractionTuple a((*itr).value, (*itr).key, (*itr).context);
            bad_interaction.insert(a); //store the relationship as always needed
            
            badqueue.push((*itr).value);
        }
            
            
        
    }
    //copy the value from set to vector
    unsigned int bad_size=0;
    
    for(auto itr=badset.begin();itr!=badset.end();itr++)
    {
        bad_size++;
        badEntitiesFound.push_back(*itr);
    }
   
    for(auto itr=bad_interaction.begin();itr!=bad_interaction.end();itr++)
    {
        badInteractions.push_back(*itr);
    }
    
    return bad_size;
    
}
bool IntelWeb::purge(const std::string &entity)
{
    vector<MultiMapTuple> v;
    int num=0;
  
    for (auto p = m_DiskmultiMapforward.search(entity); p.isValid(); ++p)
        v.push_back(*p);
    
    for(auto i=v.begin();i!=v.end();i++)
    {
        num+=m_DiskmultiMapforward.erase((*i).key, (*i).value, (*i).context);
        num+=m_DiskmultiMapreverse.erase((*i).value, (*i).key, (*i).context);
    
    }
    
    v.clear();
    
    for (auto p = m_DiskmultiMapreverse.search(entity); p.isValid(); ++p)
        v.push_back(*p);
    
    for(auto i=v.begin();i!=v.end();i++)
    {
        num+=m_DiskmultiMapforward.erase((*i).value, (*i).key, (*i).context);
        num+=m_DiskmultiMapreverse.erase((*i).key, (*i).value, (*i).context);
        
    }
    
    if(num>0)
        return true;
    else
        return false;
}
    
    
    
    
    


