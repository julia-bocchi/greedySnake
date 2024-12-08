#include <iostream>
#include "emoji.h"
#include <windows.h>//头文件
#include <conio.h>//getch
#include <random>
#include <cmath>
#include <ctime> // 添加ctime库来使用time函数
#include<stdio.h>
#include<Windows.h>
#include<mmsystem.h>
#pragma comment(lib,"winmm.lib")//音频头文件
using namespace std;

//创建蛇蛇结构体
typedef struct snake{
    int x;
    int y;
    struct snake *tail;
}greedySnake;
//生成随机数
class CustomRandom {
public:
    CustomRandom(unsigned int seed) : seed(seed) {}
    int next() {
        seed = seed * 1103515245 + 12345;
        return (seed / 65536) % 32768;
    }
private:
    unsigned int seed;
};
//移动方向
enum arrow{Up,Down,Left,Right};
//创建全局
int n[2] , m = 1;
greedySnake *Snake = nullptr;
arrow direction = Up;
int food_x[2], food_y[2];
int coordinate[1000][1000];
int tempcoor[2]={0};
int point = 0;
string restart = "yes";
int volume = 750;
// 文字居中
void middle(const std::string& s) { // 使用宽字符串
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO bInfo;
    GetConsoleScreenBufferInfo(hOutput, &bInfo); // 获取窗口长度
    int len = (bInfo.srWindow.Right - bInfo.srWindow.Left) / 2 - s.size() / 2; // 计算需要空出的格数

    // 移动光标到正确的位置
    COORD pos = { (SHORT)(bInfo.srWindow.Left + len), bInfo.dwCursorPosition.Y };
    SetConsoleCursorPosition(hOutput, pos);

    // 输出宽字符串
    cout<<s;
}
// 控制颜色
void SetConsoleColor(WORD color) {
    
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}
// 设置控制台文本属性为默认值
void ResetConsoleColor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    // 默认颜色为黑色背景（0）和白色前景（7）
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}
//隐藏光标
void HideCursor() {
    CONSOLE_CURSOR_INFO cursorInfo;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;//隐藏
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}
//打印墙体和分数
void printWall(int n[]){
    //i表示y轴即n[1],j表示x轴即n[0]
    for(int i = 0; i < n[1]; i++){//当x轴打印完，再打印y轴
            for(int j = 0; j < n[0]; j++){//先打印x轴
                if(j == 0){//当为第一个时移动光标，使矩形居中
                    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
                    CONSOLE_SCREEN_BUFFER_INFO bInfo;
                    GetConsoleScreenBufferInfo(hOutput, &bInfo);//获取窗口长度
                    int len = bInfo.dwSize.X / 2 - n[0] / 2;//空多少个格
                    printf("\033[%dC" , len);//光标右移
                }
                // 如果是第一行、最后一行、第一列或最后一列，则打印'#'
                if(i == 0 || i == n[1] - 1 || j == 0 || j == n[0] - 1){
                    cout << "#";
                    coordinate[j][i] = 1;
                } else {
                cout << " "; // 否则打印空格，形成空心矩形
            }
        }
        cout << endl; // 每打印完一行后换行
    }
}
void printpoint(){
        //打印分数
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO bInfo;
    GetConsoleScreenBufferInfo(hOutput, &bInfo);//获取窗口长度
    int len = bInfo.dwSize.X / 2 + n[0] / 2;//空多少个格
    // 移动光标到正确的位置
    COORD pos = { static_cast<SHORT>(bInfo.srWindow.Left + len), static_cast<SHORT>(bInfo.srWindow.Top) };
    SetConsoleCursorPosition(hOutput, pos);
    cout << "  当前分数：" << point << endl;
    pos.Y = static_cast<SHORT>(bInfo.srWindow.Top + 2);
    SetConsoleCursorPosition(hOutput, pos);
    cout << "  移动：wasd" << endl;
    pos.Y = static_cast<SHORT>(bInfo.srWindow.Top + 3);
    SetConsoleCursorPosition(hOutput, pos);
    cout << "  音量：⬆增加音量，⬇减少音量" << endl;
    pos.Y = static_cast<SHORT>(bInfo.srWindow.Top + 4);
    SetConsoleCursorPosition(hOutput, pos);
    cout << "  暂停： ESC" << endl;
}
//创建蛇蛇
greedySnake *creatSnake(){
    greedySnake *head = new greedySnake;
    greedySnake *node = head,*Tail = head;
    int x= n[0]/2,y=n[1]/2;//获取坐标，使蛇在中间生成
    for(int i = 0;i < 3;i++){
        node->x= x;
        node->y = y++;//使蛇是竖直的
        node->tail = (i < 2) ? new greedySnake : nullptr;//给指针赋值
        Tail = node;
        node = node->tail;//改变node指向
    }
    return head;//返回头节点
}
//打印蛇蛇
void printSnake() {
    SetConsoleOutputCP(65001);//utf-8输出
    //生成随机数实现颜色变化
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, 15);
    int random_number = distrib(gen);
    SetConsoleColor(random_number);

    greedySnake *h = Snake;//获取头节点
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO bInfo;
    GetConsoleScreenBufferInfo(hOutput, &bInfo); // 获取窗口长度

    bool isHead = true;//检查是否为头节点
    greedySnake *lastNode = nullptr; // 用于记录最后一个节点
    while (h) {
        int len = (bInfo.dwSize.X / 2) - n[0]/2 + h->x; // 获取打印位置
        COORD pos = { (SHORT)len, (SHORT)h->y }; // 光标位置
        SetConsoleCursorPosition(hOutput, pos); // 设置光标位置
        if (isHead) {
            std::cout << emojicpp::emojize(":spades:") << std::endl; //尖
            isHead = false;
        } else {
            std::cout << "○" << std::endl; // ○形
            coordinate[h->x][h->y] = 1;
            tempcoor[0] = h->x;
            tempcoor[1] = h->y;
        }
        if (h->tail == nullptr) {
            lastNode = h;//获取最后一个节点
        }
        h = h->tail; // 移动到下一个节点
    }
    // 记录最后一个节点的位置
    if (lastNode != nullptr) {
        tempcoor[0] = lastNode->x;
        tempcoor[1] = lastNode->y;
    }
}
//清除蛇蛇
void clearSnake(int n[]){
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO bInfo;
    GetConsoleScreenBufferInfo(hOutput, &bInfo);
    int len = (bInfo.dwSize.X / 2) - n[0] / 2;
    greedySnake *h = Snake;//获取头节点
    while(h){
        COORD pos = { (SHORT)(len + h->x), (SHORT)h->y };
        SetConsoleCursorPosition(hOutput, pos);
        std::cout << " "; // 用空格覆盖蛇的位置
        coordinate[h->x][h->y] = 0; // 清除蛇尾标记
        h = h->tail; // 移动到下一个节点
    }
}
//自动移动函数
void autoMove(arrow direction){
    greedySnake* node = Snake;
    int temp[2];
    temp[0] = node->x;
    temp[1] = node->y;
    //先更新头部
    switch (direction){
        case Up:
            node->y--;
            node = node->tail;
        break;
        case Down:
            node->y++;
            node = node->tail;
        break;
        case Left:
            node->x--;
            node = node->tail;
        break;
        case Right:
            node->x++;
            node = node->tail;
        break;
   
        default:
        break;
    }
    //让尾部的坐标相互继承
    while(node){
        int sweap = node->x;
        node->x = temp[0];
        temp[0] = sweap;
        sweap = node->y;
        node->y = temp[1];
        temp[1] = sweap;
        node = node->tail;
    }
}
//键盘事件
void key_down(){
    char cmd[100] = {0};
    // 检查WASD按键来更新方向
    while(_kbhit()) {
                char key = _getch();
                switch (key) {
                case 'w':
                    direction = Up;
                    break;
                case 's':
                    direction = Down;
                    break;
                case 'a':
                    direction = Left;
                    break;
                case 'd':
                    direction = Right;
                    break;
                case 'n':
                    mciSendStringA("stop gbc-bg2.mp3", 0, 0, 0);
                    break;
                case 'm':
                    mciSendStringA("play gbc-bg2.mp3 repeat", 0, 0, 0);
                    break;
                case 72: // 上箭头键，增加音量
                    volume = min(volume + 50, 1000);
                    sprintf(cmd, "setaudio gbc-bg2.mp3 volume to %d", volume);
                    mciSendStringA(cmd,0,0,0);
                    break;
                case 80: // 下箭头键，减少音量
                    volume = max(volume - 50, 0); // 限制音量不低于0
                    sprintf(cmd, "setaudio gbc-bg2.mp3 volume to %d", volume);
                    mciSendStringA(cmd, 0, 0, 0);
                    break;
                case 27:
                    system("pause");
                }
            }
}
//创建食物
void initcreateFood(int n[]) {
    // 生成随机数实现食物位置
    CustomRandom customRand(static_cast<unsigned int>(time(nullptr))); // 使用当前时间作为种子
    for(int i = 0;i <2;i++){
        food_x[i] = customRand.next() % (n[0]-2)+1;
        food_y[i] = customRand.next() % (n[1]-2)+1;
    } 
}
//打印食物
void printFood(){
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO bInfo;
    GetConsoleScreenBufferInfo(hOutput, &bInfo); // 获取窗口长度
    int len = (bInfo.dwSize.X / 2) - n[0] / 2; // 计算需要空出的格数
    // 设置光标位置并打印食物
    for(int i = 0;i < 2;i++){
        COORD pos = { (SHORT)(len + food_x[i]), (SHORT)food_y[i] }; // 光标位置
        SetConsoleCursorPosition(hOutput, pos);
        std::cout << "$";
        coordinate[food_x[i]][food_y[i]] = 2;
    }
}
//清除之前的食物
void clearFood() {
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO bInfo;
    GetConsoleScreenBufferInfo(hOutput, &bInfo);
    int len = (bInfo.dwSize.X / 2) - n[0] / 2;
    for(int i = 0;i < 2;i++){
        COORD pos = { (SHORT)(len + food_x[i]), (SHORT)food_y[i] };
        SetConsoleCursorPosition(hOutput, pos);
        std::cout << " "; // 用空格覆盖食物位置
        coordinate[food_x[i]][food_y[i]] = 0; // 清除食物标记
    }
}
//吃到食物，增长尾巴
void AddTail() {
    greedySnake *current = Snake;
    while (current->tail != nullptr) {
        current = current->tail;
    }
    greedySnake *newNode = new greedySnake;
    newNode->x = current->x;
    newNode->y = current->y;
    current->tail = newNode;
    newNode->tail = nullptr;//一定要初始化指针，不然程序崩溃
}
//检查那个食物被吃了创建新的
void createFood(int n[]){
    // 生成随机数实现食物位置
    CustomRandom customRand(static_cast<unsigned int>(time(nullptr))); // 使用当前时间作为种子
    if(Snake->x==food_x[0]&&Snake->y==food_y[0]){
        food_x[0] = customRand.next() % (n[0]-2)+1;
        food_y[0] = customRand.next() % (n[1]-2)+1;
    }else if(Snake->x==food_x[1]&&Snake->y==food_y[1]){
        food_x[1] = customRand.next() % (n[0]-2)+1;
        food_y[1] = customRand.next() % (n[1]-2)+1;
    }
}

