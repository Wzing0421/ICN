#include "CS_LRU.h"
CSLRU::CSLRU(int _size){
    this->size = _size;
    /*我在这里加上一些测试语句方便测试，之后删除11月19日*/
    /*
    string str1 = "pku/eecs/file/test1.txt/segment1";
    char *strcontent1 = "teststrlength=16";
    DataPackage package1(str1.c_str(), strcontent1, strlen(strcontent1), 1, 0);

    string str2 = "pku/eecs/file/test1.txt/segment2";
    char *strcontent2 = "testteststrlength=20";
    DataPackage package2(str2.c_str(), strcontent2, strlen(strcontent2), 1, 0);

    string str3 = "pku/eecs/file/test1.txt/segment3";
    char *strcontent3 = "testteststrlength=28";
    DataPackage package3(str3.c_str(), strcontent3, strlen(strcontent3), 1, 1);

    string str4 = "pku/eecs/file/test2.txt/segment2";
    char *strcontent4 = "testteststrlength=22";
    DataPackage package4(str4.c_str(), strcontent4, strlen(strcontent4), 1, 1);

    string str5 = "pku/eecs/file/test2.txt/segment1";
    char *strcontent5 = "testteststrlength=23";
    DataPackage package5(str5.c_str(), strcontent5, strlen(strcontent5), 1, 0);
    
    putContentNameAndDataAndLengthNoLock(package1);
    putContentNameAndDataAndLengthNoLock(package2);
    putContentNameAndDataAndLengthNoLock(package3);
    putContentNameAndDataAndLengthNoLock(package4);
    putContentNameAndDataAndLengthNoLock(package5);
    */

    pitInstance = PIT::GetInstance();
}

CSLRU::~CSLRU(){
    //我还不能确定这么写对不对
    while (lru.size() > 0)
    {
        string delstr = lru.front();
        deleteAllDataPackageByUpperName(delstr);        
    }
    if(CSLRU::cslruInstance){
        delete CSLRU::cslruInstance;
        CSLRU::cslruInstance = NULL;
    }
}

CSLRU* CSLRU::cslruInstance = NULL;
std::mutex cslrumtx;

CSLRU* CSLRU::GetInstance(int _size){
    if(cslruInstance == NULL){
        std::lock_guard<mutex> lck(cslrumtx);
        if(cslruInstance == NULL){
            cslruInstance = new CSLRU(_size);
        }
    }
    return cslruInstance;
}

void CSLRU::putContentNameAndDataAndLength(DataPackage datapack){
    
    std::lock_guard<mutex> putContentlck(cslrumtx);
    
    //1. 先取得UpperName
    string name = getUpperName((string)datapack.contentName);
    if(name.empty()){
        cout << "[Error] Invalid Content Name, name is " << datapack.contentName << endl;
        return;
    }
    //2. 查询这个UpperName(也就是这个name)
    unordered_map<string, unordered_set< DataPackage, package_hash_cs>>::iterator it = Name2ContentData.find(name);
    if(it != Name2ContentData.end()){
        //将此name放在lru的队首表示访问过这个name
        lru.erase(Name2Itermap[name]);
        lru.push_front(name);
        Name2Itermap[name] = lru.begin();

        /**
         * 插入这条数据. 这里插入的原则是：因为全局命名唯一，那么只要收到了一个datapack之后就需要查找有无．
         * 如果有则替换，无则插入即可
         */
        auto iter = Name2ContentData[name].find(datapack);
        if(iter != Name2ContentData[name].end()) {
            Name2ContentData[name].erase(iter);
        }
        Name2ContentData[name].insert(datapack);
        return;
    }
    //以下是没有的情况，分成超过额定size和没有超过来讨论
    //如果当前数量已经大于lru的额定数量，且与原有的ContentName不重复
    if(Name2ContentData.size() >= size){
        
        //先删除之前的最后一个
        string delName = lru.back();
        
        //释放资源
        //1)释放之前的DataPack数组空间
        unordered_map<string, unordered_set< DataPackage, package_hash_cs>>::iterator it = Name2ContentData.find(delName);
        if(it != Name2ContentData.end()){
            Name2ContentData.erase(delName);
        }
        //2)释放lru中最后出现的这个string,注意lru中存放的都是UpperName
        lru.pop_back();
        //3)释放delName对应的迭代器
        Name2Itermap.erase(delName);
        //4)删除PIT表中这一项等待的所有源端
        deleteContentDataInPIT(delName);

    }
    //插入新的DataPackage
    // 1)先更新lru的顺序
    lru.push_front(name);
    Name2Itermap[name] = lru.begin();
    // 2)再更新数据包
    Name2ContentData[name].insert(datapack);
}

