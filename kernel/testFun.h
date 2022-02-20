void testFun();

void testFun(){
    print(" \n\n");

    char tmp[100];
    myMemset(tmp,0,100);
    unsigned int size=fileRead("TMP111  TXT",tmp); //读取tmp111并显示
    print("TMP111  TXT:\n")  ;
    print(tmp); 

    fileCreate("HHHHHH  TXT");  //创建HHHHHH.txt
    fileCreate("TMP222  TXT");  //创建TMP222.TXT

    tmp[0]='n';
    tmp[1]='e';
    tmp[2]='w';
    tmp[3]=' ';
    tmp[4]='F';         //对缓冲区内容进行部分修改

    fileWrite("TMP222  TXT",tmp,size); //把修改后的缓冲区内容写入新文件
    myMemset(tmp,0,100);//清空缓冲区
    fileRead("TMP222  TXT",tmp); //重新读取新文件
    print(" \n\n");
    print("TMP222  TXT:\n"); 
    print(tmp); //显示新文件内容
    return;

}