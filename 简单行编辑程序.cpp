#include<bits/stdc++.h>
using namespace std;

//全局常量配置
const int ACTIVE_MAX_LEN=100;//活区最大行数
const int PAGE_SIZE=20;//每页显示行数
const int INIT_LOAD_LEN=ACTIVE_MAX_LEN-20;//初始加载最大行数（留20行插入空间）
vector<string> activeArea;//活区存储：每行文本
int activeStartLine=1;//活区起始行号
ifstream inFile;//输入文件流
ofstream outFile;//输出文件流
string inputFileName;//输入文件名
string outputFileName;//输出文件名


//函数声明
bool initActiveArea();//初始化活区
void printActiveArea();//显示活区
bool insertLine(int lineNo,const string& text);//插入行
bool deleteLines(int line1,int line2=-1);//删除行
bool switchActiveArea();//活区切换
void parseCommand();//解析用户命令
void closeFiles();//关闭文件

//主函数
int main(){
    cout<<"========== 简单行编辑程序 =========="<<endl;
    //初始化活区
    if(!initActiveArea()){
        cout<<"程序初始化失败，退出！"<<endl;
        return 0;
    }
    cout<<"========== 命令说明 =========="<<endl;
    cout<<"1. 插入：i<行号>回车→输入文本→.回车（例：i5回车Hello.回车）"<<endl;
    cout<<"2. 删除：d<行号1> <行号2>回车（例：d3 回车/ d3 5 回车）"<<endl;
    cout<<"3. 活区切换：n回车（写入当前活区，加载下一段）"<<endl;
    cout<<"4. 显示活区：p回车"<<endl;
    cout<<"5. 退出程序：q回车"<<endl;
    cout<<"==============================="<<endl;

    parseCommand();
    closeFiles();
    cout<<"程序已退出，所有修改已保存到输出文件！"<<endl;
    return 0;
}


//1.初始化活区
bool initActiveArea(){
    //获取文件名
    cout<<"请输入输入文件名（空串表示无输入文件，直接创建新文件）：";
    getline(cin,inputFileName);
    cout<<"请输入输出文件名（不存在则自动创建）：";
    getline(cin,outputFileName);
    if(inputFileName==outputFileName && !inputFileName.empty()){
        cerr<<"错误：输入文件名和输出文件名不能相同！"<<endl;
        return false;
    }

    //打开输入文件
    if(!inputFileName.empty()){
        inFile.open(inputFileName);
        if(!inFile.is_open()){
            cerr<<"错误：输入文件 '"<<inputFileName<<"' 打开失败（文件不存在或权限不足）！"<<endl;
            return false;
        }
    }

    //打开输出文件
    outFile.open(outputFileName,ios::app);
    if(!outFile.is_open()){
        cerr<<"错误：输出文件 '"<<outputFileName<<"' 创建/打开失败！"<<endl;
        return false;
    }
    cout<<"成功创建/打开输出文件："<<outputFileName<<endl;

    //加载初始活区
    if(!inputFileName.empty()){
        string line;
        while(activeArea.size()<INIT_LOAD_LEN && getline(inFile,line)){
            activeArea.push_back(line);
        }
        cout<<"从输入文件加载 "<<activeArea.size()<<" 行到活区"<<endl;
    }
	else{
        cout<<"无输入文件，活区初始为空"<<endl;
    }
    return true;
}


//2. 显示活区
void printActiveArea(){
    if(activeArea.empty()){
        cout<<"活区无内容！"<<endl;
        return;
    }

    int totalLines=activeArea.size();
    int currentPage=0;
    while(true){
        //计算当前页的起始和结束行索引
        int startIdx=currentPage*PAGE_SIZE;
        if(startIdx>=totalLines)break;
        int endIdx=min((currentPage+1)*PAGE_SIZE-1,totalLines-1);

        //显示当前页内容
        cout<<"\n===== 活区第 "<<(currentPage+1)<<" 页（共 "<<((totalLines+PAGE_SIZE-1)/PAGE_SIZE)<<" 页）====="<<endl;
        for(int i=startIdx;i<=endIdx;i++){
            int lineNo=activeStartLine+i;//实际行号 = 活区起始行号 + 索引
            printf("%4d %s\n",lineNo,activeArea[i].c_str());//4位固定行号
        }

        //询问用户是否继续显示下一页
        if(endIdx==totalLines-1)break;//已显示最后一页
        cout<<"\n是否继续显示下一页？（y/n）：";
        char choice;
        cin>>choice;
        cin.ignore();//忽略输入后的换行符
        if(choice!='y'&&choice!='Y')break;
        currentPage++;
    }
}