/**
 * 这个的粒度是upperName,　也就是 pku/eecs/file/test.txt, 因为会释放掉这个文件下面的所有的segment包    
 */
void CSLRU::deleteAllDataPackageByUpperName(string name){

    std::lock_guard<mutex> deleteContentlck(cslrumtx);
    if(Name2Itermap.count(name)){
        lru.erase(Name2Itermap[name]);
        Name2Itermap.erase(name);
    }

    //释放这个upperName下面的所有segment包空间
    auto it = Name2ContentData.find(name);
    if(it != Name2ContentData.end()){
        Name2ContentData.erase(it);
    }   
}

/**
 * 比如说一个请求ContentName可能请求的是pku/eecs/video/testfile.txt 也有可能是pku/eecs/video/testfile.txt/segment1
 * 根据上面不同的ContentName来用不同的方式来获得对应的数据包
 * 对于文件粒度，就获得所有的下面的segment包; 对于segment粒度，则直接获取到这个粒度的包就行
 */
vector<DataPackage> CSLRU::getAllRelatedContentPackage(string name){
    //这里有点问题：我没有把它获得的stringname 放在lru的头部
    //这里不需要加锁，因为子函数里面加了锁
    vector<DataPackage> ret;
    
    if(name.find("segment") != string::npos){
        //以包名为粒度
        return getDataPackageBySegmentName(name);
    }
    else{
        //这个包以文件为粒度
        return getAllDataPackageByUpperName(name);
    }
}

vector<DataPackage> CSLRU::getAllDataPackageByUpperName(string upperName){
    std::lock_guard<mutex> getAllDataPackagelck(cslrumtx);
    vector<DataPackage> ret;
    auto it = Name2ContentData.find(upperName);
    if(it == Name2ContentData.end()){
        return ret;
    }
    unordered_set< DataPackage, package_hash_cs> packset = it->second;
    for(auto setIter = packset.begin(); setIter != packset.end(); setIter++){
        ret.push_back(*setIter);
    }
    sort(ret.begin(), ret.end());
    return ret;
}

vector<DataPackage> CSLRU::getDataPackageBySegmentName(string name){
    
    vector<DataPackage> ret;
    string upperName = getUpperName(name);
    if(upperName.empty()) return ret;

    std::lock_guard<mutex> getDataPackagelck(cslrumtx);
    auto it = Name2ContentData.find(upperName);
    if(it == Name2ContentData.end()) return ret;

    unordered_set< DataPackage, package_hash_cs> contentset = it->second;
    for(auto setIter = contentset.begin(); setIter != contentset.end(); setIter++){
        string compareName = (string)(setIter->contentName);
        if(name == compareName){
            ret.push_back(*setIter);
            break;
        }
    }
    sort(ret.begin(), ret.end());
    return ret;
}

bool CSLRU::IsDataPackageInContentStore(DataPackage datapack){
    std::lock_guard<mutex> judgeInlck(cslrumtx);
    string name = datapack.contentName;
    string UpperName = getUpperName(name);
    auto it = Name2ContentData.find(UpperName);
    if(it == Name2ContentData.end()) return false;
    auto iter = Name2ContentData[UpperName].find(datapack);
    return iter != Name2ContentData[UpperName].end();
}

bool CSLRU::IsDataPackageInContentStore(string name){
    if(name.find("segment") != string::npos){
        //以包名为粒度
        return IsContentNameInContentStoreBySegmentName(name);
    }
    else{
        //这个包以文件为粒度
        return IsContentNameInContentStoreByUpperName(name);
    }
}

