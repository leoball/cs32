//
//  IntelWeb.hpp
//  project 4
//
//  Created by li yang on 3/8/16.
//  Copyright © 2016 li yang. All rights reserved.
//



#ifndef INTELWEB_H_
#define INTELWEB_H_

#include "InteractionTuple.h"
#include <string>
#include <vector>
#include"DiskMultiMap.h"
#include<set>
#include<cstring>
inline
bool operator<(const InteractionTuple& a,const InteractionTuple& b)
{
    if(a.context!=b.context)
    return a.context<b.context;
  if(a.from!=b.from)
      return a.from<b.from;
   
    return a.to<b.to;
    
}
class IntelWeb
{
public:
    IntelWeb();
    ~IntelWeb();
    bool createNew(const std::string& filePrefix, unsigned int maxDataItems);
    bool openExisting(const std::string& filePrefix);
    void close();
    bool ingest(const std::string& telemetryFile);
    unsigned int crawl(const std::vector<std::string>& indicators,
                       unsigned int minPrevalenceToBeGood,
                       std::vector<std::string>& badEntitiesFound,
                       std::vector<InteractionTuple>& badInteractions
                       );
    bool purge(const std::string& entity);
    
private:
    DiskMultiMap m_DiskmultiMapforward;
    DiskMultiMap m_DiskmultiMapreverse;
    unsigned long prevalence(const std::string& key)
    {
        int num = 0;
        for (auto itr = m_DiskmultiMapforward.search(key); itr.isValid(); ++itr)
        {
            num++;
        }
        
        for (auto itr = m_DiskmultiMapreverse.search(key); itr.isValid(); ++itr)
        {
            num++;
        }
        
        return num;  //used for test only
        
    }
  
       
};


#endif // INTELWEB_H_



