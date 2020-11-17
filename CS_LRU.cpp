#include "CS_LRU.h"
CSLRU::CSLRU(int _size){
    this->size = _size;
}
CSLRU::~CSLRU(){

    list<string>::iterator it = lru.begin();
    while (it != lru.end())
    {
        string delstr = *it;
        it++;
        deleteContentDataAndLength(delstr);
    }
    if(CSLRU::cslruInstance){
        delete CSLRU::cslruInstance;
        CSLRU::cslruInstance = NULL;
    }
}

CSLRU* CSLRU::cslruInstance = NULL;
std::mutex cslrumtx;

CSLRU* CSLRU::getInstance(int _size){
    if(cslruInstance == NULL){
        std::lock_guard<mutex> lck(cslrumtx);
        if(cslruInstance == NULL){
            cslruInstance = new CSLRU(_size);
        }
    }
    return cslruInstance;
}

char* CSLRU::getContentData(string name){
    //如果没有这个Content Name
    if(Name2ContentData.find(name) == Name2ContentData.end()){
        return NULL;
    }
    //需要在lru中更新它的位置,放在队首
    if(Name2ContentData.count(name)) lru.erase(Name2Itermap[name]);
    lru.push_front(name);
    Name2Itermap[name] = lru.begin();
    
    return Name2ContentData[name];
}

int CSLRU::getContentLength(string name){
    if(Name2ContentLength.find(name) == Name2ContentLength.end()){
        return -1;
    }

    //因为getContentData这个函数一定要和getContentLength一起执行，那么此函数就不需要执行调整lru了
    return Name2ContentLength[name];

}


void CSLRU::putContentNameAndDataAndLength(string name, char* data, int length){
    //如果之前就有这个ContentName了，那么直接替换掉就可以了。不论当前有多少个元素，直接替换不会使得超过原来的元素数量
    unordered_map<string, char*>::iterator it = Name2ContentData.find(name);
    if(it != Name2ContentData.end()){
        //将此重复元素放在队首
        lru.erase(Name2Itermap[name]);
        lru.push_front(name);
        Name2Itermap[name] = lru.begin();

        //释放原有空间
        char* delContentData = Name2ContentData[name];
        delete []delContentData;
        
        char* newContentData = new char[length];
        memcpy(newContentData, data, length);
        
        //更新数据和长度
        Name2ContentData[name] = newContentData;
        Name2ContentLength[name] = length;    
        return;
    }

    //如果当前数量已经大于lru的额定数量，且与原有的ContentName不重复
    if(Name2ContentData.size() >= size){
        //先查找之前有没有同样的ContentName作为key的
        
        //先删除之前的最后一个
        string delName = lru.back();
        
        //释放之前的char数组空间
        unordered_map<string, char*>::iterator it = Name2ContentData.find(delName);
        if(it != Name2ContentData.end()){
            char* delContentData = it->second;
            delete []delContentData;
        }

        //释放lru中最后出现的这个string
        lru.pop_back();
        //释放delName对应的数组指针
        Name2ContentData.erase(delName);
        //释放delName对应的数组长度
        Name2ContentLength.erase(delName);
        //释放delName对应的迭代器
        Name2Itermap.erase(delName);
    }
    //插入新的数据
    char* newContentData = new char[length];
    memcpy(newContentData, data, length);
    //更新顺序
    lru.push_front(name);
    Name2Itermap[name] = lru.begin();
    //更新数据和长度
    Name2ContentData[name] = newContentData;
    Name2ContentLength[name] = length;
}
    
void CSLRU::deleteContentDataAndLength(string name){
    if(Name2Itermap.count(name)){
        lru.erase(Name2Itermap[name]);
        Name2Itermap.erase(name);
    }

    //释放指针数组的空间
    unordered_map<string, char*>::iterator it = Name2ContentData.find(name);
    if(it != Name2ContentData.end()){
        char* delContentData = it->second;
        delete []delContentData;
    }
    
    Name2ContentData.erase(name);
    Name2ContentLength.erase(name);   
}
/**
 * 比如说一个请求可能请求的是pku/eecs/video/testfile.txt
 * 根据现有的缓存查找出所有合适的缓存，比如说pku/eecs/video/testfile.txt/segment1 pku/eecs/video/testfile.txt/segment2等等。
 */
vector<string> CSLRU::getAllRelatedContentPackage(string name){
    
    vector<string> ContentNameVector;
    vector<string> ret;
    
    //将输入的name按照 / 拆分成多个
    SplitString(name, ContentNameVector);
    
    //这里优化以下查找速度，如果最后一个 / 后面的字段包含segment那么就认为已经是最长匹配目录了，这种情况下已经只有自己
    if(ContentNameVector.size() >= 1 && ContentNameVector[ContentNameVector.size() - 1].find("segment") != string::npos){
        //必须有这个内容才可以
        if(Name2ContentData.find(name) != Name2ContentData.end() && Name2ContentLength.find(name) != Name2ContentLength.end()) ret.push_back(name);
        return ret;
    }
    //遍历lru查找所有符合条件的
    list<string>::iterator it = lru.begin();
    while (it != lru.end())
    {
        if((*it).find(name) != string::npos) ret.push_back(*it);
        it++;
    }
    return ret;
}

void CSLRU::SplitString(string& s, vector<string>& v){

    string c = "/";
    std::string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while(std::string::npos != pos2)
    {
        v.push_back(s.substr(pos1, pos2-pos1));
 
        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    if(pos1 != s.length())
        v.push_back(s.substr(pos1));
}

void CSLRU::printCSLRU(){
    cout << "==========================" << endl;
    list<string>::iterator it = lru.begin();
    while (it != lru.end())
    {
        cout << "Name: " << *it << " ContentData: " << Name2ContentData[*it] << " ContentLength: " << Name2ContentLength[*it] << endl;
        it++;
    }
    cout << Name2ContentData.size() << " " << Name2ContentLength.size() << " " << Name2Itermap.size() << endl;
    cout << "==========================" << endl;
}