bool CSLRU::IsContentNameInContentStoreBySegmentName(string name){
    string upperName = getUpperName(name);
    if(upperName.empty()) return false;
    
    std::lock_guard<mutex> judgeInlckBySegmentName(cslrumtx);
    if(Name2ContentData.find(upperName) == Name2ContentData.end()) return false;
    
    //这实际上是一个数据空包，这个包仅仅用于map的查询
    string data = "Inquire";
    DataPackage datapack(name.c_str(), data.c_str(), data.size(), 0, 0);
    return Name2ContentData[upperName].find(datapack) != Name2ContentData[upperName].end();
}

bool CSLRU::IsContentNameInContentStoreByUpperName(string name){
    std::lock_guard<mutex> judgeInlckByUpperName(cslrumtx);
    return Name2ContentData.find(name) != Name2ContentData.end();
}

void CSLRU::printCSLRU(){
    cout << "=======================================" << endl;
    //输出lru则按照upperName下面来输出
    list<string>::iterator it = lru.begin();
    while (it != lru.end())
    {
        cout << "------------"<< *it << "-------------" << endl;
        //按照upperName分级输出
        string upperName = *it;
        unordered_set< DataPackage, package_hash_cs> mySet = Name2ContentData[upperName];
        for(auto setIter = mySet.begin(); setIter != mySet.end(); setIter++){
            cout << "ContentName: " << setIter->contentName << " Data: " << setIter->data << " SegmentNum: " << setIter->segmentNum << " IsEnd: " << setIter->end << endl;
        }
        it++;
    }
    cout << "=======================================" << endl;
}

//这个函数在构造函数中使用．开发结束就删除．
void CSLRU::putContentNameAndDataAndLengthNoLock(DataPackage datapack){
    //1. 先取得UpperName
    string name = getUpperName((string)datapack.contentName);
    if(name.empty()){
        cout << "[Error] Invalid Content Name, name is " << datapack.contentName << endl;
        return;
    }
    //2. 查询这个UpperName(也就是这个name)
    unordered_map<string, unordered_set< DataPackage, package_hash_cs>>::iterator it = Name2ContentData.find(name);
    if(it != Name2ContentData.end()){
        //将此name放在lru的队首表示访问过这个name
        lru.erase(Name2Itermap[name]);
        lru.push_front(name);
        Name2Itermap[name] = lru.begin();

        /**
         * 插入这条数据. 这里插入的原则是：因为全局命名唯一，那么只要收到了一个datapack之后就需要查找有无．
         * 如果有则替换，无则插入即可
         */
        auto iter = Name2ContentData[name].find(datapack);
        if(iter != Name2ContentData[name].end()) {
            Name2ContentData[name].erase(iter);
        }
        Name2ContentData[name].insert(datapack);
        return;
    }
    //以下是没有的情况，分成超过额定size和没有超过来讨论
    //如果当前数量已经大于lru的额定数量，且与原有的ContentName不重复
    if(Name2ContentData.size() >= size){
        
        //先删除之前的最后一个
        string delName = lru.back();
        
        //释放资源
        //1)释放之前的DataPack数组空间
        unordered_map<string, unordered_set< DataPackage, package_hash_cs>>::iterator it = Name2ContentData.find(delName);
        if(it != Name2ContentData.end()){
            Name2ContentData.erase(delName);
        }
        //2)释放lru中最后出现的这个string,注意lru中存放的都是UpperName
        lru.pop_back();
        //3)释放delName对应的迭代器
        Name2Itermap.erase(delName);
    }
    //插入新的DataPackage
    // 1)先更新lru的顺序
    lru.push_front(name);
    Name2Itermap[name] = lru.begin();
    // 2)再更新数据包
    Name2ContentData[name].insert(datapack);
}

string CSLRU::getUpperName(string name){
    string upperName = "";
    size_t position = name.find("segment");
    if(position == string::npos){
        return upperName;
    }
    return name.substr(0, position - 1);
}

void CSLRU::deleteContentDataInPIT(string name){
    pitInstance->deleteContentName(name);
}