//3. 插入行
bool insertLine(int lineNo,const string& text){
    //校验行号合法性
    int maxActiveLine=activeStartLine+activeArea.size()-1;
    if(lineNo<activeStartLine-1 || lineNo>maxActiveLine){
        cerr<<"错误：插入行号 "<<lineNo<<" 非法（合法范围："<<activeStartLine-1<<" ~ "<<maxActiveLine<<"）"<<endl;
        return false;
    }

    //处理活区满的情况：插入后行数不能超过 ACTIVE_MAX_LEN
    if(activeArea.size()>=ACTIVE_MAX_LEN){
        cout<<"警告：活区已满，自动输出一行到文件以腾出空间..."<<endl;
        //插入点在第一行之前：输出新插入的行（特殊情况）
        if(lineNo==activeStartLine-1){
            outFile<<text<<endl;
            return true;
        }
        //插入点在其他位置：输出插入点之前的第一行
        outFile<<activeArea[0]<<endl;
        activeArea.erase(activeArea.begin());//删除活区第一行
        activeStartLine++;//活区起始行号+1（后续行号同步）
    }

    //计算插入索引
    int insertIdx=(lineNo==activeStartLine-1)?0:(lineNo-activeStartLine+1);
    activeArea.insert(activeArea.begin()+insertIdx,text);
    cout<<"成功插入行："<<text<<endl;
    return true;
}


//4. 删除行
bool deleteLines(int line1,int line2){
    //处理单行删除（line2 未指定）
    if(line2==-1)line2=line1;

    //校验行号合法性：line1 <= line2，且都在活区范围内
    int minActiveLine=activeStartLine;
    int maxActiveLine=activeStartLine+activeArea.size()-1;
    if(line1<minActiveLine||line2>maxActiveLine||line1>line2){
        cerr<<"错误：删除行号非法（合法范围："<<minActiveLine<<" ~ "<<maxActiveLine<<"，且起始行<=结束行）"<<endl;
        return false;
    }

    //计算删除的索引范围（转换为 vector 索引）
    int startIdx=line1-activeStartLine;
    int endIdx=line2-activeStartLine;
    int deleteCount=endIdx-startIdx+1;

    //执行删除
    activeArea.erase(activeArea.begin()+startIdx,activeArea.begin()+endIdx+1);
    cout<<"成功删除 "<<deleteCount<<" 行（行号："<<line1<<" ~ "<<line2<<"）"<<endl;
    return true;
}


//5. 活区切换
bool switchActiveArea(){
    cout<<"正在执行活区切换..."<<endl;

    //将当前活区所有内容写入输出文件
    for(const string& line:activeArea){
        outFile<<line<<endl;
    }
    cout<<"当前活区 "<<activeArea.size()<<" 行已写入输出文件"<<endl;

    //清空当前活区，准备加载下一段
    activeArea.clear();

    //从输入文件加载下一段
    if(!inFile.eof()){
        string line;
        int loadCount=0;
        const int keepLines=10;
        if(keepLines>0&&!activeArea.empty()){
            vector<string> keepVec(activeArea.end()-keepLines,activeArea.end());
            activeArea.swap(keepVec);
        }
        while(activeArea.size()<ACTIVE_MAX_LEN&&getline(inFile,line)){
            activeArea.push_back(line);
            loadCount++;
        }
        activeStartLine=activeStartLine+(ACTIVE_MAX_LEN-loadCount);//更新起始行号
        cout<<"从输入文件加载 "<<loadCount<<" 行到新活区"<<endl;
    }
	else{
        cout<<"输入文件已读完，新活区为空"<<endl;
        activeStartLine=1;//重置起始行号
    }

    printActiveArea();
    return true;
}


//6. 解析用户命令
void parseCommand(){
    string command;
    while(true){
        cout<<"\n请输入命令（输入 q 退出）：";
        getline(cin,command);
        if(command.empty())continue;
        char cmdType=command[0];//抓取命令 
        string cmdParam=command.substr(1);//命令参数

        //退出命令：q
        if(cmdType=='q' || cmdType=='Q'){
            break;
        }

        //显示命令：p
        else if(cmdType=='p' || cmdType=='P'){
            printActiveArea();
        }

        //活区切换命令：n
        else if(cmdType=='n' || cmdType=='N'){
            switchActiveArea();
        }

        //删除命令：d<行号1> [行号2]
        else if(cmdType=='d' || cmdType=='D'){
            int line1=0,line2=-1;
            //解析参数：支持 "d3"（单行）或 "d3 5"（多行）
            if(sscanf(cmdParam.c_str(),"%d %d",&line1,&line2)==1){
                line2=-1;//仅解析到 line1，设为单行删除
            }
            deleteLines(line1,line2);
            printActiveArea();//删除后自动显示活区
        }

        //插入命令：i<行号> → 输入文本 → . 结束
        else if(cmdType=='i' || cmdType=='I'){
            int lineNo=0;
            //解析插入行号
            if(sscanf(cmdParam.c_str(),"%d",&lineNo)!=1){
                cerr<<"错误：插入命令格式错误（正确格式：i<行号>回车）"<<endl;
                continue;
            }
            cout<<"请输入插入文本（输入 . 回车结束）：";
            string text;
            while(true){
                getline(cin,text);
                if(text==".")break;//输入 . 结束插入
                insertLine(lineNo,text);
                lineNo++;//多行插入时，后续行插入到上一行之后
            }
            printActiveArea();//插入后自动显示活区
        }

        //未知命令
        else{
            cerr<<"错误：未知命令！请输入正确命令（i/d/n/p/q）"<<endl;
        }
    }
}

//7. 关闭文件
void closeFiles(){
    if(inFile.is_open()){
        inFile.close();
    }
    if(outFile.is_open()){
        outFile.close();
    }
}