void playSound(const char* soundFile);
void reStart(){
    string str = "你要再试一次吗？（输入yes或y则再来一次，输入其他则结束）";
    middle(str);
    cin>>restart;
    std::transform(restart.begin(), restart.end(), restart.begin(), ::tolower);
}
void other(){
    std::string warn = "游玩时请不要改变窗口大小\n";
    std::string warn1 = "如果觉得音乐吵可按‘n’来关闭音乐,觉得好听可按m来开启音乐\n";
    std::string warn2 = "建议窗口长为50~100，宽为30~50\n";
    middle(warn);
    middle(warn1);
    middle(warn2);
    system("pause");
    system("cls");
}

int main()
{   
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    other();
    std::string lin1 = "请输入边框长宽（请输入两个数,用空格隔开）：";
    std::string lin2 = "请输入难度（输入数字越大，难度越大）：";
    middle(lin1);
    cin >> n[0] >> n[1];
    middle(lin2);
    cin >> m;
    if(m == 0 )m = 1;
    cout << endl;
    system("cls");

    HideCursor();
    bool index = true;
    while(index){
        system("cls");
        point = 0;
        Snake = creatSnake();
        direction = Up;
        std::fill(&coordinate[0][0], &coordinate[999][999] + 1, 0);
        initcreateFood(n);
        printWall(n);
        mciSendStringA("open gbc-bg2.mp3",0,0,0);
        mciSendStringA("play gbc-bg2.mp3 repeat", 0, 0, 0);
        char cmd[100] = {0};
        sprintf(cmd, "setaudio gbc-bg2.mp3 volume to %d", volume);
        mciSendStringA(cmd, 0, 0, 0);
        while(true){
            printpoint();//打印分数和提示键
            key_down();//获取按键
            autoMove(direction);//蛇蛇移动
            printFood();//打印食物
            printSnake();//打印蛇
            ResetConsoleColor();//重置控制台颜色
            //撞墙和吃到食物事件
            if(coordinate[Snake->x][Snake->y]==1){
                mciSendStringA("stop gbc-bg2.mp3", 0, 0, 0);
                playSound("output.mp3");
                system("cls");
                reStart();
                break;
            }else if(coordinate[Snake->x][Snake->y]==2){//吃到食物
                clearFood(); // 清除旧食物位置
                createFood(n); // 重新生成食物位置
                printFood(); // 打印新食物位置
                AddTail(); // 增加蛇的长度
                point++;
                if(point % 100==0){
                    index=false;
                    system("cls");
                    mciSendStringA("stop gbc-bg2.mp3",0,0,0);
                    middle("なんで春日影やったの\n");
                    middle("感谢游玩\n");
                    mciSendStringA("open MyGO!!!!!.mp3",0,0,0);
                    mciSendStringA("play MyGO!!!!!.mp3",0,0,0);
                    break;
                }else if(point % 10 == 0) playSound("soyo.mp3");
                else playSound("yada.mp3");
            }
            
            Sleep(200/m);
            clearSnake(n);// 清除蛇位置
        }
        // 检查是否要重新开始
        if (restart != "yes" && restart != "y") {
            break; // 如果用户选择不重新开始，退出循环
        }
        delete [] Snake;
    }
    system("pause");
	return 0;
}

void playSound(const char* soundFile) {
    MCIERROR err;
    char cmd[100] = {0};
    // 停止当前播放的音频
    sprintf(cmd,"stop %s",soundFile);
    err = mciSendStringA(cmd, 0, 0, 0);
    if (err != 0) {
        // 处理错误
        cout << "Failed to stop audio file." << endl;
    }

    // 关闭音频设备
    sprintf(cmd,"close %s",soundFile);
    err = mciSendStringA(cmd, 0, 0, 0);

    // 打开音频文件
    sprintf(cmd,"open %s",soundFile);
    err = mciSendStringA(cmd, 0, 0, 0);
    if (err != 0) {
        // 处理错误
        cout << "Failed to open audio file." + std::string(soundFile) + "." << endl;
    }
    
    
    // 播放音频文件
    sprintf(cmd,"play %s",soundFile);
    err = mciSendStringA(cmd, 0, 0, 0);
    int i = volume * 500 / 1000;
    sprintf(cmd,"setaudio %s volume to %d",soundFile,i);
    mciSendStringA(cmd,0,0,0);
    if (err != 0) {
        // 处理错误
        cout << "Failed to play audio file." + std::string(soundFile) + "." << endl;
    }
}